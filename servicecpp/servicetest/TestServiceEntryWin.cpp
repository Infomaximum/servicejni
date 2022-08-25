#include "TestServiceEntry.h"
#include <servicelib/ServiceEntry.h>

namespace test
{
	uint32_t exitCodes = 0;

	uint32_t makeExitCode(ExitCodeMask code)
	{
		exitCodes |= code;
		return EXIT_CODE_WRAP + exitCodes;
	}

	int executeAsService()
	{
		ServiceEntry serviceEntry(SERVICE_NAME);

		serviceEntry.RegisterEventHandler(ServiceEntry::Event::STARTING, [&serviceEntry]()
		{
			serviceEntry.SetExitCode(makeExitCode(ExitCodeMask::STARTING));
		});

		serviceEntry.RegisterEventHandler(ServiceEntry::Event::STOPPING, [&serviceEntry]()
		{
			serviceEntry.SetExitCode(makeExitCode(ExitCodeMask::STOPPING));
		});

		serviceEntry.RegisterEventHandler(USER_CODE,
			[&serviceEntry](
#if defined(WINDOWS)
				uint32_t eventType, void* pEventData
#endif
				)
		{
			serviceEntry.SetExitCode(makeExitCode(ExitCodeMask::USER_CODE_ACCEPTED));
		});

		uint32_t res;
		serviceEntry.Execute(res);

		return static_cast<int>(res);
	}
}