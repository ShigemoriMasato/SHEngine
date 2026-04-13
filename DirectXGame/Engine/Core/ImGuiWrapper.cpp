#include "ImGuiWrapper.h"
#include <imgui/imgui_impl_dx12.h>
#include <imgui/imgui_impl_win32.h>
#include <filesystem>

using namespace SHEngine;

void ImGuiWrapper::Initialize(DXDevice* device, Command::Manager* manager, Screen::WindowsAPI* window, Command::Object* cmdObject) {
	logger_ = getLogger("ImGui", LoggerFlag::UseDebugString);

#ifdef USE_IMGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(window->GetHwnd());
	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = "Assets/ImGui/imgui.ini"; // 設定ファイルのパスを指定
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // ドッキングを有効化
	std::filesystem::create_directory("Assets/ImGui"); // ini用ディレクトリを作成
	io.Fonts->AddFontFromFileTTF("Assets/.EngineResource/Fonts/MPLUS1p-Medium.ttf", 17.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
	
	ImGui_ImplDX12_InitInfo initInfo;
	initInfo.Device = device->GetDevice();
	initInfo.NumFramesInFlight = bufferNum_;
	initInfo.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	initInfo.CommandQueue = manager->GetCommandQueue(Command::Type::Direct);
	initInfo.SrvDescriptorHeap = device->GetSRVManager()->GetHeap();

	srvHandles_.resize(bufferNum_);
	for (int i = 0; i < bufferNum_; ++i) {
		srvHandles_[i].UpdateHandle(device->GetSRVManager());
	}

	initInfo.LegacySingleSrvCpuDescriptor = srvHandles_.front().GetCPU();
	initInfo.LegacySingleSrvGpuDescriptor = srvHandles_.front().GetGPU();

	ImGui_ImplDX12_Init(&initInfo);

	device_ = device;
	cmdObject_ = cmdObject;

	logger_->info("ImGui Activate");

	return;
#endif

	logger_->warn("ImGui is not activated");
}

void ImGuiWrapper::NewFrame() {
#ifdef USE_IMGUI
	ImGuiIO& io = ImGui::GetIO();

	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(1280.0f, 720.0f), ImGuiCond_Always);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f); // 角丸をなくす

	ImGui::Begin("DockSpaceWindow", nullptr, ImGuiWindowFlags_NoTitleBar | // タイトルバーなし 
		ImGuiWindowFlags_NoResize | // リサイズ不可
		ImGuiWindowFlags_NoMove | // 移動不可
		ImGuiWindowFlags_NoScrollbar | // スクロールバーなし
		ImGuiWindowFlags_NoCollapse | // 折り畳み不可
		ImGuiWindowFlags_NoBackground | // 背景なし（必要に応じて）
		ImGuiWindowFlags_NoSavedSettings);

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");

	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

	ImGui::End();
	ImGui::PopStyleVar(2);

#endif
}

void ImGuiWrapper::Render() {
#ifdef USE_IMGUI
	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmdObject_->GetCommandList());

#endif
}

void ImGuiWrapper::EndFrame() {
#ifdef USE_IMGUI
	ImGui::EndFrame();
#endif
}

void ImGuiWrapper::Finalize() {
#if USE_IMGUI
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif
}
