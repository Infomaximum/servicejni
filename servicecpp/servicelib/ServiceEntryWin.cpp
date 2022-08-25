#pragma once

#include "Pending.hpp"
#include "ServiceEntry.h"
#include "StringConverter.h"

static ServiceEntry* pInstance = nullptr;

struct ServiceEntry::Impl
{
	const std::wstring		serviceName;
	SERVICE_STATUS_HANDLE	hSS = nullptr;
	SERVICE_STATUS			status;

	Impl(std::wstring&& serviceName)
		: serviceName(std::move(serviceName))
	{
		memset(&status, 0, sizeof(status));

		status.dwServiceSpecificExitCode = 0;
		status.dwWin32ExitCode = NO_ERROR;
		status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
		status.dwCheckPoint = 0;
		status.dwWaitHint = 2000;
		status.dwCurrentState = SERVICE_START_PENDING;
		status.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
	}

	static void WINAPI ServiceMain(DWORD /*dwArgc*/, LPWSTR* /*pszArgv*/)
	{
		pInstance->pImpl->Register();
	}

	static DWORD WINAPI ServiceHandler(DWORD dwCode, DWORD dwEventType, LPVOID pEventData, LPVOID pContext)
	{
		switch (dwCode)
		{
		case SERVICE_CONTROL_SHUTDOWN:
		case SERVICE_CONTROL_STOP:
			ServicePrepare(pInstance->onStoppingHandler, pInstance->pImpl->hSS, &pInstance->pImpl->status, SERVICE_STOPPED);
			break;
		default:
			const auto iter = pInstance->eventHandlers.find(dwCode);
			if (iter != pInstance->eventHandlers.end())
			{
				iter->second(dwEventType, pEventData);
			}
			break;
		}

		return NO_ERROR;
	}

	void Register()
	{
		hSS = RegisterServiceCtrlHandlerExW(serviceName.c_str(), ServiceHandler, 0); // Сразу регистрируем обработчик запросов.
		ServicePrepare(pInstance->onStartingHandler, hSS, &status, SERVICE_RUNNING);
	}
};

ServiceEntry::ServiceEntry(const std::string& serviceName)
	: pImpl(std::make_unique<Impl>(StringConverter::UTF8toUTF16(serviceName)))
{
	pInstance = this;
}

ServiceEntry::~ServiceEntry()
{
	pInstance = nullptr;
}

void ServiceEntry::RegisterEventHandler(Event event, Handler&& handler)
{
	switch (event)
	{
	case Event::STARTING:
		onStartingHandler = std::move(handler);
		break;
	case Event::STOPPING:
		onStoppingHandler = std::move(handler);
		break;
	}
}

void ServiceEntry::RegisterEventHandler(uint32_t nativeEventCode, HandlerNative&& handler)
{
	eventHandlers.insert_or_assign(nativeEventCode, std::move(handler));

	if (nativeEventCode == SERVICE_CONTROL_SESSIONCHANGE)
	{
		pImpl->status.dwControlsAccepted |= SERVICE_ACCEPT_SESSIONCHANGE;
	}
}

void ServiceEntry::SetExitCode(uint32_t exitCode)
{
	pImpl->status.dwServiceSpecificExitCode = exitCode;
	pImpl->status.dwWin32ExitCode = exitCode != 0 ? ERROR_SERVICE_SPECIFIC_ERROR : NO_ERROR;
}

void ServiceEntry::Execute(uint32_t& err)
{
	err = ERROR_SUCCESS;

	SERVICE_TABLE_ENTRYW serviceTableEntry[2] = { 0 };
	serviceTableEntry[0].lpServiceName = const_cast<wchar_t*>(pImpl->serviceName.c_str());
	serviceTableEntry[0].lpServiceProc = ServiceEntry::Impl::ServiceMain;
	serviceTableEntry[1].lpServiceName = NULL;
	serviceTableEntry[1].lpServiceProc = NULL;
	if (!StartServiceCtrlDispatcherW(serviceTableEntry)) // Устанавливаем соединение с SCM. Внутри этой функции происходит прием и диспетчеризация запросов.
	{
		err = GetLastError();
	}
}

void ServiceEntry::Exit()
{
	ServicePrepare(onStoppingHandler, pImpl->hSS, &pImpl->status, SERVICE_STOPPED);
}
