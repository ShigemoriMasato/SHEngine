#pragma once
#include <SHEngine.h>
#include <Render/Renderer.h>

class PostEffectContext {
public:

	// 頂点などの静的リソースの初期化。Engine側で呼ばれる
	static void StaticInitialize(SHEngine::Engine* engine);

	// 静的リソースの解放。Engine側で呼ばれる
	static void StaticFinalize();

	// 中間リソースの生成
	void Initialize(uint32_t width, uint32_t height);

	// @brief ポストエフェクトの開始
	// @param display エフェクトをかけたいRTV
	void Begin(SHEngine::Screen::IDisplay* display);

	// @brief Beginで指定したdisplayに確実に絵を戻すための関数。
	// @param output display以外に絵を移したい場合に入力する。nullptrならdisplayにもどる
	void End(SHEngine::Screen::IDisplay* output = nullptr);


public:// PostEffectで使用する関数群

	// Beginが呼ばれていない場合はnullptrが返される。
	SHEngine::Screen::IDisplay* GetDestination() const { return display_ ? destination_ : nullptr; }
	// Beginが呼ばれていない場合はnullptrが返される。
	SHEngine::Screen::IDisplay* GetSource() const { return display_ ? source_ : nullptr; }

	// Beginが呼ばれていない場合はnullptrが返される。
	SHEngine::GPUBuffer* GetSourceIndex() const;

	SHEngine::Renderer* GetRenderer() const { return renderer_.get(); };
	DCC* GetDCC() const { return engine_->GetDirectCommandContext(); }

	void Toggle() { std::swap(source_, destination_); }
	
private:

	void CommonEnd();

	static inline std::unique_ptr<SHEngine::GPUBuffer> vertexPos_;
	static inline std::unique_ptr<SHEngine::GPUBuffer> vertexUV_;
	static inline std::unique_ptr<SHEngine::Renderer> renderer_;
	static inline SHEngine::Engine* engine_;
	static inline Logger logger_;

	//中間リソース
	std::unique_ptr<SHEngine::Screen::Display> intermediateDisplay_;
	std::unique_ptr<SHEngine::GPUBuffer> intermediateIndexBuffer_;

	// 受け取ったRTV
	SHEngine::Screen::IDisplay* display_ = nullptr;
	std::unique_ptr<SHEngine::GPUBuffer> displayIndexBuffer_;

	// エフェクト元の絵がある側
	SHEngine::Screen::IDisplay* source_ = nullptr;
	// 次の描画先
	SHEngine::Screen::IDisplay* destination_ = nullptr;

};
