#pragma once
#include <vector>
#include <string>
#include <cstring>
#include <cstdint>
#include <type_traits>
#include "Value.h"

class BinaryManager {
public:

	bool Boot(const std::string& fileName);
	void BootRawData(const std::string& rawData);

	/// @brief Boot/Writeで使用するベースパスを変更する。デフォルトは "Assets/Binary/"
	/// @param basePath 末尾に'/'を含めること
	void SetBasePath(const std::string& basePath) { basePath_ = basePath; }

	//ポインタを渡した場合は指す先の値を登録する
	template<typename T>
	void Register(const T& data);
	void Write(const std::string& fileName);

	//Registerした順番で値を吐き出す。一回しか吐き出さない。
	//型・サイズの不一致や末尾超過で失敗した場合は false を返し、以降の読み込みもすべて失敗する。
	template<typename T>
	bool Reverse(T& data);
	template<typename T>
	bool Reverse(T*& data);
	template<typename T>
	bool Reverse(std::vector<T>& data);
	void Back() { readIndex_ = prevReadIndex_; }

	bool IsEmpty() const { return inputBuffer_.empty(); }
	/// @brief 読み込みに一度でも失敗したか
	bool IsFailed() const { return failed_; }

	std::string GetOutputRawData() const { return outputBuffer_; }
	std::string GetInputRawData() const { return inputBuffer_; }

private:

	/// @brief 現在位置のヘッダーを検証し、ペイロードサイズを返す。失敗時は failed_ を立てて false
	bool ReadHeader(TypeID expected, uint32_t& outSize);
	template<typename T>
	void RegisterValue(const T& data);

	// std::vector用コード
	template<typename T>
	void RegisterValue(const std::vector<T>& data);
	template<typename T>
	bool RegisterVectorValue(std::string& buffer, const T* data, uint32_t size);
	template<typename T>
	bool RegisterVectorValue(std::string& buffer, const std::vector<T>* data, uint32_t size);

	template<typename T>
	bool ReadVector(std::vector<T>& data, uint32_t size, uint32_t& readBuffer);
	template<typename T>
	bool ReadVector(std::vector<std::vector<T>>& data, uint32_t size, uint32_t& readBuffer);

	std::string inputBuffer_;
	std::string outputBuffer_;

	//TypeIDと混在しないように、バージョンは0xf0以上にする
	static constexpr uint8_t version = 0xf0;
	uint8_t version_ = version;

	uint32_t readIndex_ = 0;
	uint32_t prevReadIndex_ = 0;
	bool failed_ = false;

	std::string basePath_ = "Assets/Binary/";

	static constexpr uint32_t idSize = uint32_t(sizeof(TypeID));
	static constexpr uint32_t sizeSize = uint32_t(sizeof(uint32_t));
	static constexpr uint32_t headerSize = idSize + sizeSize;
};

inline bool BinaryManager::ReadHeader(TypeID expected, uint32_t& outSize) {
	if (failed_) {
		return false;
	}

	// 残りバイト数で判定する(加算によるオーバーフローを避ける)
	const size_t bufferSize = inputBuffer_.size();
	if (readIndex_ > bufferSize || bufferSize - readIndex_ < headerSize) {
		failed_ = true;
		return false;
	}

	TypeID id;
	uint32_t size;
	std::memcpy(&id, inputBuffer_.data() + readIndex_, idSize);
	std::memcpy(&size, inputBuffer_.data() + readIndex_ + idSize, sizeSize);

	if (id != expected) {
		failed_ = true;
		return false;
	}

	if (bufferSize - readIndex_ - headerSize < size) {
		failed_ = true;
		return false;
	}

	outSize = size;
	return true;
}

template<typename T>
void BinaryManager::Register(const T& data) {
	// 非constの T* は const T* より const T&(T=ポインタ) のほうが優先されるため、ここで参照先へ振り分ける
	if constexpr (std::is_pointer_v<T>) {
		Register(*data);
	} else {
		RegisterValue(data);
	}
}

template<typename T>
void BinaryManager::RegisterValue(const T& data) {
	static_assert(TypeIDResolver<T>::id != TypeID::kUnknown,
		"BinaryManager::Register: 未対応の型です。Value.h に TypeIDResolver を追加してください");
	static_assert(std::is_trivially_copyable_v<T>,
		"BinaryManager::Register: memcpy できない型は登録できません");

	constexpr TypeID currentID = TypeIDResolver<T>::id;
	uint32_t size = uint32_t(sizeof(T));

	/* ID->Size->値 */
	outputBuffer_.append(reinterpret_cast<const char*>(&currentID), sizeof(TypeID));
	outputBuffer_.append(reinterpret_cast<const char*>(&size), sizeof(uint32_t));
	outputBuffer_.append(reinterpret_cast<const char*>(&data), size);
}

template<typename T>
bool BinaryManager::Reverse(T& data) {
	static_assert(TypeIDResolver<T>::id != TypeID::kUnknown,
		"BinaryManager::Reverse: 未対応の型です。Value.h に TypeIDResolver を追加してください");
	static_assert(std::is_trivially_copyable_v<T>,
		"BinaryManager::Reverse: memcpy できない型は読み込めません");

	prevReadIndex_ = readIndex_;

	uint32_t size = 0;
	if (!ReadHeader(TypeIDResolver<T>::id, size)) {
		return false;
	}

	// 構造体のレイアウト変更などでサイズが変わっている場合は読まない
	if (size != sizeof(T)) {
		failed_ = true;
		return false;
	}

	std::memcpy(&data, inputBuffer_.data() + readIndex_ + headerSize, sizeof(T));
	readIndex_ += headerSize + size;
	return true;
}

template<typename T>
bool BinaryManager::Reverse(T*& data) {
	return Reverse(*data);
}

template<typename T>
inline bool BinaryManager::Reverse(std::vector<T>& data) {
	TypeID id = *reinterpret_cast<TypeID*>(inputBuffer_.data() + readIndex_);

	if (id != TypeID::Vector) {
		failed_ = true;
		return false;
	}

	readIndex_ += sizeof(TypeID);
	uint32_t allDataSize = *reinterpret_cast<uint32_t*>(inputBuffer_.data() + readIndex_);
	readIndex_ += sizeof(uint32_t);
	uint32_t elementSize = *reinterpret_cast<uint32_t*>(inputBuffer_.data() + readIndex_);
	readIndex_ += sizeof(uint32_t);

	uint32_t tmpReadIndex = readIndex_;
	if (!ReadVector(data, elementSize, tmpReadIndex)) {
		failed_ = true;
		return false;
	}

	readIndex_ = tmpReadIndex;
	return true;
}

template<>
inline void BinaryManager::RegisterValue<std::string>(const std::string& data) {
	constexpr TypeID id = TypeIDResolver<std::string>::id;
	uint32_t size = uint32_t(data.size());

	outputBuffer_.append(reinterpret_cast<const char*>(&id), sizeof(id));
	outputBuffer_.append(reinterpret_cast<const char*>(&size), sizeof(size));
	outputBuffer_.append(data.data(), size);
}

template<>
inline bool BinaryManager::Reverse<std::string>(std::string& data) {
	prevReadIndex_ = readIndex_;

	uint32_t size = 0;
	if (!ReadHeader(TypeIDResolver<std::string>::id, size)) {
		return false;
	}

	data.assign(inputBuffer_.data() + readIndex_ + headerSize, size);
	readIndex_ += headerSize + size;
	return true;
}

template<typename T>
inline void BinaryManager::RegisterValue(const std::vector<T>& data) {
	constexpr TypeID id = TypeIDResolver<std::vector<T>>::id;
	const uint32_t size = uint32_t(data.size());

	std::string tmp;

	if (!RegisterVectorValue(tmp, data.data(), size)) {
		return;
	}

	const uint32_t allSize = tmp.size();

	outputBuffer_.append(reinterpret_cast<const char*>(&id), sizeof(id));				//std::vectorである印
	outputBuffer_.append(reinterpret_cast<const char*>(&allSize), sizeof(allSize));		//読み飛ばし用
	outputBuffer_.append(reinterpret_cast<const char*>(&size), sizeof(size));			//vectorの要素数
	outputBuffer_.append(tmp);
}

template<typename T>
inline bool BinaryManager::RegisterVectorValue(std::string& buffer, const T* data, uint32_t size) {
	static_assert(TypeIDResolver<T>::id != TypeID::kUnknown,
		"BinaryManager::RegisterVectorValue: 未対応の型です。Value.h に TypeIDResolver を追加してください");
	static_assert(std::is_trivially_copyable_v<T>,
		"BinaryManager::RegisterVectorValue: memcpy できない型は登録できません");

	constexpr TypeID currentID = TypeIDResolver<T>::id;
	uint32_t typeSize = uint32_t(sizeof(T));
	buffer.append(reinterpret_cast<const char*>(&currentID), sizeof(TypeID));
	buffer.append(reinterpret_cast<const char*>(&typeSize), sizeof(typeSize));
	buffer.append(reinterpret_cast<const char*>(data), size_t(typeSize) * size);

	return true;
}

template<typename T>
inline bool BinaryManager::RegisterVectorValue(std::string& buffer, const std::vector<T>* data, uint32_t size) {
	constexpr TypeID currentID = TypeIDResolver<std::vector<T>>::id;
	buffer.append(reinterpret_cast<const char*>(&currentID), sizeof(TypeID));

	for (uint32_t i = 0; i < size; ++i) {
		const uint32_t dataSize = uint32_t(data[i].size());
		buffer.append(reinterpret_cast<const char*>(&dataSize), sizeof(dataSize));

		if (!RegisterVectorValue(buffer, data[i].data(), dataSize)) {
			failed_ = true;
			return false;
		}
	}

	return true;
}

template<typename T>
inline bool BinaryManager::ReadVector(std::vector<T>& data, uint32_t size, uint32_t& readBuffer) {
	constexpr TypeID currentID = TypeIDResolver<T>::id;
	TypeID id = *reinterpret_cast<const TypeID*>(inputBuffer_.data() + readBuffer);
	readBuffer += sizeof(TypeID);

	if (id != currentID) {
		failed_ = true;
		return false;
	}

	uint32_t typeSize = *reinterpret_cast<uint32_t*>(inputBuffer_.data() + readBuffer);
	readBuffer += sizeof(uint32_t);

	if (typeSize != sizeof(T)) {
		failed_ = true;
		return false;
	}

	data.resize(size);
	std::memcpy(data.data(), inputBuffer_.data() + readBuffer, sizeof(T) * size);
	readBuffer += size_t(typeSize) * size;
	return true;
}

template<typename T>
inline bool BinaryManager::ReadVector(std::vector<std::vector<T>>& data, uint32_t size, uint32_t& readBuffer) {
	constexpr TypeID currentID = TypeIDResolver<std::vector<T>>::id;
	TypeID id = *reinterpret_cast<const TypeID*>(inputBuffer_.data() + readBuffer);
	readBuffer += sizeof(TypeID);

	if (currentID != id) {
		failed_ = true;
		return false;
	}

	data.resize(size);
	for (auto& d : data) {
		uint32_t elementSize = *reinterpret_cast<uint32_t*>(inputBuffer_.data() + readBuffer);
		readBuffer += sizeof(uint32_t);

		ReadVector(d, elementSize, readBuffer);
	}

	return true;
}
