#include "Utils.h"
#include <Windows.h>

std::wstring Utils::GetWStringFromString(const std::string& str)
{
	int num = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, str.c_str(), -1, nullptr, 0);
	std::wstring wstr;
	wstr.resize(num);

	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, str.c_str(), -1, &wstr[0], num);
	return wstr;
}