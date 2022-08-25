#include <Windows.h>

#include "gtest/gtest.h"
#include <servicelib/ErrorConverter.h>

namespace ErrorConverterTest
{
	TEST(ErrorConverter, toString)
	{
		ASSERT_EQ(std::string(), ErrorConverter::toString(ERROR_SUCCESS));
		ASSERT_EQ(std::string(u8"No. 5\n�������� � �������."), ErrorConverter::toString(ERROR_ACCESS_DENIED));
		ASSERT_EQ(std::string(u8"No. 0x00030200\n���� ��� ������������ � ������ ������."), ErrorConverter::toString(STG_S_CONVERTED));
		ASSERT_EQ(std::string(u8"No. 0x00ffffff"), ErrorConverter::toString(0x00ffffff));
	}
}