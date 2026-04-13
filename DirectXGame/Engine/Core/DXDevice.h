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

	Count           ///< シェーダータイプの総数
};

namespace SHEngine {

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
		ID3D12Device* GetDevice() { return device_.Get(); }

		/// @brief DXGIファクトリを取得
		/// @return IDXGIFactory7*
		IDXGIFactory7* GetDxgiFactory() { return dxgiFactory_.Get(); }

		/// @brief SRV/CBV/UAVディスクリプタヒープマネージャーを取得
		/// @return SRVManager*
		SRVManager* GetSRVManager() { return srvManager_.get(); }

		/// @brief RTVディスクリプタヒープマネージャーを取得
		/// @return RTVManager*
		RTVManager* GetRTVManager() { return rtvManager_.get(); }

		/// @brief DSVディスクリプタヒープマネージャーを取得
		/// @return DSVManager*
		DSVManager* GetDSVManager() { return dsvManager_.get(); }

		IDxcBlob* CompileShader(const std::string& filePath, ShaderType shaderType);

	private:

		/// @brief DirectX12デバッグコントローラー
		Microsoft::WRL::ComPtr<ID3D12Debug1> debugController_ = nullptr;
		/// @brief DXGIファクトリ
		Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_ = nullptr;
		/// @brief 使用するグラフィックスアダプタ
		Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter_ = nullptr;
		/// @brief DirectX12デバイス
		Microsoft::WRL::ComPtr<ID3D12Device> device_ = nullptr;

		/// @brief SRV/CBV/UAVディスクリプタヒープマネージャー
		std::unique_ptr<SRVManager> srvManager_;
		/// @brief RTVディスクリプタヒープマネージャー
		std::unique_ptr<RTVManager> rtvManager_;
		/// @brief DSVディスクリプタヒープマネージャー
		std::unique_ptr<DSVManager> dsvManager_;

	private:

		/// @brief ロガー
		Logger logger_;

	private:

		//コンパイル関係
		Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_ = nullptr;
		Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_ = nullptr;
		Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_ = nullptr;

		std::map<ShaderType, std::wstring> compileProfiles_ = {
			{ShaderType::VERTEX_SHADER, L"vs_6_0"},
			{ShaderType::PIXEL_SHADER, L"ps_6_0"},
			{ShaderType::COMPUTE_SHADER, L"cs_6_0"}
		};

	};

}
