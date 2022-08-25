#pragma once

#include <Windows.h>
#include <functional>

DWORD WINAPI RunPrepareFunc(void* pFunc)
{
	auto& func = *(std::function<void()>*)pFunc;
	if (func != nullptr)
	{
		func();
	}
	
	return 0;
}

void ServicePrepare(std::function<void()>& prepareFunc, SERVICE_STATUS_HANDLE hServiceStatus, SERVICE_STATUS* pServiceStatus, DWORD dwTargetState)
{
	// параметр указывает время опроса сервиса системой(SCM) на предемет того, завис он или еще нет
	pServiceStatus->dwWaitHint = 2000;
	pServiceStatus->dwCheckPoint = 0; // установили счетчик прогресса

	// если требуется инициализация сервиса (если функцию не передали - не требуется)
	if (prepareFunc != nullptr)
	{
		// необходимо известить SCM о наших намерениях, что мы и делаем
		switch (dwTargetState)
		{
		case SERVICE_RUNNING:
			pServiceStatus->dwCurrentState = SERVICE_START_PENDING;
			break;
		case SERVICE_STOPPED:
			pServiceStatus->dwCurrentState = SERVICE_STOP_PENDING;
			break;
		default:
			break;
		}

		HANDLE hPreparerThread = CreateThread(NULL, 0, RunPrepareFunc, &prepareFunc, 0, NULL);
		
		// уведомляем систему, что мы не зависли
		do
		{
			SetServiceStatus(hServiceStatus, pServiceStatus);
			// изменяем счетчик прогресса для подготовки к следующему уведомлению SCM (костыль - иначе будет системная ошибка и сервис прикроют)
			pServiceStatus->dwCheckPoint++;
		} while (WaitForSingleObject(hPreparerThread, 1000) == WAIT_TIMEOUT);

		CloseHandle(hPreparerThread);

		pServiceStatus->dwCheckPoint = 0;
	}
	
	// сервис инициализирован, выставляем статус сервиса
	pServiceStatus->dwCurrentState = dwTargetState;
	SetServiceStatus(hServiceStatus, pServiceStatus);
}
