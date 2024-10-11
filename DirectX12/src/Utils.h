#pragma once
#include <string>
#include <Windows.h>

#define NUM_BACK_BUFFERS 2

namespace Utils
{
	inline std::wstring GetWStringFromString(const std::string& str)
	{
		int num = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, str.c_str(), -1, nullptr, 0);
		std::wstring wstr;
		wstr.resize(num);

		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, str.c_str(), -1, &wstr[0], num);
		return wstr;
	}

};