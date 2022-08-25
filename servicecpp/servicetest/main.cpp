#include "gtest/gtest.h"
#include "TestServiceEntry.h"

int main(int argc, char* argv[])
{
	using namespace test;

	setCurrentExeFile(argv[0]);

	if (argc == 2 && std::string(argv[1]) == RUN_AS_SERVICE)
	{
		return executeAsService();
	}

	::testing::InitGoogleTest(&argc, argv);
	int res = RUN_ALL_TESTS();
	system("Pause");
	return res;
}