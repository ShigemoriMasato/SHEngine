#pragma once
#include <Core/DXDevice.h>
#include "ShaderReflection.h"
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

			/**
			 * @brief 指定名のシェーダーのリフレクション結果を取得
			 *
			 * 未コンパイルの場合はコンパイルする。
			 *
			 * @param shaderType シェーダーの種類
			 * @param shaderName シェーダー名
			 * @return リフレクション結果。コンパイルに失敗した場合はnullptr
			 */
			const ShaderReflectionData* GetShaderReflection(ShaderType shaderType, const std::string& shaderName);

			/**
			 * @brief シェーダーをリセットする
			 *
			 * @param shaderName シェーダー名
			 */
			void ShaderReset(const std::string& shaderName);

		private:

			/**
			 * @brief シェーダーバイトコードを登録
			 *
			 * @param shaderName シェーダーファイル名
			 * @param shaderType シェーダーの種類
			 */
			void RegisterShaderByteCode(std::string shaderName, ShaderType shaderType);

			void ReleaseShader(std::string shaderName = "All");

		public:

			/**
			 * @brief ファイル名の拡張子からシェーダーの種類を判定する
			 *
			 * "名前.VS.hlsl" のように、".hlsl" の直前の拡張子(VS/PS/CS/MS/AS)で判定する。
			 *
			 * @param fileName シェーダーファイル名(パス可)
			 * @return シェーダーの種類。判定できない場合はShaderType::Count
			 */
			static ShaderType GetShaderTypeFromFileName(const std::string& fileName);

		private:

			DXDevice* device_ = nullptr;

			/// @brief シェーダーファイルのベースパス
			std::filesystem::path basePath_ = "Assets/Shader";

			struct ShaderEntry {
				/// @brief コンパイル済みシェーダーの中身。bytecodeはこの中を指す
				std::vector<uint8_t> bytes;
				D3D12_SHADER_BYTECODE bytecode{};
				/// @brief リソースのバインド情報(必要なCBV/SRV/UAV/Texture/Samplerとレジスタ番号)
				ShaderReflectionData reflection;
			};

			/// @brief 登録済みのEntryを取得する。未登録ならコンパイルして登録する
			ShaderEntry& GetOrRegister(ShaderType shaderType, const std::string& shaderName);

			/// @brief シェーダータイプ別のシェーダーマップ(名前->Entry)
			std::array<std::unordered_map<std::string, ShaderEntry>, static_cast<size_t>(ShaderType::Count)> shaders_;

			Logger logger_ = GetLogger("Engine");
		};

	}// namespace PSO

} // namespace SHEngine
