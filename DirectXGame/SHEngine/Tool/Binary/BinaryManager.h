#pragma once
#include <string>
#include <cstring>
#include <cstdint>
#include <type_traits>
#include "Value.h"

class BinaryManager {
public:

	bool Boot(const std::string& fileName);
	void BootRawData(const std::string& rawData);

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

	std::string inputBuffer_;
	std::string outputBuffer_;

	//TypeIDと混在しないように、バージョンは0xf0以上にする
	static constexpr uint8_t version = 0xf0;
	uint8_t version_ = version;

	uint32_t readIndex_ = 0;
	uint32_t prevReadIndex_ = 0;
	bool failed_ = false;

	static inline const std::string basePath = "Assets/Binary/";

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
