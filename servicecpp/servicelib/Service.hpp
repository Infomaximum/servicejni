#pragma once

bool ServiceRunning(SC_HANDLE hSCManager, const WCHAR* pServiceName)
{
	bool bResult = false;

	SC_HANDLE hService = OpenService(hSCManager, pServiceName, SERVICE_QUERY_STATUS);
	if (hService)
	{
		SERVICE_STATUS	ssStatus;
		if (QueryServiceStatus(hService, &ssStatus))
		{
			bResult = ssStatus.dwCurrentState == SERVICE_RUNNING;
		}
		CloseServiceHandle(hService);
	}

	return bResult;
}

bool ServiceSetup(SC_HANDLE hSCManager, const WCHAR* pServiceName, const WCHAR* pDisplayName, const WCHAR* pServicePath, DWORD dwServiceType, DWORD dwStartType, const WCHAR* pDependencies)
{
	bool			bResult = false;

	SC_HANDLE hService = CreateService(hSCManager, pServiceName, pDisplayName, SERVICE_ALL_ACCESS, dwServiceType, dwStartType, SERVICE_ERROR_IGNORE, pServicePath, NULL, NULL, pDependencies, NULL, NULL);
	if (hService != NULL)
	{
		CloseServiceHandle(hService);
		bResult = true;
	}
	else if (GetLastError() == ERROR_SERVICE_EXISTS)
	{
		hService = OpenService(hSCManager, pServiceName, SERVICE_CHANGE_CONFIG);
		if (hService)
		{
			bResult = ChangeServiceConfig(hService, dwServiceType, dwStartType, SERVICE_ERROR_IGNORE, pServicePath, NULL, NULL, pDependencies, 0, 0, pDisplayName) != FALSE;
			CloseServiceHandle(hService);
		}
	}

	return bResult;
}

DWORD ServiceDelete(SC_HANDLE hSCManager, const WCHAR* pServiceName)
{
	DWORD	error = ERROR_SUCCESS;

	SC_HANDLE hService = OpenServiceW(hSCManager, pServiceName, SERVICE_QUERY_STATUS | DELETE);
	if (hService != NULL)
	{
		SERVICE_STATUS_PROCESS status = { 0 };
		DWORD bytesNeed;
		if (QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, (BYTE*)&status, sizeof(SERVICE_STATUS_PROCESS), &bytesNeed))
		{
			HANDLE process = OpenProcess(PROCESS_TERMINATE, FALSE, status.dwProcessId);
			if (process != NULL)
			{
				::TerminateProcess(process, 0);
				WaitForSingleObject(process, INFINITE);
				CloseHandle(process);
			}
		}

		if (!DeleteService(hService))
		{
			error = GetLastError();
		}
		CloseServiceHandle(hService);
	}
	else
	{
		error = GetLastError();
	}

	switch (error)
	{
	case ERROR_SERVICE_DOES_NOT_EXIST:
		error = ERROR_SUCCESS;
		break;
	}

	return error;
}

DWORD ServiceSetStartType(SC_HANDLE hSCManager, const WCHAR* pServiceName, DWORD dwStartType)
{
	DWORD dwError = ERROR_SUCCESS;

	SC_HANDLE schService = OpenServiceW(hSCManager, pServiceName, SERVICE_CHANGE_CONFIG);
	if (schService != NULL)
	{
		if (!ChangeServiceConfigW(schService, SERVICE_NO_CHANGE, dwStartType, SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL, NULL, NULL, NULL))
		{
			dwError = GetLastError();
		}
		CloseServiceHandle(schService);
	}
	else
	{
		dwError = GetLastError();
	}

	return dwError;
}

DWORD ServiceGetStartType(SC_HANDLE hSCManager, const WCHAR* pServiceName, DWORD* pStartType)
{
	DWORD			error = ERROR_SUCCESS;

	SC_HANDLE schService = OpenServiceW(hSCManager, pServiceName, SERVICE_QUERY_CONFIG);
	if (schService != NULL)
	{
		BYTE			pBuffer[8192];
		DWORD			dwBytesNeeded;

		if (QueryServiceConfig(schService, (QUERY_SERVICE_CONFIG*)pBuffer, 8192, &dwBytesNeeded))
		{
			*pStartType = ((QUERY_SERVICE_CONFIG*)pBuffer)->dwStartType;
		}
		else
		{
			error = GetLastError();
		}

		CloseServiceHandle(schService);
	}
	else
	{
		error = GetLastError();
	}

	return error;
}

DWORD ServiceStart(SC_HANDLE hSCManager, const WCHAR* pServiceName, DWORD defaultStartType = SERVICE_DEMAND_START)
{
	DWORD			error;
	DWORD			startType;

	if (ServiceGetStartType(hSCManager, pServiceName, &startType) == ERROR_SUCCESS &&
		startType == SERVICE_DISABLED)
	{
		error = ServiceSetStartType(hSCManager, pServiceName, defaultStartType);
		if (error != ERROR_SUCCESS)
		{
			return error;
		}
	}

	SC_HANDLE hService = OpenServiceW(hSCManager, pServiceName, SERVICE_START | SERVICE_QUERY_STATUS);
	if (hService != NULL)
	{
		bool res = StartServiceW(hService, 0, NULL) != FALSE;
		error = GetLastError();
		if (res || error == ERROR_IO_PENDING)
		{
			SERVICE_STATUS	ssStatus = {0};
			while (QueryServiceStatus(hService, &ssStatus))
			{
				if (ssStatus.dwCurrentState == SERVICE_START_PENDING)
				{
					//Дмитрий Иванов: Магия. Получено эмпирическим путем.
					Sleep(500);
				}
				else
				{
					error = ERROR_SUCCESS;
					break;
				}
			}
		}
		else if (error == ERROR_SERVICE_ALREADY_RUNNING)
		{
			error = ERROR_SUCCESS;
		}

		CloseServiceHandle(hService);
	}
	else
	{
		error = GetLastError();
	}

	return error;
}

bool ServiceGetExitCode(SC_HANDLE hSCManager, const WCHAR* pServiceName, DWORD *pExitCode)
{
	bool			bResult = false;

	SC_HANDLE schService = OpenService(hSCManager, pServiceName, SERVICE_QUERY_STATUS);
	if (schService != NULL)
	{
		SERVICE_STATUS_PROCESS	status = {0};
		DWORD			dwBytesNeeded;

		bResult = QueryServiceStatusEx(schService, SC_STATUS_PROCESS_INFO, (LPBYTE)&status,  sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded) != FALSE;
		if (bResult)
		{
			*pExitCode = status.dwServiceSpecificExitCode;
		}
		CloseServiceHandle(schService);
	}

	return bResult;
}

DWORD ServiceExecuteOnce(SC_HANDLE hSCManager, const WCHAR *pServiceName, const WCHAR *pServiceDisplayName, const WCHAR *pBinaryPath)
{
	bool res = ServiceSetup(hSCManager, pServiceName, pServiceDisplayName, pBinaryPath, SERVICE_WIN32_OWN_PROCESS, SERVICE_DEMAND_START, NULL);
	if (!res)
	{
		return GetLastError();
	}

	DWORD error = ServiceStart(hSCManager, pServiceName);
	if (error == ERROR_SUCCESS)
	{
		// ждем пока удаленный сервис не завершит свою работу
		while (ServiceRunning(hSCManager, pServiceName)) Sleep(300);

		ServiceGetExitCode(hSCManager, pServiceName, &error);
	}
	ServiceDelete(hSCManager, pServiceName);

	return error;
}
