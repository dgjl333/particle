#pragma once
#include <d3d12.h>
#include <vector>
#include <string>
#include <wrl/client.h> 

using Microsoft::WRL::ComPtr;

class GUI
{
public:
	static void Init();
	static void Update();
	static void Render(ID3D12GraphicsCommandList* cmdList);
	static void Destroy();
	static void Debug(const std::string& message);
	static void Debug(const char* message);
	static void Debug(bool value);

private:
	static ComPtr<ID3D12DescriptorHeap> s_descriptorHeap;

	static std::vector<std::string> s_debugMessages;
	static std::vector<int> s_debugMessagesCount;
};