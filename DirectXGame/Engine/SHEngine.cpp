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

	cmdManager_ = std::make_unique<Command::Manager>();
	cmdManager_->Initialize(device_.get());

	textureManager_ = std::make_unique<TextureManager>();
	textureManager_->Initialize(device_.get(), cmdManager_.get());

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
	csPsoManager_->Initialize(device_.get());

	Screen::IDisplay::SetDevice(device_.get());
	RenderObject::StaticInitialize(device_.get(), psoEditor_.get());
	Renderer::SetPSOEditor(psoEditor_.get(), device_->GetSRVManager()->GetStartPtr());
	GPUBuffer::SetDevice(device_.get());
	Text::SetFontLoader(fontLoader_.get());
	ComputeObject::StaticInitialize(csPsoManager_.get());

	fpsObserver_ = std::make_unique<FPSObserver>();

	hInstance_ = hInstance;
}

bool Engine::IsLoop() {
	while (PeekMessage(&msg_, nullptr, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg_);
		DispatchMessage(&msg_);
	}
	return msg_.message != WM_QUIT;
}

void Engine::BeginFrame() {
	input_->Update();
	fpsObserver_->TimeAdjustment();
	if (imGuiWrapper_) {
		imGuiWrapper_->NewFrame();
		imguiDrawed_ = false;
	}
}

void Engine::PostDraw() {

	if (!imguiDrawed_) {
		imGuiWrapper_->EndFrame();
		imguiDrawed_ = true;
	}

}

std::unique_ptr<Screen::SwapChain> SHEngine::Engine::MakeWindow(Screen::WindowsAPI* windowsApi, uint32_t clearColor) {
	auto swapChain = std::make_unique<Screen::SwapChain>();
	swapChain->Initialize(device_.get(), textureManager_.get(), cmdManager_.get(), windowsApi, clearColor);
	return std::move(swapChain);
}
