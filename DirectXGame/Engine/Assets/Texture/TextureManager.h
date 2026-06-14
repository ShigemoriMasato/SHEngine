#pragma once
#include "TextureData.h"
#include <Core/DXDevice.h>
#include <Core/Command/CommandObject.h>
#include <map>
#include <unordered_map>
#include <memory>

namespace SHEngine {

	/**
	 * @class TextureManager
	 * @brief テクスチャの読み込みと管理を行うクラス
	 *
	 * ファイルからのテクスチャ読み込み、ウィンドウ用テクスチャ、
	 * スワップチェーン用テクスチャの作成をサポート。
	 * GPUへのアップロードと中間リソースの管理も行う。
	 */
	class TextureManager {
	public:

		/// @brief デフォルトコンストラクタ
		TextureManager() = default;
		/// @brief デストラクタ
		~TextureManager();

		/**
		 * @brief テクスチャマネージャーを初期化
		 * @param device DirectX12デバイス
		 * @param CmdListManager コマンドリストマネージャー
		 * @param cmdObj コマンドオブジェクト(渡した後使用不可能になるため気を付けること)
		 */
		void Initialize(DXDevice* device);

		/**
		 * @brief 全てのテクスチャをクリア
		 */
		void AllTextureClear();

		/**
		 * @brief すべてのテクスチャを読み込む
		 */
		void LoadAllTextures();

		/**
		 * @brief ファイルからテクスチャを読み込む
		 * @param filePath テクスチャファイルのパス
		 * @return テクスチャハンドル
		 */
		int LoadTexture(const std::string& filePath);

		/**
		 * @brief エラーテクスチャのハンドルを取得
		 * @return エラーテクスチャハンドル
		 */
		int GetErrorTextureHandle() const { return 2; }

		/**
		 * @brief ウィンドウ用テクスチャを作成
		 * @param width 幅
		 * @param height 高さ
		 * @param clearColor クリアカラー
		 * @return テクスチャハンドル
		 */
		int CreateWindowTexture(uint32_t width, uint32_t height, uint32_t clearColor);

		/**
		 * @brief スワップチェーン用テクスチャを作成
		 * @param resource スワップチェーンのリソース
		 * @return テクスチャハンドル
		 */
		int CreateSwapChainTexture(ID3D12Resource* resource, uint32_t clearColor);

		/**
		* @brief 深度テクスチャを作成
		* @param resource 深度テクスチャのリソース
		* @return テクスチャハンドル
		*/
		int CreateDepthTexture(ID3D12Resource* resource);

		/**
		 * @brief ビットマップテクスチャを作成
		 * @param width 幅
		 * @param height 高さ
		 * @param colorMap カラーマップ（RGBA形式の32ビットカラーの配列）
		 * @return テクスチャハンドル
		 */
		int CreateBitmapTexture(uint32_t width, uint32_t height, DirectX::ScratchImage& scratchImage);

		/**
		 * @brief テクスチャを削除
		 * @param handle テクスチャハンドル
		 */
		void DeleteTexture(int handle);

		/**
		 * @brief テクスチャを削除
		 * @param textureData テクスチャデータへのポインタ
		 */
		void DeleteTexture(TextureData* textureData);

		/**
		 * @brief ファイルからテクスチャを取得（未読込のときerrorテクスチャを返す）
		 * @param filePath テクスチャファイルのパス
		 * @return テクスチャハンドル
		 */
		TextureData* GetTextureData(std::string filePath);

		/**
		 * @brief テクスチャデータを取得
		 * @param handle テクスチャハンドル
		 * @return TextureDataへのポインタ
		 */
		TextureData* GetTextureData(int handle);

		/**
		 * @brief テクスチャリソースを取得
		 * @param handle テクスチャハンドル
		 * @return ID3D12ResourceのComポインタ
		 */
		Microsoft::WRL::ComPtr<ID3D12Resource> GetTextureResource(int handle) {
			return textureDataList_[handle]->textureResource_;
		}

		void UploadResources(CmdObj* cmdObj);

		void ClearIntermediateResource();

	private:

		/**
		 * @brief テクスチャ数の上限をチェック
		 * @param offset オフセット
		 */
		void CheckMaxCount(int offset);

		/// @brief DirectX12デバイスへのポインタ
		DXDevice* device_ = nullptr;
		/// @brief SRVマネージャーへのポインタ
		SRVManager* srvManager_ = nullptr;

		/// @brief 最大テクスチャ数
		const int maxTextureCount = 1024;

		/// @brief テクスチャデータのマップ（ハンドル → TextureData）
		std::map<int, std::unique_ptr<TextureData>> textureDataList_;

		struct Resource {
			Microsoft::WRL::ComPtr<ID3D12Resource> res;
		};
		/// @brief 中間リソース
		std::vector<Resource> intermediateResources_;
		/// テクスチャのアップロードに使用する素材のリスト（ハンドル、ScratchImageのペア）
		std::vector<std::pair<int, DirectX::ScratchImage>> uploadStandby_;
		/// @brief 読み込んだテクスチャパスのマップ（ファイルパス → ハンドル）
		std::unordered_map<std::string, int> loadedTexturePaths_;

		/// @brief エラーテクスチャハンドル
		int errorTextureHandle_ = -1;

		/// @brief ロガー
		Logger logger_ = nullptr;
	};

}
