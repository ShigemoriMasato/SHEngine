#pragma once
#include <nlohmann/json.hpp>
#include <Tool/Binary/Value.h>

class JsonManager {
public:

    // 指定したファイルが存在すれば読み込み。無ければ空の状態で作成
	void Boot(const std::string& fileName);

	//データを追加。keyが重複している場合は上書き。対応しない型は例外が発生する。
	template<typename T>
	void Add(const std::string& key, T& data);
	template<typename T>
	void Add(const std::string& key, const T& data);


	//データを取得。keyが存在しない場合は例外が発生。対応しない型は例外が発生。
	template<typename T>
	T Get(const std::string& key);

	//ファイルに保存。Bootで読み込んだ内容は上書きされる。
    void Save();

private:

	static inline const std::string extension = ".json";
    static inline const std::string directory = "Assets/Json/";
	std::string fileName_;
	nlohmann::json jsonData_;

};

template<typename T>
void JsonManager::Add(const std::string& key, T& data) {
	using U = std::remove_cv_t<std::remove_reference_t<T>>;
	constexpr TypeID id = TypeIDResolver<U>::id;
	if constexpr (id == TypeID::kUnknown) {
		throw std::runtime_error("Unsupported type for JsonManager::Add");
	}
	else if constexpr (id == TypeID::Vector2) {
		jsonData_[key] = { data.x, data.y };
	}
	else if constexpr (id == TypeID::Vector3) {
		jsonData_[key] = { data.x, data.y, data.z };
	}
	else if constexpr (id == TypeID::Vector4) {
		jsonData_[key] = { data.x, data.y, data.z, data.w };
	}
	else if constexpr (id == TypeID::Matrix2x2) {
		jsonData_[key] = { { data.m[0][0], data.m[0][1] }, { data.m[1][0], data.m[1][1] } };
	}
	else if constexpr (id == TypeID::Matrix3x3) {
		jsonData_[key] = { { data.m[0][0], data.m[0][1], data.m[0][2] }, { data.m[1][0], data.m[1][1], data.m[1][2] }, { data.m[2][0], data.m[2][1], data.m[2][2] } };
	}
	else if constexpr (id == TypeID::Matrix4x4) {
		jsonData_[key] = { { data.m[0][0], data.m[0][1], data.m[0][2], data.m[0][3] }, { data.m[1][0], data.m[1][1], data.m[1][2], data.m[1][3] }, { data.m[2][0], data.m[2][1], data.m[2][2], data.m[2][3] }, { data.m[3][0], data.m[3][1], data.m[3][2], data.m[3][3] } };
	}
	else if constexpr (id == TypeID::Quaternion) {
		jsonData_[key] = { data.x, data.y, data.z, data.w };
	}
	else {
		jsonData_[key] = data;
	}
}

template<typename T>
void JsonManager::Add(const std::string& key, const T& data)
{
	using U = std::remove_cv_t<std::remove_reference_t<T>>;
	constexpr TypeID id = TypeIDResolver<U>::id;
	if constexpr (id == TypeID::kUnknown)
	{
		throw std::runtime_error("Unsupported type for JsonManager::Add");
	}
	else if constexpr (id == TypeID::Vector2)
	{
		jsonData_[key] = { data.x, data.y };
	}
	else if constexpr (id == TypeID::Vector3)
	{
		jsonData_[key] = { data.x, data.y, data.z };
	}
	else if constexpr (id == TypeID::Vector4)
	{
		jsonData_[key] = { data.x, data.y, data.z, data.w };
	}
	else if constexpr (id == TypeID::Matrix2x2)
	{
		jsonData_[key] = { { data.m[0][0], data.m[0][1] }, { data.m[1][0], data.m[1][1] } };
	}
	else if constexpr (id == TypeID::Matrix3x3)
	{
		jsonData_[key] = { { data.m[0][0], data.m[0][1], data.m[0][2] }, { data.m[1][0], data.m[1][1], data.m[1][2] }, { data.m[2][0], data.m[2][1], data.m[2][2] } };
	}
	else if constexpr (id == TypeID::Matrix4x4)
	{
		jsonData_[key] = { { data.m[0][0], data.m[0][1], data.m[0][2], data.m[0][3] }, { data.m[1][0], data.m[1][1], data.m[1][2], data.m[1][3] }, { data.m[2][0], data.m[2][1], data.m[2][2], data.m[2][3] }, { data.m[3][0], data.m[3][1], data.m[3][2], data.m[3][3] } };
	}
	else if constexpr (id == TypeID::Quaternion)
	{
		jsonData_[key] = { data.x, data.y, data.z, data.w };
	}
	else
	{
		jsonData_[key] = data;
	}
}

template<typename T>
T JsonManager::Get(const std::string& key) {
    constexpr TypeID id = TypeIDResolver<T>::id;
    if(id == TypeID::kUnknown) {
		throw std::runtime_error("Unsupported type for JsonManager::Get");
	}

	const nlohmann::json& j = jsonData_.at(key);

    if constexpr (id == TypeID::Vector2) {
        auto v = j.get<std::array<float, 2>>();
        return T{ v[0], v[1] };
    }
    else if constexpr (id == TypeID::Vector3) {
        auto v = j.get<std::array<float, 3>>();
        return T{ v[0], v[1], v[2] };
    }
    else if constexpr (id == TypeID::Vector4) {
        auto v = j.get<std::array<float, 4>>();
        return T{ v[0], v[1], v[2], v[3] };
    }
    else if constexpr (id == TypeID::Quaternion) {
        auto v = j.get<std::array<float, 4>>();
        return T{ v[0], v[1], v[2], v[3] };
    }
    else if constexpr (id == TypeID::Matrix2x2) {
        auto v = j.get<std::array<std::array<float, 2>, 2>>();
        T result;
        result.m[0][0] = v[0][0]; result.m[0][1] = v[0][1];
        result.m[1][0] = v[1][0]; result.m[1][1] = v[1][1];
        return result;
    }
    else if constexpr (id == TypeID::Matrix3x3) {
        auto v = j.get<std::array<std::array<float, 3>, 3>>();
        T result;
        result.m[0][0] = v[0][0]; result.m[0][1] = v[0][1]; result.m[0][2] = v[0][2];
        result.m[1][0] = v[1][0]; result.m[1][1] = v[1][1]; result.m[1][2] = v[1][2];
        result.m[2][0] = v[2][0]; result.m[2][1] = v[2][1]; result.m[2][2] = v[2][2];
        return result;
    }
    else if constexpr (id == TypeID::Matrix4x4) {
        auto v = j.get<std::array<std::array<float, 4>, 4>>();
        T result;
        result.m[0][0] = v[0][0]; result.m[0][1] = v[0][1]; result.m[0][2] = v[0][2]; result.m[0][3] = v[0][3];
        result.m[1][0] = v[1][0]; result.m[1][1] = v[1][1]; result.m[1][2] = v[1][2]; result.m[1][3] = v[1][3];
        result.m[2][0] = v[2][0]; result.m[2][1] = v[2][1]; result.m[2][2] = v[2][2]; result.m[2][3] = v[2][3];
        result.m[3][0] = v[3][0]; result.m[3][1] = v[3][1]; result.m[3][2] = v[3][2]; result.m[3][3] = v[3][3];
        return result;
    }
    else {
        try {
            return j.get<T>();
        } catch (const nlohmann::json::type_error&) {
            throw std::runtime_error("Type mismatch for JsonManager::Get");
        }
    }
}

