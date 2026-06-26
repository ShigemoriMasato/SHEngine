#pragma once
#include <Assets/Fonts/FontLoader.h>
#include <Assets/Model/ModelManager.h>
#include <Assets/Texture/TextureManager.h>
#include <Core/Command/CommandObject.h>
#include <Core/DXDevice.h>
#include <Core/ImGuiWrapper.h>
#include <Input/Input.h>
#include <Render/Screen/SwapChain.h>
#include <Render/Screen/WindowsAPI.h>
#include <Tool/FPS/FPSObserver.h>
#include <Render/PSO/PSOEditor.h>
#include <Render/PSO/ShelfManager.h>
#include <Compute/PSO/CSPSOManager.h>
#include <Assets/Audio/AudioManager.h>
#include <Core/FrameCounter.h>
#include <Render/Command/DirectCommandContext.h>
#include <Compute/ComputeCommandContext.h>

namespace SHEngine {

	class Engine {
	public:
		~Engine();

		void Initialize(HINSTANCE hInstance);

		// エンジン側の終了命令
		bool IsLoop();

		// Inputとコマンドの更新
		void BeginFrame();
		// コマンドのクローズ
		void PostDraw();

		void StopGPU() {
			directCmdContext_->StopGPU();
		}

		// ImGuiの有効化
		void ImGuiActivate(Screen::WindowsAPI* window);

		// ImGuiの描画
		void DrawImGui(CmdObj* cmdObj);

	public: // Getter
		TextureManager* GetTextureManager() { return textureManager_.get(); }
		FontLoader* GetFontLoader() { return fontLoader_.get(); }
		ModelManager* GetModelManager() { return modelManager_.get(); }
		DrawDataManager* GetDrawDataManager() { return drawDataManager_.get(); }
		DirectCommandContext* GetDirectCommandContext() { return directCmdContext_.get(); }
		ComputeCommandContext* GetComputeCommandContext() { return computeCmdContext_.get(); }
		Input* GetInput() { return input_.get(); }
		FPSObserver* GetFPSObserver() { return fpsObserver_.get(); }
		float GetDeltaTime() { return fpsObserver_->GetDeltatime(); }

		HINSTANCE GetHInstance() const { return hInstance_; }

	private: // Engine内で完結するクラス
		D3DResourceLeakChecker resourceLeakChecker_;

		std::unique_ptr<DXDevice> device_;
		std::unique_ptr<ImGuiWrapper> imGuiWrapper_;
		std::unique_ptr<PSO::Editor> psoEditor_;
		std::unique_ptr<PSO::ShelfManager> shelfManager_;
		std::unique_ptr<PSO::CSPSOManager> csPsoManager_;

	private: // Engine外部からアクセスするクラス
		std::unique_ptr<TextureManager> textureManager_;
		std::unique_ptr<FontLoader> fontLoader_;
		std::unique_ptr<ModelManager> modelManager_;
		std::unique_ptr<DrawDataManager> drawDataManager_;
		std::unique_ptr<DirectCommandContext> directCmdContext_;
		std::unique_ptr<ComputeCommandContext> computeCmdContext_;

		std::unique_ptr<Input> input_;
		std::unique_ptr<FPSObserver> fpsObserver_;

	private: // その他系
		HINSTANCE hInstance_;
		MSG msg_{};
		bool imguiDrew_ = true;
		FrameCounter frameCounter_;
		Logger logger_;
	};

} // namespace SHEngine
