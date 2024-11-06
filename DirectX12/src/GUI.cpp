#include "GUI.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx12.h"
#include "Utils.h"
#include "Window.h"
#include "GraphicDevice.h"

ID3D12DescriptorHeap* GUI::s_descriptorHeap = nullptr;

void GUI::Init()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.Fonts->AddFontFromFileTTF("assets/font/ARIAL.TTF", 15);
	io.Fonts->Build();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(Window::GetHWND());

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.NumDescriptors = 1;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	ID3D12Device* device = GraphicDevice::GetDevice();
	device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&s_descriptorHeap));
	ImGui_ImplDX12_Init(device, NUM_BACK_BUFFERS, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, s_descriptorHeap, s_descriptorHeap->GetCPUDescriptorHandleForHeapStart(), s_descriptorHeap->GetGPUDescriptorHandleForHeapStart());
}

void GUI::Update()
{
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(100, 40), ImGuiCond_Always);
	ImGui::Begin("FPS Counter", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground);
	ImGui::Text("%d", (int)ImGui::GetIO().Framerate);
	ImGui::End();
}

void GUI::Render(ID3D12GraphicsCommandList* cmdList)
{
	cmdList->SetDescriptorHeaps(1, &s_descriptorHeap);
	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmdList);
}

void GUI::Destroy()
{
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	if (s_descriptorHeap)
	{
		s_descriptorHeap->Release();
		s_descriptorHeap = nullptr;
	}
}

