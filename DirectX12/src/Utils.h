#pragma once
#include <string>

#ifdef _DEBUG
	#define ProjectPath "./"
#else
	#define ProjectPath "../../DirectX12/"
#endif


class Utils
{
public:
	static std::wstring GetWStringFromString(const std::string& str);

	Utils() = delete;
	~Utils() = delete;
};