#pragma once

#include <string>

namespace test
{
	const std::string RUN_AS_SERVICE("run_as_service");
	const std::string SERVICE_NAME("_test_cpp_service");
	const uint32_t USER_CODE = 192;

	const uint32_t EXIT_CODE_WRAP = 20000;

	enum ExitCodeMask : uint32_t
	{
		STARTING = 1,
		STOPPING = 2,
		USER_CODE_ACCEPTED = 4
	};

	int executeAsService();

	void setCurrentExeFile(const std::string& value);
}