#include "SHEngine.h"
#include <Tool/Dump/CreateDump.h>
#include <Render/Screen/IDisplay.h>
#include <Render/RenderObject.h>
#include <Compute/ComputeObject.h>
#include <Render/Font/Text.h>
#include <Render/Renderer.h>

#pragma comment(lib, "Dbghelp.lib")

using namespace SHEngine;

static LONG WINAPI ClashHandler(EXCEPTION_POINTERS* pExceptionPointers) {
	Func::CreateDump(pExceptionPointers);
	return EXCEPTION_EXECUTE_HANDLER;
}

SHEngine::Engine::~Engine() {
	AudioManager::GetInstance()->Finalize();
	imGuiWrapper_->Finalize();
	CoUninitialize();
}

void Engine::Initialize(HINSTANCE hInstance) {
	SetUnhandledExceptionFilter(ClashHandler);

	logger_ = getLogger("Engine");
	logger_->info("Begin Engine Initialize");

	CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	device_ = std::make_unique<DXDevice>();
	device_->Initialize();

	directCmdContext_ = std::make_unique<DirectCommandContext>();
	directCmdContext_->Initialize(device_.get());

	textureManager_ = std::make_unique<TextureManager>();
	textureManager_->Initialize(device_.get());

	fontLoader_ = std::make_unique<FontLoader>();
	fontLoader_->Initialize(textureManager_.get());

	drawDataManager_ = std::make_unique<DrawDataManager>();
	drawDataManager_->Initialize(device_.get());

	modelManager_ = std::make_unique<ModelManager>();
	modelManager_->Initialize(textureManager_.get(), drawDataManager_.get());

	input_ = std::make_unique<Input>();
	input_->Initialize(hInstance);

	psoEditor_ = std::make_unique<PSO::Editor>();
	psoEditor_->Initialize(device_.get());

	csPsoManager_ = std::make_unique<PSO::CSPSOManager>();
	csPsoManager_->Initialize(device_.get(), psoEditor_->GetSamplers());

	Screen::IDisplay::SetDevice(device_.get());
	RenderObject::StaticInitialize(device_.get(), psoEditor_.get());
	Renderer::SetPSOEditor(psoEditor_.get(), device_->GetSRVManager()->GetStartPtr());
	GPUBuffer::SetDevice(device_.get());
	Text::SetFontLoader(fontLoader_.get());
	ComputeObject::StaticInitialize(csPsoManager_.get(), device_->GetSRVManager()->GetStartPtr());
	AudioManager::GetInstance()->Initialize();

	fpsObserver_ = std::make_unique<FPSObserver>();

	hInstance_ = hInstance;

	frameCounter_.Initialize();
}

bool Engine::IsLoop() {
	while (PeekMessage(&msg_, nullptr, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg_);
		DispatchMessage(&msg_);
	}
	return msg_.message != WM_QUIT;
}

void Engine::BeginFrame() {
	directCmdContext_->BeginFrame();
	input_->Update();
	fpsObserver_->TimeAdjustment();
	AudioManager::GetInstance()->Update();
	if (imGuiWrapper_) {
		imGuiWrapper_->NewFrame();
		imguiDrew_ = false;
	}
}

void Engine::PostDraw() {
	if (!imguiDrew_) {
		imGuiWrapper_->EndFrame();
		imguiDrew_ = true;
	}

	frameCounter_.Update();
}

void SHEngine::Engine::ImGuiActivate(Screen::WindowsAPI* window) {
	imGuiWrapper_ = std::make_unique<ImGuiWrapper>();
	imGuiWrapper_->Initialize(device_.get(), directCmdContext_.get(), window);
	imGuiWrapper_->NewFrame();
}

void SHEngine::Engine::DrawImGui(CmdObj* cmdObj) {
	if (imGuiWrapper_) {
		imGuiWrapper_->Render(cmdObj);
	}
}
