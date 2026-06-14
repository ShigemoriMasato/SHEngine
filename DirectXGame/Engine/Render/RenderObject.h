#pragma once
#include "DrawDataManager.h"
#include <Core/Command/CommandObject.h>
#include <Utility/Color.h>
#include <Camera/Camera.h>
#include <Render/PSO/PSOEditor.h>

namespace SHEngine {

	/**
	 * @class RenderObject
	 * @brief 描画オブジェクトを管理するクラス
	 *
	 * 頂点データ、インデックスデータ、定数バッファ、シェーダーリソースビューを管理し、
	 * DirectX12のパイプラインステートを使用してオブジェクトを描画する。
	 * インスタンシング描画にも対応する。
	 */
	class RenderObject {
	public:

		/**
		 * @brief コンストラクタ
		 * @param debugName デバッグ用の名前
		 */
		RenderObject(std::string debugName = "");

		/// @brief デストラクタ
		~RenderObject();

		/**
		 * @brief 静的初期化（デバイスの設定）
		 * @param device DirectX12デバイス
		 */
		static void StaticInitialize(DXDevice* device, PSO::Editor* psoEditor);

		/**
		 * @brief レンダーオブジェクトを初期化
		 */
		void Initialize();

		/**
		 * @brief 描画データを設定
		 * @param data 描画データ
		 */
		void SetDrawData(const DrawData& data);

		/**
		 * @brief 定数バッファビュー（CBV）を作成
		 * @param size バッファサイズ
		 * @param type シェーダータイプ
		 * @param debugName デバッグ用の名前
		 * @return CBVのインデックス
		 */
		int CreateCBV(size_t size, ShaderType type, std::string debugName = "");

		/**
		 * @brief シェーダーリソースビュー（SRV）を作成
		 * @param size バッファサイズ
		 * @param num 要素数
		 * @param type シェーダータイプ
		 * @param debugName デバッグ用の名前
		 * @return SRVのインデックス
		 */
		int CreateSRV(size_t size, uint32_t num, ShaderType type, std::string debugName = "");

		/**
		 * @brief バッファにデータをコピー
		 * @param index バッファインデックス
		 * @param data コピーするデータ
		 * @param size データサイズ
		 */
		void CopyBufferData(int index, const void* data, size_t size);

		/**
		 * @brief オブジェクトを描画
		 * @param window 描画先ウィンドウ
		 */
		void Draw(Command::Object* cmdObject);

		/**
		 * @brief テクスチャ使用フラグを設定
		 * @param useTexture テクスチャを使用するかどうか
		 */
		void SetUseTexture(bool useTexture);

		/// @brief パイプラインステート設定
		PSO::Config psoConfig_{};
		/// @brief インスタンス数
		uint32_t instanceNum_ = 1;

		/// @brief コピー禁止
		void operator=(const RenderObject& other) = delete;


	private://static

		/// @brief DirectX12デバイス（全インスタンスで共有）
		static DXDevice* device_;
		/// @brief PSOエディタ
		static PSO::Editor* psoEditor_;
		/// @brief ロガー
		static Logger logger_;
		/// @brief バッファ数
		constexpr static int bufferNum_ = 3;

	private://データ管理

		/// @brief 頂点バッファビューのリスト
		std::vector<D3D12_VERTEX_BUFFER_VIEW> vbv_{};
		/// @brief インデックスバッファビュー
		D3D12_INDEX_BUFFER_VIEW ibv_{};
		/// @brief インデックス数
		uint32_t indexNum_ = 0;

		/**
		* @struct BufferData
		* @brief マップされたバッファ情報
		*/
		struct BufferData {
			/// @brief マップされたメモリへのポインタ
			void* mapped = nullptr;
			/// @brief バッファサイズ
			size_t size = 0;
		};
		/// @brief バッファデータのリスト（二重配列：外側=バッファ種類、内側=スワップチェーン対応）
		std::vector<std::vector<BufferData>> bufferList_{};

		/// @brief 定数バッファのGPUアドレスリスト
		std::vector<std::vector<D3D12_GPU_VIRTUAL_ADDRESS>> cbvAddresses_{};
		/// @brief シェーダーリソースビューハンドルのリスト
		std::vector<std::vector<std::unique_ptr<SRVHandle>>> srvHandles_{};

		/// @brief スワップチェーンインデックス
		int index_ = 0;

	private://寿命管理

		/**
		 * @struct GPUBuffer
		 * @brief リソースラッパー
		 */
		struct Resource {
			/// @brief ID3D12ResourceのComポインタ
			Microsoft::WRL::ComPtr<ID3D12Resource> res = nullptr;
		};
		/// @brief リソースのリスト
		std::vector<Resource> resources_{};

	private://Debug

		/// @brief デバッグ用の名前
		std::string debugName_;

	};

}
