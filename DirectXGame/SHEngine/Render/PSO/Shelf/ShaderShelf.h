#pragma once
#include <Core/DXDevice.h>
#include <unordered_map>
#include <filesystem>
#include <array>
#include <list>

namespace SHEngine {

	namespace PSO {

		/**
		 * @class ShaderShelf
		 * @brief シェーダーのコンパイルと管理を行うクラス
		 *
		 * DXC(DirectX Shader Compiler)を使用してHLSLシェーダーをコンパイルし、
		 * コンパイル済みシェーダーを名前で管理する。
		 */
		class ShaderShelf {
		public:

			ShaderShelf(DXDevice* device);
			~ShaderShelf();

			/**
			 * @brief 全シェーダーのコンパイル
			 *
			 * Assets/Shaderフォルダ内の全HLSLファイルを検索し、コンパイルして登録する。
			 */
			void CompileAllShader();

			/**
			 * @brief 指定タイプの全シェーダーのバイトコードリストを取得
			 *
			 * @param shaderType シェーダーの種類
			 * @return シェーダーバイトコードのリスト
			 */
			std::list<D3D12_SHADER_BYTECODE> GetShaderBytecodes(ShaderType shaderType);

			/**
			 * @brief 指定タイプの全シェーダー名のリストを取得
			 *
			 * @param shaderType シェーダーの種類
			 * @return シェーダー名のリスト
			 */
			std::list<std::string> GetShaderNames(ShaderType shaderType);

			/**
			 * @brief 指定名のシェーダーバイトコードを取得
			 *
			 * @param shaderType シェーダーの種類
			 * @param shaderName シェーダー名
			 * @return シェーダーバイトコード
			 */
			D3D12_SHADER_BYTECODE GetShaderBytecode(ShaderType shaderType, std::string shaderName);

		private:

			/**
			 * @brief シェーダーバイトコードを登録
			 *
			 * @param shaderName シェーダーファイル名
			 * @param shaderType シェーダーの種類
			 */
			void RegisterShaderByteCode(std::string shaderName, ShaderType shaderType);

			DXDevice* device_ = nullptr;

			/// @brief シェーダーファイルのベースパス
			std::filesystem::path basePath_ = "Assets/Shader";

			/// @brief シェーダータイプ毎のコンパイルバージョン
			std::array<std::wstring, static_cast<size_t>(ShaderType::Count)> compileVersions_;

			/// @brief コンパイル済みシェーダーBlobのリスト
			std::list<IDxcBlob*> shaderBlobs_;

			/// @brief シェーダータイプ別のバイトコードマップ(名前->Bytecode)
			std::array<std::unordered_map<std::string, D3D12_SHADER_BYTECODE>, static_cast<size_t>(ShaderType::Count)> shaderBytecodes_;
		};

	}// namespace PSO

} // namespace SHEngine
