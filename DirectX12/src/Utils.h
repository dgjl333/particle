#pragma once
#include <string>

#define ProjectPath "./"

class Utils
{
public:
	static std::wstring GetWStringFromString(const std::string& str);

	Utils() = delete;
	~Utils() = delete;
};