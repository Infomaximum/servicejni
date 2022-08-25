#pragma once

#include <string>
#include <codecvt>
#include <locale>
#include <algorithm> 
#include <cwctype>

struct StringConverter
{
	static std::string UTF16toUTF8(const std::wstring& value)
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		return converter.to_bytes(value);
	}

	static std::wstring UTF8toUTF16(const std::string& value)
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		return converter.from_bytes(value);
	}

	static void ltrim(std::wstring& s) {
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
			return !std::iswspace(ch);
		}));
	}

	static void rtrim(std::wstring& s) {
		s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
			return !std::iswspace(ch);
		}).base(), s.end());
	}

	static void trim(std::wstring& s) 
	{
		ltrim(s);
		rtrim(s);
	}
};
