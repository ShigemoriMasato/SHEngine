#pragma once
#include "GPUBuffer.h"

namespace SHEngine {

	// @brief GPUBuffer用のクラス。作成と寿命管理を行う。GPUBufferの作成関数やemplace_back等で1行が長くなるので、その解決用として存在している。
	// @brief ただ、GPUBufferの数が多くなると管理が大変なので、CBVだけでも生ポインタでいいからメンバ変数に持っておくことをおすすめする。
	class BufferContainer {
	public:

		BufferContainer(uint32_t size = 32) { buffers_.reserve(size); }
		 
		// @brief GPUBufferの作成。作られた順番に0からインデックスが振られる。Copyするときはこのインデックスを指定する。
		// @param bufferType バッファの種類（CBV、SRV、UAVの組み合わせ）
		// @param byte バッファのサイズ（バイト単位）
		// @param num バッファの数（デフォルトは1）
		// @param bufferNum バッファの数（デフォルトは3、スワップチェーンのバッファ数に合わせる）
		GPUBuffer* Create(BufferType bufferType, size_t byte, uint32_t num = 1, BufferNum bufferNum = BufferNum::MatchSwapChain) {
			auto& buffer = buffers_.emplace_back(std::make_unique<GPUBuffer>(bufferType, byte, num, bufferNum));
			return buffer.get();
		}

		GPUBuffer* Create(TextureData* textureData) {
			auto& buffer = buffers_.emplace_back(std::make_unique<GPUBuffer>(textureData));
			return buffer.get();
		}

		// @brief GPUBufferへのデータコピー。indexはCreateしたときの順番。UAVバッファにはコピーできない。
		// @param index GPUBufferのインデックス。Createされた順番で0から振られる。
		// @param data コピーするデータのポインタ。
		// @param dataSize コピーするデータのサイズ（バイト単位）
		void Copy(uint32_t index, const void* data, size_t dataSize) {
			assert(index < buffers_.size());
			buffers_[index]->CopyBuffer(data, dataSize);
		}

	private:

		std::vector<std::unique_ptr<GPUBuffer>> buffers_;

	};

}
