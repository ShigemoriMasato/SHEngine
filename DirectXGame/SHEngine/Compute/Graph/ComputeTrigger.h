#pragma once
#include <Compute/Graph/ComputeGraph.h>

namespace SHEngine {

	// @brief コードから好きなタイミングでComputeGraphを実行するための入口。
	// @note 名前ごとにComputeGraphが1つ対応し、どのComputeShaderをどの順番で、どのリソースで実行するかは
	//       ノード画面 ("ComputeGraph" ウィンドウ) で設定する。同じ名前のComputeTriggerが複数あってもグラフは共有され、
	//       Bindした引数だけがインスタンスごとに異なる。
	//
	//   trigger_ = std::make_unique<ComputeTrigger>("BattleField/Update");
	//   trigger_->Bind("Velocities", velocities_);	// ノード画面の「引数ノード」に出てくる
	//   trigger_->SetCount("Particles", count);		// Dispatch数に使える
	//   trigger_->Fire(computeContext_);
	class ComputeTrigger {
	public:

		// @param name グラフの名前。"/" で区切ると保存先がフォルダ分けされる
		explicit ComputeTrigger(const std::string& name);
		~ComputeTrigger();

		ComputeTrigger(const ComputeTrigger&) = delete;
		ComputeTrigger& operator=(const ComputeTrigger&) = delete;

		// @brief 引数ノードに渡すバッファを設定する。nullptrで解除
		void Bind(const std::string& argumentName, GPUBuffer* buffer);
		// @brief Dispatch数(ArgumentCount)に使う数を設定する
		void SetCount(const std::string& countName, uint32_t count);
		// @brief 何もつながっていないピンに使うバッファを設定する(レジスタ番号で指定)。nullptrで解除
		// @param bufferType CBV/SRV/UAVのどれか。テクスチャのピンにはSRV(RWTextureはUAV)で指定する
		// @param label ノード画面に「既定: label」と表示される名前
		// @note ParticleのEmitterのように、どのシェーダーでも同じレジスタに同じバッファを使う場合に、毎回つなぐ手間を省く
		void BindDefault(BufferType bufferType, uint32_t registerIndex, GPUBuffer* buffer, const std::string& label = "");

		// @brief グラフに設定されたComputeShaderを順に実行する
		void Fire(ICommandContext* context);

		const std::string& GetName() const { return name_; }
		const ComputeArguments& GetArguments() const { return arguments_; }
		ComputeGraph* GetGraph() const { return graph_; }
		// @brief 最後にFireしたフレーム (未実行ならUINT32_MAX)
		uint32_t GetLastFireFrame() const { return lastFireFrame_; }

	private:

		std::string name_;
		ComputeGraph* graph_ = nullptr;
		ComputeArguments arguments_;
		uint32_t lastFireFrame_ = UINT32_MAX;
	};

}
