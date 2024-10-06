#pragma once
#include <string>

#define ProjectPath "../../DirectX12/"

class Utils
{
public:
	static std::wstring GetWStringFromString(const std::string& str);

	Utils() = delete;
	~Utils() = delete;
};