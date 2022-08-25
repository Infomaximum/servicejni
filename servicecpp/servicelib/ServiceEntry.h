#pragma once

#include "PlatformConfiguration.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <functional>

class ServiceEntry
{
public:

	enum class Event
	{
		STARTING = 0, STOPPING = 1
	};

	using Handler = std::function<void()>;

#if defined(WINDOWS)
	using HandlerNative = std::function<void(uint32_t eventType, void* pEventData)>;
#else
	using HandlerNative = std::function<void()>;
#endif

private:

	struct Impl;

private:

	std::unique_ptr<Impl> pImpl;

	Handler				onStartingHandler = nullptr;
	Handler				onStoppingHandler = nullptr;

	std::unordered_map<uint32_t, HandlerNative>	eventHandlers;

public:

	ServiceEntry(const std::string& serviceName);
	~ServiceEntry();

	void RegisterEventHandler(Event event, Handler&& handler);
	void RegisterEventHandler(uint32_t nativeEventCode, HandlerNative&& handler);
	void SetExitCode(uint32_t exitCode);

	void Execute(uint32_t& err);
	void Exit();
};
