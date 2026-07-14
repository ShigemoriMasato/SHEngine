#pragma once
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <assimp/scene.h>
#include <Tool/Logger/Logger.h>
#include <Render/DrawDataManager.h>

#include "ModelData.h"

namespace SHEngine {

	class TextureManager;

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
		void Initialize(TextureManager* textureManager, DrawDataManager* drawDataManager);

		/**
		 * @brief 3Dモデルを読み込む
		 * @param filePath モデルファイルのパス
		 * @return モデルID
		 */
		int LoadModel(std::string filePath);

		/**
		 * @brief すべてのモデルを読み込む
		 */
		void LoadAllModels();

		/**
		 * @brief アニメーションデータを読み込む
		 * @param filePath アニメーションファイルのパス
		 * @param index アニメーションインデックス
		 * @return アニメーションデータ
		 */
		Animation LoadAnimation(std::string filePath, int index);

		/**
		 * @brief ノードモデルデータを取得（失敗時はLogとキューブを返す）
		 * @param id モデルID
		 * @return NodeModelDataの参照
		 */
		NodeModelData& GetNodeModelData(int id);

		/**
		 * @brief スキニングモデルデータを取得（失敗時はLogとキューブを返す）
		 * @param id モデルID
		 * @return SkinningModelDataの参照
		 */
		SkinningModelData& GetSkinningModelData(int id);

		// @brief モデルIDからポリゴンリストを取得
		PolygonList CreatePolygonList(int id);
		// @brief モデル名からポリゴンリストを取得
		PolygonList CreatePolygonList(std::string modelPath);

	private:

		/**
		 * @brief ファイルパスをチェックし、正しい形式に修正する
		 * @param filePath ファイルパス
		 * @return 修正されたファイル名
		 */
		std::string FilePathChecker(std::string& filePath);

		/**
		 * @brief ノードモデルデータを書き込む
		 * @param scene Assimpシーン
		 * @param filePath ファイルパス
		 * @return ノードモデルデータ
		 */
		NodeModelData WritingNodeModelData(const aiScene* scene, std::string filePath);

		/**
		 * @brief スキニングモデルデータを書き込む
		 * @param scene Assimpシーン
		 * @param filePath ファイルパス
		 * @return スキニングモデルデータ
		 */
		SkinningModelData WritingSkinningModelData(const aiScene* scene, std::string filePath);

	private://Model

		/// @brief モデルファイルパスとIDのマップ
		std::unordered_map<std::string, int> modelFilePaths_{};

		/// @brief ノードモデルデータのリスト
		std::vector<NodeModelData> nodeModelDatas_{};
		/// @brief スキニングモデルデータのリスト
		std::vector<SkinningModelData> skinningModelDatas_{};

		/// @brief 次に割り当てるID
		int nextID_ = 0;

		/// @brief テクスチャマネージャーへのポインタ
		TextureManager* textureManager_;
		/// @brief 描画データマネージャーへのポインタ
		DrawDataManager* drawDataManager_;

	private://Animation

		/// @brief アニメーションデータのマップ
		std::unordered_map<std::string, std::vector<Animation>> animations_{};

	private://Debug

		/// @brief ロガー
		Logger logger_;

	};

}

Matrix4x4 AnimationUpdate(const Animation& animation, float time, const Node& node);
void AnimationUpdate(const Animation& animation, float time, Skeleton& skeleton);
void SkeletonUpdate(Skeleton& skeleton);
void SkinningUpdate(std::vector<WellForGPU>& result, std::map<std::string, JointWeightData> skinCluster, const Skeleton& skeleton);

Vector3 CalculateValue(const std::vector<KeyframeVector3>& keyframes, float time);
Quaternion CalculateValue(const std::vector<KeyframeQuaternion>& keyframes, float time);
