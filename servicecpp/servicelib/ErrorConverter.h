#pragma once

#include <string>

class ErrorConverter
{
public:

	static std::string toString(uint32_t errorCode)
	{
		const std::string message = ConvertErrorToString(errorCode);
		std::string formattedMessage = ConvertCodeToString(errorCode);

		if (!message.empty())
		{
			formattedMessage.append("\n").append(message);
		}

		return formattedMessage;
	}

private:

	static std::string ConvertErrorToString(uint32_t errorCode);
	static std::string ConvertCodeToString(uint32_t errorCode);
};