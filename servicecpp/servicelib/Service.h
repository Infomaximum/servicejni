#pragma once

#include "PlatformConfiguration.h"
#include <string>
#include <memory>

class Service
{
public:

	struct Status
	{
		bool		isRunning = false;
		uint32_t	exitCode = 0;
	};

	enum class AccountType
	{
		LocalSystem = 0, LocalService = 1, VirtualAccount = 2
	};

	enum class OpenAccessRights
	{
		AllAccess = 0, ReadWrite = 1, ExecuteOnly = 2, ReadOnly = 3
	};

private:

#if defined(WINDOWS)
	struct ScManager;
#endif

#if defined(WINDOWS)
	void* hService = nullptr;
#elif defined(LINUX)
	const std::string serviceName;
	static const pid_t SERVICE_INVALID_PID = -1;
#endif

public:

	~Service();

	Service(const Service& service) = delete;
	Service& operator =(const Service& service) = delete;

	Service(Service&& service) = delete;
	Service& operator =(Service&& service) = delete;

private:

#if defined(WINDOWS)
	Service(void* hService);
#elif defined(LINUX)
	Service(const std::string& serviceName);
#endif
	
public:

	static std::unique_ptr<Service> Create(const std::string& name, const std::string& executablePath, AccountType serviceAccount, uint32_t& err);
	static std::unique_ptr<Service> Open(const std::string& name, OpenAccessRights accessRigths, uint32_t& err);
	static bool Exists(const std::string& name, uint32_t& err);
	static void Remove(const std::string& name, uint32_t& err);

#if defined(WINDOWS)
	static std::unique_ptr<Service> CreateDriver(const std::string& name, const std::string& servicePath, uint32_t& err);
	static void ExecuteOnce(const std::string& name, const std::string& executablePath, uint32_t& err);
#endif

	void Start(uint32_t& err);
	void StartAsync(uint32_t& err);
	void Stop(uint32_t& err);

	Status GetStatus(uint32_t& err) const;

	void SetAutoRestart(bool autoRestart, const std::string& postFailCommand, uint32_t& err);

	std::string GetDescription(uint32_t& err) const;
	void SetDescription(const std::string& description, uint32_t& err);

	void SendCode(uint32_t userCode, uint32_t& err);

	uint32_t GetPid(uint32_t& err) const;
	std::string GetExecutablePath(uint32_t& err) const;
};
