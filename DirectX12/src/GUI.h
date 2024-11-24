#pragma once
#include <d3d12.h>
#include <vector>
#include <string>
#include <wrl/client.h> 
#include <type_traits>

using Microsoft::WRL::ComPtr;

class GUI
{
public:
	static void Init();
	static void Update();
	static void Render(ID3D12GraphicsCommandList* cmdList);
	static void Destroy();
	static inline bool IsCursorShown() { return s_cursorShown; }
	static inline bool IsCursorInsideClient() { return s_cursorInsideClient; }

private:
	static bool s_cursorShown;
	static bool s_cursorInsideClient;

	static ComPtr<ID3D12DescriptorHeap> s_descriptorHeap;

	static std::vector<std::string> s_debugMessages;
	static std::vector<int> s_debugMessagesCount;

public:
	static void Debug(const std::string& message)
	{
#ifdef _DEBUG
		if (s_debugMessages.empty() || message != s_debugMessages.back())
		{
			s_debugMessages.push_back(message);
			s_debugMessagesCount.push_back(1);
		}
		else
		{
			s_debugMessagesCount.back()++;
		}
#endif
	}
	static void Debug(const char* message)
	{
		GUI::Debug(std::string(message));

	}
	static void Debug(bool value)
	{
		std::string str = value ? "True" : "False";
		GUI::Debug(str);
	}

	template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
	static void Debug(T value)
	{
		GUI::Debug(std::to_string(value));
	}
};