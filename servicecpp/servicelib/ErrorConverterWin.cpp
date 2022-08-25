#include "ErrorConverter.h"
#include "StringConverter.h"

#include <Windows.h>

std::string ErrorConverter::ConvertErrorToString(uint32_t errorCode)
{
	if (errorCode == ERROR_SUCCESS)
	{
		return std::string();
	}

	const DWORD messageLen = 2 * 1024;
	wchar_t message[messageLen];
	std::string result;
	if (0 != FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), message, messageLen, NULL))
	{
		std::wstring msg(message);
		StringConverter::trim(msg);

		result = StringConverter::UTF16toUTF8(msg);
	}

	return result;
}

std::string ErrorConverter::ConvertCodeToString(uint32_t errorCode)
{
	if (errorCode == ERROR_SUCCESS)
	{
		return std::string();
	}

	char buffer[128];
	if (errorCode < 0x0000FFFF)
	{
		std::snprintf(buffer, sizeof(buffer), "No. %u", errorCode);
	}
	else
	{
		std::snprintf(buffer, sizeof(buffer), "No. 0x%08x", errorCode);
	}

	return std::string(buffer);
}