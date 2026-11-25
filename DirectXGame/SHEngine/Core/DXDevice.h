#pragma once
#include <cstdint>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxcapi.h>
#include <Tool/Logger/Logger.h>

#include <wrl.h>

#include "View/SRVManager.h"
#include "View/RTVManager.h"
#include "View/DSVManager.h"

/**
 * @enum ShaderType
 * @brief シェーダーの種類
 */
enum class ShaderType {
	VERTEX_SHADER,  ///< 頂点シェーダー
	PIXEL_SHADER,   ///< ピクセルシェーダー
	COMPUTE_SHADER, ///< コンピュートシェーダー
	MESH_SHADER,
	AMPLIFICATION_SHADER,

	Count           ///< シェーダータイプの総数
};

namespace SHEngine {

	struct ShaderReflectionData;

	/**
	 * @class DXDevice
	 * @brief DirectX12デバイスとその関連リソースを管理するクラス
	 *
	 * DirectX12のデバイス、DXGIファクトリ、アダプタの初期化と管理を行う。
	 * また、各種ディスクリプタヒープマネージャー（SRV、RTV、DSV）やPSOエディタを保持し、
	 * グラフィックスパイプラインの基盤を提供する。
	 */
	class DXDevice {
	public:

		/// @brief デフォルトコンストラクタ
		DXDevice() = default;

		/// @brief デストラクタ
		~DXDevice();

		/**
		 * @brief DirectX12デバイスと関連リソースを初期化する
		 *
		 * デバッグレイヤーの有効化、DXGIファクトリの作成、アダプタの選択、
		 * デバイスの作成、ディスクリプタヒープマネージャーの初期化を行う。
		 */
		void Initialize();

		/// @brief DirectX12デバイスを取得
		/// @return ID3D12Device*
		ID3D12Device2* GetDevice() const { return device_.Get(); }

		/// @brief DXGIファクトリを取得
		/// @return IDXGIFactory7*
		IDXGIFactory7* GetDxgiFactory() const { return dxgiFactory_.Get(); }

		/// @brief SRV/CBV/UAVディスクリプタヒープマネージャーを取得
		/// @return SRVManager*
		SRVManager* GetSRVManager() const { return srvManager_.get(); }

		/// @brief RTVディスクリプタヒープマネージャーを取得
		/// @return RTVManager*
		RTVManager* GetRTVManager() const { return rtvManager_.get(); }

		/// @brief DSVディスクリプタヒープマネージャーを取得
		/// @return DSVManager*
		DSVManager* GetDSVManager() const { return dsvManager_.get(); }

		/// @brief シェーダーをコンパイルする
		/// @param filePath シェーダーファイルのパス
		/// @param shaderType シェーダーの種類
		/// @param outReflection nullptrでなければ、リフレクション結果を書き込む(Releaseビルドでも取得できる)
		/// @param outIncludes nullptrでなければ、#includeで読み込んだファイルのパスを書き込む
		/// @return コンパイル済みBlob(参照カウント1。呼び出し側で解放すること)。失敗時はnullptr
		IDxcBlob* CompileShader(const std::string& filePath, ShaderType shaderType, ShaderReflectionData* outReflection = nullptr, std::vector<std::string>* outIncludes = nullptr);

		/// @brief シェーダーのコンパイル設定名。キャッシュの保存先を分けるために使う(Debugとそれ以外で設定が違う)
		static const char* GetShaderCompileConfigName();

		int GetBufferCount() const { return bufferCount; }

	private:

		/// @brief DirectX12デバッグコントローラー
		Microsoft::WRL::ComPtr<ID3D12Debug1> debugController_ = nullptr;
		/// @brief DXGIファクトリ
		Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_ = nullptr;
		/// @brief 使用するグラフィックスアダプタ
		Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter_ = nullptr;
		/// @brief DirectX12デバイス
		Microsoft::WRL::ComPtr<ID3D12Device2> device_ = nullptr;

		/// @brief SRV/CBV/UAVディスクリプタヒープマネージャー
		std::unique_ptr<SRVManager> srvManager_;
		/// @brief RTVディスクリプタヒープマネージャー
		std::unique_ptr<RTVManager> rtvManager_;
		/// @brief DSVディスクリプタヒープマネージャー
		std::unique_ptr<DSVManager> dsvManager_;

		const int bufferCount = 3;	// バッファ数（フレームバッファリングのため）

	private:

		/// @brief ロガー
		Logger logger_;

	private:

		//コンパイル関係
		Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_ = nullptr;
		Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_ = nullptr;
		Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_ = nullptr;

		std::map<ShaderType, std::wstring> compileProfiles_ = {
			{ShaderType::VERTEX_SHADER, L"vs_6_5"},
			{ShaderType::PIXEL_SHADER, L"ps_6_5"},
			{ShaderType::COMPUTE_SHADER, L"cs_6_5"},
			{ShaderType::MESH_SHADER, L"ms_6_5"},
			{ShaderType::AMPLIFICATION_SHADER, L"as_6_5"}
		};

	};

}
