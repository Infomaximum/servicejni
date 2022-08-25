#pragma once

#include "Sync.hpp"
#include "LimitSingleInstance.h"

ServiceEntry::ServiceEntry(const String& serviceName)
    : serviceName(std::move(String() << serviceName))
	, sync(true)
{
    pInstance = this;
}
	
void ServiceEntry::RegisterEventHandler(Event event, Handler handler)
{
    Assert::Throw(handler != nullptr, "handler is empty");

    switch (event)
    {
        case Event::STARTED:
            ServiceEntry::onStartedCallback = handler;
            break;
        case Event::STOPPING:
            ServiceEntry::onStoppingCallback = handler;
            break;
        case Event::STOPPED:
            ServiceEntry::onStoppedCallback = handler;
            break;
    }
}
	
void ServiceEntry::RegisterEventHandler(DWORD nativeEventCode, Handler handler)
{
    Assert::Throw(handler != nullptr, "handler is empty");
    ServiceEntry::eventHandlers.Add(nativeEventCode) = handler;
}

DWORD ServiceEntry::Run()
{
    pid_t pid = fork();
    if (pid < 0)
        exit(EXIT_FAILURE);

    if (pid > 0)
        exit(EXIT_SUCCESS);

    if (setsid() < 0)
        exit(EXIT_FAILURE);

    RegisterHandlers();

    pid = fork();
    if (pid < 0)
        exit(EXIT_FAILURE);

    if (pid > 0)
        exit(EXIT_SUCCESS);

    umask(0);
    chdir("/");

    /* Close all open file descriptors */
    for (int x = sysconf(_SC_OPEN_MAX); x > 0; x--)
    {
        close(x);
    }

    LimitSingleInstance singleInstance(serviceName.GetValue(), "/var/run");
    if (!singleInstance.IsSingleInstanceRunning())
    {
        return EXIT_FAILURE;
    }

    if (onStartedCallback)
    {
        onStartedCallback();
    }

    ServiceEntry::sync.Wait();

    if (onStoppedCallback)
    {
        onStoppedCallback();
    }

    return EXIT_SUCCESS;
}

void ServiceEntry::Exit(DWORD /*errorCode*/)
{
    Assert::Throw(false, "not implemented");
}

void ServiceEntry::RegisterHandlers()
{
    signal(SIGTERM, &ServiceEntry::Signalhandler);

	for (auto it = pInstance->eventHandlers.begin(); it.Exists(); ++it)
	{
		signal(it.GetKey(), &ServiceEntry::Signalhandler);
	}
}
	
void ServiceEntry::Signalhandler(int signal)
{
    //syslog(LOG_NOTICE, (String() << "received signal " << signal).GetValue());

    switch (signal)
    {
    case SIGTERM:
        if (pInstance->onStoppingCallback)
        {
            pInstance->onStoppingCallback();
        }
		pInstance->sync.Signal();
        break;
    default:
        auto handlerIt = pInstance->eventHandlers.Find(signal);
        if (handlerIt.Exists())
        {
            handlerIt.GetValue()();
        }
        break;
    }
}


ServiceEntry* ServiceEntry::pInstance = nullptr;
