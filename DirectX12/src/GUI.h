#pragma once
#include <d3d12.h>
#include <vector>
#include <string>


class GUI
{
private:
	static ID3D12DescriptorHeap* s_descriptorHeap;

	static std::vector<std::string> s_debugMessages;
	static std::vector<int> s_debugMessagesCount;

public:
	static void Init();
	static void Update();
	static void Render(ID3D12GraphicsCommandList* cmdList);
	static void Destroy();
	
	template<typename T>
	static void Debug(const T& message)
#ifndef _DEBUG
	{}
#else
	{
		Debug(std::to_string(message));
	}

	static void Debug(const std::string& message)
	{
		if (s_debugMessages.empty() || message != s_debugMessages.back())
		{
			s_debugMessages.push_back(message);
			s_debugMessagesCount.push_back(1);
		}
		else
		{
			s_debugMessagesCount.back()++;
		}
	}
#endif

};