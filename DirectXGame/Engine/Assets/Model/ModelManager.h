#pragma once
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <assimp/scene.h>
#include <Tool/Logger/Logger.h>

#include "ModelData.h"

namespace SHEngine {

	class TextureManager;

	enum class TestModel {
		Cube,
		Plane,
		Sphere,
		SimpleSkin,
		Desc,
		Camera,
		Field,
		Tower
	};

	/**
	 * @class ModelManager
	 * @brief 3Dモデルの読み込みと管理を行うクラス
	 *
	 * Assimpを使用して様々な形式の3Dモデルを読み込み、
	 * ノード構造とスキニングの両方に対応。
	 * アニメーションデータの読み込みもサポートする。
	 */
	class ModelManager {
	public:

		/// @brief デフォルトコンストラクタ
		ModelManager() = default;
		/// @brief デストラクタ
		~ModelManager() = default;

		/**
		 * @brief モデルマネージャーを初期化
		 * @param textureManager テクスチャマネージャー
		 * @param drawDataManager 描画データマネージャー
		 */
		void Initialize(TextureManager* textureManager);

		/**
		 * @brief 3Dモデルを読み込む
		 * @param filePath モデルファイルのパス
		 * @return モデルID
		 */
		const ModelData* LoadModel(std::string filePath);

		const ModelData* GetModelData(TestModel model);

		/**
		 * @brief Assets/Models/以下にあるすべてのモデルファイルを読み込む
		 */
		void LoadAllModels();

		/**
		 * @brief アニメーションデータを読み込む
		 * @param filePath アニメーションファイルのパス
		 * @param index アニメーションインデックス
		 * @return アニメーションデータ
		 */
		Animation LoadAnimation(std::string filePath, std::string animationName = "");

	private:

		/**
		 * @brief ファイルパスをチェックし、正しい形式に修正する
		 * @param filePath ファイルパス
		 * @return 修正されたファイル名
		 */
		std::string FilePathChecker(std::string& filePath);

		ModelData CreateModelData(const aiScene* scene, std::string filePath);

	private://Model

		/// @brief モデルファイルパスとIDのマップ
		std::unordered_map<std::string, int> modelFilePaths_{};

		/// @brief ModelDataのリスト
		std::vector<std::unique_ptr<ModelData>> modelData_{};

		/// @brief テクスチャマネージャーへのポインタ
		TextureManager* textureManager_;

	private://Animation

		/// @brief アニメーションデータのマップ
		std::unordered_map<std::string, std::unordered_map<std::string, Animation>> animations_{};

	private://Debug

		/// @brief ロガー
		Logger logger_;

	};

}
