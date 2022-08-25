#pragma once

#include <Windows.h>

#include "Service.h"
#include "Service.hpp"
#include "StringConverter.h"
#include <chrono>

const uint32_t TIME_OUT_MS = 200; // ms

struct Service::ScManager
{
	const SC_HANDLE hSCManager = NULL;
	const DWORD dwError = ERROR_SUCCESS;

	explicit ScManager(OpenAccessRights accessRights)
		: hSCManager(OpenSCManager(NULL, NULL, ToServiceManagerAccessFlags(accessRights)))
		, dwError(hSCManager == NULL ? GetLastError() : ERROR_SUCCESS)
	{
	}

	~ScManager()
	{
		CloseServiceHandle(hSCManager);
	}

	std::unique_ptr<Service> Open(const std::string& name, OpenAccessRights accessRigths, uint32_t& err)
	{
		if (hSCManager == NULL)
		{
			err = dwError;
			return nullptr;
		}

		const std::wstring nameW = StringConverter::UTF8toUTF16(name);

		SC_HANDLE hService = ::OpenServiceW(hSCManager, nameW.c_str(), ToServiceAccessFlags(accessRigths));
		if (hService == NULL)
		{
			err = GetLastError();
			return nullptr;
		}

		err = ERROR_SUCCESS;
		return std::unique_ptr<Service>(new Service(hService));
	}

	std::unique_ptr<Service> Create(const std::string& name, const std::string& servicePath, uint32_t serviceType, AccountType serviceAccount, uint32_t& err)
	{
		if (hSCManager == NULL)
		{
			err = dwError;
			return nullptr;
		}

		std::wstring servicePathW = StringConverter::UTF8toUTF16(servicePath);
		// Для драйверов пробелы экранировать не нужно
		if ((serviceType & SERVICE_KERNEL_DRIVER) != SERVICE_KERNEL_DRIVER &&
			*servicePathW.begin() != L'"')
		{
			servicePathW.insert(servicePathW.begin(), L'"');
			servicePathW.append(L"\"");
		}

		const std::wstring	nameW = StringConverter::UTF8toUTF16(name);
		const std::wstring serviceStartName = MakeAccountName(serviceAccount, nameW);
		const wchar_t* pServiceStartName = serviceStartName.empty() ? NULL : serviceStartName.c_str();

		SC_HANDLE hService = ::CreateServiceW(hSCManager, nameW.c_str(), nameW.c_str(),
			SERVICE_ALL_ACCESS, serviceType, SERVICE_AUTO_START, SERVICE_ERROR_IGNORE, servicePathW.c_str(), 
			NULL, NULL, NULL, pServiceStartName, NULL);
		if (hService == NULL)
		{
			if (GetLastError() != ERROR_SERVICE_EXISTS)
			{
				err = GetLastError();
				return nullptr;
			}

			hService = ::OpenServiceW(hSCManager, nameW.c_str(), SERVICE_ALL_ACCESS);
			if (hService == NULL)
			{
				err = GetLastError();
				return nullptr;
			}

			if (!ChangeServiceConfigW(hService, serviceType, SERVICE_AUTO_START, SERVICE_ERROR_IGNORE, servicePathW.c_str(), NULL, NULL, NULL, pServiceStartName, NULL, nameW.c_str()))
			{
				err = GetLastError();
				CloseServiceHandle(hService);
				return nullptr;
			}
		}

		std::unique_ptr<Service> pService(new Service(hService));
		pService->Stop(err);

		err = ERROR_SUCCESS;
		return pService;
	}

private:

	static std::wstring MakeAccountName(AccountType serviceAccount, const std::wstring& name)
	{
		switch (serviceAccount)
		{
		case AccountType::LocalService:
			return L"NT AUTHORITY\\LocalService";
		case AccountType::VirtualAccount:
			return L"NT SERVICE\\" + name;
		default:
			return std::wstring();
		}
	}

	static DWORD ToServiceManagerAccessFlags(OpenAccessRights accessRigths)
	{
		switch (accessRigths)
		{
		case OpenAccessRights::AllAccess:
		case OpenAccessRights::ExecuteOnly:
			return SC_MANAGER_ALL_ACCESS;
		case OpenAccessRights::ReadWrite:
			return GENERIC_READ | GENERIC_WRITE;
		case OpenAccessRights::ReadOnly:
		default:
			return GENERIC_READ;
		}
	}

	static DWORD ToServiceAccessFlags(OpenAccessRights accessRigths)
	{
		switch (accessRigths)
		{
		case OpenAccessRights::AllAccess:
			return SERVICE_ALL_ACCESS;
		case OpenAccessRights::ReadWrite:
			return GENERIC_READ | GENERIC_WRITE;
		case OpenAccessRights::ExecuteOnly:
			return GENERIC_READ | STANDARD_RIGHTS_EXECUTE | SERVICE_START | SERVICE_USER_DEFINED_CONTROL;
		case OpenAccessRights::ReadOnly:
		default:
			return GENERIC_READ;
		}
	}
};

Service::Service(void* hService)
{
	static_assert(sizeof(hService) == sizeof(SC_HANDLE), "sizeof(hService) != sizeof(SC_HANDLE)");

	this->hService = hService;
}

Service::~Service()
{
	CloseServiceHandle(reinterpret_cast<SC_HANDLE>(hService));
	hService = nullptr;
}

std::unique_ptr<Service> Service::Create(const std::string& name, const std::string& servicePath, AccountType serviceAccount, uint32_t& err)
{
	ScManager scManager(OpenAccessRights::AllAccess);

	return scManager.Create(name, servicePath, SERVICE_WIN32_OWN_PROCESS, serviceAccount, err);
}

std::unique_ptr<Service> Service::CreateDriver(const std::string& name, const std::string& servicePath, uint32_t& err)
{
	ScManager scManager(OpenAccessRights::AllAccess);

	return scManager.Create(name, servicePath, SERVICE_KERNEL_DRIVER, AccountType::LocalSystem, err);
}

std::unique_ptr<Service> Service::Open(const std::string& name, OpenAccessRights accessRights, uint32_t& err)
{
	ScManager scManager(accessRights);

	return scManager.Open(name, accessRights, err);
}

void Service::ExecuteOnce(const std::string& name, const std::string& binaryPath, uint32_t& err)
{
	ScManager scManager(OpenAccessRights::AllAccess);
	if (scManager.hSCManager == NULL)
	{
		err = scManager.dwError;
		return;
	}

	const std::wstring nameW = StringConverter::UTF8toUTF16(name);

	if (ServiceRunning(scManager.hSCManager, nameW.c_str()))
	{
		err = ERROR_SERVICE_ALREADY_RUNNING;
		return;
	}

	const std::wstring pathW = StringConverter::UTF8toUTF16(binaryPath);

	err = ServiceExecuteOnce(scManager.hSCManager, nameW.c_str(), nameW.c_str(), pathW.c_str());
}

bool Service::Exists(const std::string& name, uint32_t& err)
{
	auto pService = Open(name, OpenAccessRights::ReadOnly, err);
	switch (err)
	{
	case ERROR_SUCCESS:
		return true;
	case ERROR_SERVICE_DOES_NOT_EXIST:
		err = ERROR_SUCCESS;
		break;
	}
	return false;
}

void Service::Remove(const std::string& name, uint32_t& err)
{
	auto pService = Open(name, OpenAccessRights::AllAccess, err);
	if (err == ERROR_SERVICE_DOES_NOT_EXIST)
	{
		err = ERROR_SUCCESS;
		return;
	}

	pService->Stop(err);
	err = ERROR_SUCCESS;
	if (!DeleteService(reinterpret_cast<SC_HANDLE>(pService->hService)))
	{
		err = GetLastError();
	}
}

void Service::Start(uint32_t& err)
{
	auto status = GetStatus(err);
	if (status.isRunning)
	{
		return;
	}

	StartAsync(err);

	do
	{
		if (err == ERROR_SUCCESS)
		{
			Sleep(TIME_OUT_MS);
		}
		else
		{
			break;
		}
		status = GetStatus(err);
	} while (!status.isRunning && status.exitCode == NO_ERROR);
}

void Service::StartAsync(uint32_t& err)
{
	err = ERROR_SUCCESS;
	if (!StartServiceW(reinterpret_cast<SC_HANDLE>(hService), 0, 0))
	{
		err = GetLastError();
		if (err == ERROR_SERVICE_ALREADY_RUNNING)
		{
			err = ERROR_SUCCESS;
		}
	}
}

void Service::Stop(uint32_t& err)
{
	SERVICE_STATUS	ssStatus = { 0 };

	err = ERROR_SUCCESS;
	if (!ControlService(reinterpret_cast<SC_HANDLE>(hService), SERVICE_CONTROL_STOP, &ssStatus))
	{
		err = GetLastError();
		switch (err)
		{
		case ERROR_SERVICE_NOT_ACTIVE:
			err = ERROR_SUCCESS;
			return;
		case ERROR_SERVICE_REQUEST_TIMEOUT:
			//do nothing
			break;
		default:
			return;
		}
		Sleep(TIME_OUT_MS);
	}

	while (QueryServiceStatus(reinterpret_cast<SC_HANDLE>(hService), &ssStatus))
	{
		if (ssStatus.dwCurrentState == SERVICE_STOP_PENDING)
		{
			Sleep(TIME_OUT_MS);
		}
		else
		{
			err = ERROR_SUCCESS;
			return;
		}
	}
	err = GetLastError();
}

void Service::SetAutoRestart(bool autoRestart, const std::string& postFailCommand, uint32_t& err)
{
	using namespace std::chrono;

	SERVICE_FAILURE_ACTIONSW	servFailActions = { 0 };
	SC_ACTION					failActions[] = {
		{ SC_ACTION_RESTART,	(DWORD)duration_cast<milliseconds>(1min).count() },
		{ SC_ACTION_RESTART,	(DWORD)duration_cast<milliseconds>(3min).count() },
		{ SC_ACTION_NONE,		0 }
	};
	const DWORD					actionCount = autoRestart ? sizeof(failActions) / sizeof(SC_ACTION) : 0;

	std::wstring command;
	if (postFailCommand.empty())
	{
		servFailActions.lpCommand = L"";
	}
	else if (autoRestart)
	{
		failActions[actionCount - 1].Type = SC_ACTION_RUN_COMMAND;
		failActions[actionCount - 1].Delay = (DWORD)duration_cast<milliseconds>(30s).count();

		command = StringConverter::UTF8toUTF16(postFailCommand);
		servFailActions.lpCommand = const_cast<wchar_t*>(command.c_str());
	}

	servFailActions.dwResetPeriod = autoRestart ? (DWORD)duration_cast<seconds>(1h).count() : 0;
	servFailActions.lpRebootMsg = L"";
	servFailActions.cActions = actionCount;
	servFailActions.lpsaActions = failActions;

	err = ERROR_SUCCESS;
	if (!ChangeServiceConfig2W(reinterpret_cast<SC_HANDLE>(hService), SERVICE_CONFIG_FAILURE_ACTIONS, &servFailActions))
	{
		err = GetLastError();
	}
}

std::string Service::GetDescription(uint32_t& err) const
{
	BYTE	pBuffer[8 * 1024];
	DWORD	dwBytesNeeded;

	if (!QueryServiceConfig2W(reinterpret_cast<SC_HANDLE>(hService), SERVICE_CONFIG_DESCRIPTION, pBuffer, sizeof(pBuffer), &dwBytesNeeded))
	{
		err = GetLastError();
		return std::string();
	}
	err = ERROR_SUCCESS;

	SERVICE_DESCRIPTIONW* pSrvDesc = (SERVICE_DESCRIPTIONW*)pBuffer;
	if (pSrvDesc->lpDescription == NULL)
	{
		return std::string();
	}

	return StringConverter::UTF16toUTF8(pSrvDesc->lpDescription);
}

void Service::SetDescription(const std::string& description, uint32_t& err)
{
	SERVICE_DESCRIPTIONW	srvDesc = { 0 };
	const std::wstring		descW = StringConverter::UTF8toUTF16(description);

	srvDesc.lpDescription = descW.empty() ? L"" : descW.c_str();

	if (ChangeServiceConfig2W(reinterpret_cast<SC_HANDLE>(hService), SERVICE_CONFIG_DESCRIPTION, &srvDesc))
	{
		err = ERROR_SUCCESS;
	}
	else
	{
		err = GetLastError();
	}
}

void Service::SendCode(uint32_t userCode, uint32_t& err)
{
	SERVICE_STATUS			status = { 0 };

	if (ControlService(reinterpret_cast<SC_HANDLE>(hService), userCode, &status))
	{
		err = ERROR_SUCCESS;
	}
	else
	{
		err = GetLastError();
	}
}

uint32_t Service::GetPid(uint32_t& err) const
{
	SERVICE_STATUS_PROCESS status = { 0 };
	DWORD bytesNeed;
	if (!QueryServiceStatusEx(reinterpret_cast<SC_HANDLE>(hService), SC_STATUS_PROCESS_INFO, (BYTE*)&status, sizeof(SERVICE_STATUS_PROCESS), &bytesNeed))
	{
		err = GetLastError();
		return 0;
	}

	err = ERROR_SUCCESS;
	return status.dwProcessId;
}

std::string Service::GetExecutablePath(uint32_t& err) const
{
	BYTE	buffer[8 * 1024];

	QUERY_SERVICE_CONFIGW* pConfig = (QUERY_SERVICE_CONFIGW*)buffer;
	DWORD bytesNeeded;
	if (!QueryServiceConfigW(reinterpret_cast<SC_HANDLE>(hService), pConfig, sizeof(buffer), &bytesNeeded))
	{
		err = GetLastError();
		return std::string();
	}
	err = ERROR_SUCCESS;
	return StringConverter::UTF16toUTF8(std::wstring(pConfig->lpBinaryPathName));
}

Service::Status Service::GetStatus(uint32_t& err) const
{
	SERVICE_STATUS	status = { 0 };

	if (!QueryServiceStatus(reinterpret_cast<SC_HANDLE>(hService), &status))
	{
		err = GetLastError();
		return Status();
	}
	err = ERROR_SUCCESS;

	Status runningStatus;
	runningStatus.isRunning = status.dwCurrentState == SERVICE_RUNNING;
	if (status.dwWin32ExitCode == ERROR_SERVICE_SPECIFIC_ERROR)
	{
		runningStatus.exitCode = status.dwServiceSpecificExitCode;
	}
	else
	{
		runningStatus.exitCode = status.dwWin32ExitCode;
	}

	if (runningStatus.exitCode == ERROR_SERVICE_NEVER_STARTED)
	{
		runningStatus.exitCode = NO_ERROR;
	}

	return runningStatus;
}
