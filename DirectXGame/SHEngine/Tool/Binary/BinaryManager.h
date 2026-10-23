#pragma once
#include <string>
#include <cstring>
#include "Value.h"

class BinaryManager {
public:

	bool Boot(const std::string& fileName);
	void BootRawData(const std::string& rawData);

	template<typename T>
	void Register(const T& data);
	template<typename T>
	void Register(const T* const& data);
	void Write(const std::string& fileName);

	//Registerした順番で値を吐き出す。一回しか吐き出さない。
	template<typename T>
	void Reverse(T& data);
	template<typename T>
	void Reverse(const T*& data);
	void Back() { readIndex_ = prevReadIndex_; }

	bool IsEmpty() const { return inputBuffer_.empty(); }

	std::string GetOutputRawData() const { return outputBuffer_; }
	std::string GetInputRawData() const { return inputBuffer_; }

private:

	std::string inputBuffer_;
	std::string outputBuffer_;

	//TypeIDと混在しないように、バージョンは0xf0以上にする
	static constexpr uint8_t version = 0xf0;
	uint8_t version_ = version;

	uint32_t readIndex_ = 0;
	uint32_t prevReadIndex_ = 0;

	static inline const std::string basePath = "Assets/Binary/";

	static constexpr uint32_t idSize = uint32_t(sizeof(TypeID));
	static constexpr uint32_t sizeSize = uint32_t(sizeof(uint32_t));
	static constexpr uint32_t headerSize = idSize + sizeSize;
};

template<typename T>
void BinaryManager::Register(const T& data) {
	constexpr TypeID currentID = TypeIDResolver<T>::id;
	uint32_t size = uint32_t(sizeof(T));

	// 未対応の型の場合は登録しない
	if (currentID == TypeID::kUnknown) {
		return;
	}

	/* ID->Size->値 */
	outputBuffer_.append(reinterpret_cast<const char*>(&currentID), sizeof(TypeID));
	outputBuffer_.append(reinterpret_cast<const char*>(&size), sizeof(uint32_t));
	outputBuffer_.append(reinterpret_cast<const char*>(&data), size);
}

template<typename T>
void BinaryManager::Register(const T* const& data) {
	Register(*data);
}

template<typename T>
void BinaryManager::Reverse(T& data) {
	prevReadIndex_ = readIndex_;

	if (inputBuffer_.size() < headerSize) {
		return;
	}

	TypeID id;
	uint32_t size;

	std::memcpy(&id, inputBuffer_.data() + readIndex_, idSize);
	std::memcpy(&size, inputBuffer_.data() + readIndex_ + idSize, sizeSize);

	if (id != TypeIDResolver<T>::id) {
		return;
	}

	if (inputBuffer_.size() < headerSize + size) {
		return;
	}

	std::memcpy(&data, inputBuffer_.data() + readIndex_ + headerSize, sizeof(T));
	readIndex_ += headerSize + size;
}

template<typename T>
void BinaryManager::Reverse(const T*& data) {
	Reverse(*data);
}

template<>
inline void BinaryManager::Register<std::string>(const std::string& data) {
	constexpr TypeID id = TypeIDResolver<std::string>::id;
	uint32_t size = uint32_t(data.size());

	outputBuffer_.append(reinterpret_cast<const char*>(&id), sizeof(id));
	outputBuffer_.append(reinterpret_cast<const char*>(&size), sizeof(size));
	outputBuffer_.append(data.data(), size);
}

template<>
inline void BinaryManager::Reverse<std::string>(std::string& data) {
	prevReadIndex_ = readIndex_;

	if (inputBuffer_.size() < headerSize) {
		return;
	}

	TypeID id;
	uint32_t size;
	std::memcpy(&id, inputBuffer_.data() + readIndex_, idSize);
	std::memcpy(&size, inputBuffer_.data() + readIndex_ + idSize, sizeSize);

	if (id != TypeIDResolver<std::string>::id) {
		return;
	}

	if (inputBuffer_.size() < headerSize + size) {
		return;
	}

	data.resize(size);
	std::memcpy(data.data(), inputBuffer_.data() + readIndex_ + headerSize, size);
	readIndex_ += headerSize + size;
}
