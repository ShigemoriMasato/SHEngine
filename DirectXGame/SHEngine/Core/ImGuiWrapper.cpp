#include "ImGuiWrapper.h"
#include <imgui/imgui_impl_dx12.h>
#include <imgui/imgui_impl_win32.h>
#include <filesystem>

using namespace SHEngine;

void ImGuiWrapper::Initialize(DXDevice* device, DirectCommandContext* directContext, Screen::WindowsAPI* window) {
	logger_ = GetLogger("ImGui", LoggerFlag::UseDebugString);

#ifdef USE_IMGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = "Assets/ImGui/imgui.ini"; // 設定ファイルのパスを指定
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // ドッキングを有効化
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // ウィンドウをメインウィンドウの外に出せるようにする(バックエンドの初期化より前に設定する)

	// メインウィンドウの外に出したウィンドウは OS のウィンドウになるので、角丸と透過をなくして見た目を揃える
	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowRounding = 0.0f;
	style.Colors[ImGuiCol_WindowBg].w = 1.0f;

	ImGui_ImplWin32_Init(window->GetHwnd());
	std::filesystem::create_directory("Assets/ImGui"); // ini用ディレクトリを作成
	io.Fonts->AddFontFromFileTTF("Assets/.EngineResource/Fonts/MPLUS1p-Medium.ttf", 17.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
	
	ImGui_ImplDX12_InitInfo initInfo;
	initInfo.Device = device->GetDevice();
	initInfo.NumFramesInFlight = bufferNum_;
	initInfo.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	initInfo.CommandQueue = directContext->GetCommandQueue();
	initInfo.SrvDescriptorHeap = device->GetSRVManager()->GetHeap();

	// テクスチャごとにSRVを確保・解放する(旧形式のLegacySingleSrv*は1枚しか扱えず、フォントアトラスの拡張でassertする)
	device_ = device;
	initInfo.UserData = this;
	initInfo.SrvDescriptorAllocFn = &ImGuiWrapper::AllocateSrvDescriptor;
	initInfo.SrvDescriptorFreeFn = &ImGuiWrapper::FreeSrvDescriptor;

	ImGui_ImplDX12_Init(&initInfo);

	logger_->info("ImGui Activate");

	return;
#endif

	logger_->warn("ImGui is not activated");
}

void ImGuiWrapper::NewFrame() {
#ifdef USE_IMGUI
	ImGuiIO& io = ImGui::GetIO();

	// 前フレームの外のウィンドウをここで更新・描画する。
	// Engine::BeginFrame から呼ばれるので、前フレームのメインのコマンドリストは実行済み
	if (isFrameStarted_) {
		RenderPlatformWindows();
	}
	isFrameStarted_ = true;
	isRendered_ = false;

	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();

	// マルチビューポートでは ImGui の座標がデスクトップ座標になるため、メインウィンドウの位置・サイズに合わせる
	const ImGuiViewport* mainViewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(mainViewport->Pos, ImGuiCond_Always);
	ImGui::SetNextWindowSize(mainViewport->Size, ImGuiCond_Always);
	ImGui::SetNextWindowViewport(mainViewport->ID);
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

void ImGuiWrapper::Render(ID3D12GraphicsCommandList* cmdList) {
#ifdef USE_IMGUI

	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmdList);
	isRendered_ = true;

#endif
}

void ImGuiWrapper::RenderPlatformWindows() {
#ifdef USE_IMGUI

	if (!isFrameStarted_ || !(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)) {
		return;
	}

	// Render されなかったフレームでも EndFrame と UpdatePlatformWindows は毎フレーム必要
	ImGui::EndFrame();
	ImGui::UpdatePlatformWindows();

	// 外のウィンドウはバックエンドが専用のコマンドリストで描画し、メインと同じキューで実行する。
	// ゲーム画面のテクスチャなどを参照するため、メインのコマンドリストを実行した後に呼ぶこと
	if (isRendered_) {
		ImGui::RenderPlatformWindowsDefault();
	}
	isRendered_ = false;
	isFrameStarted_ = false;

#endif
}
                                                                                                                                                                      
void ImGuiWrapper::EndFrame() {
#ifdef USE_IMGUI
	ImGui::EndFrame();
#endif
}

void ImGuiWrapper::AllocateSrvDescriptor(ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE* outCpu, D3D12_GPU_DESCRIPTOR_HANDLE* outGpu) {
	auto* self = static_cast<ImGuiWrapper*>(info->UserData);
	auto& handle = self->srvHandles_.emplace_back(std::make_unique<SRVHandle>());
	handle->UpdateHandle(self->device_->GetSRVManager());
	*outCpu = handle->GetCPU();
	*outGpu = handle->GetGPU();
}

void ImGuiWrapper::FreeSrvDescriptor(ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE cpu, D3D12_GPU_DESCRIPTOR_HANDLE) {
	auto* self = static_cast<ImGuiWrapper*>(info->UserData);
	// SRVHandleのデストラクタでSRVManagerの使用中フラグが外れる
	std::erase_if(self->srvHandles_, [cpu](const std::unique_ptr<SRVHandle>& handle) {
		return handle->GetCPU().ptr == cpu.ptr;
		});
}

void ImGuiWrapper::Finalize() {
#if USE_IMGUI
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif
}

void ImGuizmo::OriginalSetRect(SHEngine::Screen::IDisplay* display) {
#ifdef USE_IMGUI
	// display->GetPos() は ImGui 上の位置(デスクトップ座標)。display が無い場合はメインウィンドウの左上
	const ImVec2 mainPos = ImGui::GetMainViewport()->Pos;
	Vector2 pos = display ? display->GetPos() : Vector2(mainPos.x, mainPos.y);
	Vector2 size = display ? display->GetSize() : Vector2(1280.0f, 720.0f);

	ImGuizmo::SetRect(pos.x, pos.y, size.x, size.y);
#endif
}
