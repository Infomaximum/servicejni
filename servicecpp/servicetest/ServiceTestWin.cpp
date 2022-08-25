#include <Windows.h>

#include "gtest/gtest.h"
#include <servicelib/Service.h>
#include "TestServiceEntry.h"

namespace test
{
	std::string currentExeFile;

	void setCurrentExeFile(const std::string& value)
	{
		currentExeFile = value;
	}
}

namespace ServiceTest
{

TEST(Service, Read)
{
	uint32_t err;
	
	bool res = Service::Exists("not_existence_service", err);
	ASSERT_EQ(err, 0);
	ASSERT_FALSE(res);

	res = Service::Exists("Browser", err);
	ASSERT_EQ(err, 0);
	ASSERT_TRUE(res);

	auto pService = Service::Open("not_existence_service", Service::OpenAccessRights::AllAccess, err);
	ASSERT_EQ(err, ERROR_SERVICE_DOES_NOT_EXIST);
	ASSERT_EQ(pService, nullptr);
	
	pService = Service::Open("Browser", Service::OpenAccessRights::AllAccess, err);
	ASSERT_EQ(err, 0);
	ASSERT_NE(pService, nullptr);

	pService = Service::Open("Browser", Service::OpenAccessRights::ReadWrite, err);
	ASSERT_EQ(err, 0);
	ASSERT_NE(pService, nullptr);

	pService = Service::Open("Browser", Service::OpenAccessRights::ExecuteOnly, err);
	ASSERT_EQ(err, 0);
	ASSERT_NE(pService, nullptr);

	pService = Service::Open("Browser", Service::OpenAccessRights::ReadOnly, err);
	ASSERT_EQ(err, 0);
	ASSERT_NE(pService, nullptr);

	auto status = pService->GetStatus(err);
	ASSERT_EQ(err, 0);
	ASSERT_TRUE(status.isRunning);
	ASSERT_EQ(status.exitCode, 0);

	const std::string desc = pService->GetDescription(err);
	ASSERT_EQ(err, 0);
	ASSERT_EQ(desc, std::string(u8"Обслуживает список компьютеров в сети и выдает его программам по запросу. Если служба остановлена, список не будет создан или обновлен. Если данная служба неразрешена, не удастся запустить любые явно зависимые службы."));

	uint32_t pid = pService->GetPid(err);
	ASSERT_EQ(err, 0);
	ASSERT_NE(pid, 0);

	const std::string path = pService->GetExecutablePath(err);
	ASSERT_EQ(err, 0);
	ASSERT_EQ(path, std::string(u8"C:\\Windows\\System32\\svchost.exe -k netsvcs"));

	pService->SetDescription(u8"Обслуживает список компьютеров в сети и выдает его программам по запросу. Если служба остановлена, список не будет создан или обновлен. Если данная служба неразрешена, не удастся запустить любые явно зависимые службы.", err);
	ASSERT_EQ(err, ERROR_ACCESS_DENIED);

	pService->StartAsync(err);
	ASSERT_EQ(err, ERROR_ACCESS_DENIED);

	pService->Stop(err);
	ASSERT_EQ(err, ERROR_ACCESS_DENIED);
}

void ASSERT_EXISTS_EXIT_CODE_MASK(uint32_t exitCode, test::ExitCodeMask mask)
{
	using namespace test;

	const uint32_t exitCodes = exitCode > EXIT_CODE_WRAP ? exitCode - EXIT_CODE_WRAP : 0;
	ASSERT_EQ((exitCodes & mask), mask);
}

TEST(Service, Edit)
{
	using namespace test;

	uint32_t err;
	Service::Remove(SERVICE_NAME, err);
	ASSERT_EQ(err, 0);
	{

		std::string path = "\"" + currentExeFile + "\" " + RUN_AS_SERVICE;
		auto pService = Service::Create(SERVICE_NAME, path, Service::AccountType::LocalSystem, err);
		ASSERT_EQ(err, 0);
		ASSERT_NE(pService, nullptr);

		pService->Stop(err);
		ASSERT_EQ(err, 0);

		auto status = pService->GetStatus(err);
		ASSERT_EQ(err, 0);
		ASSERT_FALSE(status.isRunning);
		ASSERT_EQ(status.exitCode, 0);

		pService->Start(err);
		ASSERT_EQ(err, 0);
		status = pService->GetStatus(err);
		ASSERT_EQ(err, 0);
		ASSERT_TRUE(status.isRunning);

		pService->StartAsync(err);
		ASSERT_EQ(err, 0);
		
		const std::string descEtalon(u8"Некоторое description");
		pService->SetDescription(descEtalon, err);
		ASSERT_EQ(err, 0);
		const std::string desc = pService->GetDescription(err);
		ASSERT_EQ(err, 0);
		ASSERT_EQ(desc, descEtalon);

		pService->SendCode(USER_CODE, err);
		ASSERT_EQ(err, 0);
		status = pService->GetStatus(err);
		ASSERT_EQ(err, 0);
		ASSERT_TRUE(status.isRunning);

		pService->Stop(err);
		ASSERT_EQ(err, 0);
		status = pService->GetStatus(err);
		ASSERT_EQ(err, 0);
		ASSERT_FALSE(status.isRunning);
		ASSERT_EXISTS_EXIT_CODE_MASK(status.exitCode, ExitCodeMask::STARTING);
		ASSERT_EXISTS_EXIT_CODE_MASK(status.exitCode, ExitCodeMask::STOPPING);
		ASSERT_EXISTS_EXIT_CODE_MASK(status.exitCode, ExitCodeMask::USER_CODE_ACCEPTED);

		pService->Start(err);
		ASSERT_EQ(err, 0);
	}
	Service::Remove(SERVICE_NAME, err);
	ASSERT_EQ(err, 0);

	auto pService = Service::Open(SERVICE_NAME, Service::OpenAccessRights::ReadOnly, err);
	ASSERT_EQ(err, ERROR_SERVICE_DOES_NOT_EXIST);
	ASSERT_EQ(pService, nullptr);
}

}
