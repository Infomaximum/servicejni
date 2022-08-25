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
	// �������� ��������� ����� ������ ������� ��������(SCM) �� �������� ����, ����� �� ��� ��� ���
	pServiceStatus->dwWaitHint = 2000;
	pServiceStatus->dwCheckPoint = 0; // ���������� ������� ���������

	// ���� ��������� ������������� ������� (���� ������� �� �������� - �� ���������)
	if (prepareFunc != nullptr)
	{
		// ���������� ��������� SCM � ����� ����������, ��� �� � ������
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
		
		// ���������� �������, ��� �� �� �������
		do
		{
			SetServiceStatus(hServiceStatus, pServiceStatus);
			// �������� ������� ��������� ��� ���������� � ���������� ����������� SCM (������� - ����� ����� ��������� ������ � ������ ��������)
			pServiceStatus->dwCheckPoint++;
		} while (WaitForSingleObject(hPreparerThread, 1000) == WAIT_TIMEOUT);

		CloseHandle(hPreparerThread);

		pServiceStatus->dwCheckPoint = 0;
	}
	
	// ������ ���������������, ���������� ������ �������
	pServiceStatus->dwCurrentState = dwTargetState;
	SetServiceStatus(hServiceStatus, pServiceStatus);
}
