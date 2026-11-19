#pragma once
#include <Tool/CurveEditor/Data/CurveAsset.h>
#include <array>
#include <functional>
#include <map>
#include <string>
#include <vector>

class Camera;

/// @brief カーブ(Track)と実際の値を名前で結びつける。
/// Track 名とバインド名が一致したものに Apply で値を書き込む。
/// 次元数が違う場合は少ないほうに合わせて書き込む。
class CurveBinder {
public:

	using Values = std::array<float, CurveTrack::kMaxDimension>;

	// ポインタ版
	void BindFloat(const std::string& name, float* target);
	void BindVector2(const std::string& name, Vector2* target);
	void BindVector3(const std::string& name, Vector3* target);
	void BindVector4(const std::string& name, Vector4* target);

	// getter / setter 版
	void BindFloat(const std::string& name, std::function<float()> getter, std::function<void(float)> setter);
	void BindVector2(const std::string& name, std::function<Vector2()> getter, std::function<void(const Vector2&)> setter);
	void BindVector3(const std::string& name, std::function<Vector3()> getter, std::function<void(const Vector3&)> setter);
	void BindVector4(const std::string& name, std::function<Vector4()> getter, std::function<void(const Vector4&)> setter);

	/// @brief Apply / Restore の後に1回呼ばれる処理(カメラの MakeMatrix など)
	void AddPostApply(std::function<void()> func);

	void Unbind(const std::string& name);
	void Clear();

	/// @brief asset の各 Track を time で評価し、同名のバインド先へ書き込む
	void Apply(const CurveAsset& asset, float time) const;

	bool IsBound(const std::string& name) const { return bindings_.contains(name); }
	/// @brief バインドされている値の次元数(未バインドは 0)
	int GetDimension(const std::string& name) const;
	/// @brief バインド先の現在値を取得する
	bool GetCurrentValue(const std::string& name, Values& out) const;
	std::vector<std::string> GetBindingNames() const;

	/// @brief 全バインド先の現在値を保存する(プレビュー開始時)
	void Capture();
	/// @brief Capture した値に戻す(プレビュー終了時)
	void Restore();
	bool HasCaptured() const { return !captured_.empty(); }

private:

	struct Binding {
		int dimension = 1;
		std::function<void(float*)> get;
		std::function<void(const float*)> set;
	};

	void Bind(const std::string& name, Binding binding);
	void CallPostApply() const;

	std::map<std::string, Binding> bindings_;
	std::vector<std::function<void()>> postApply_;
	std::map<std::string, Values> captured_;
};

/// @brief カメラ用アダプタ。Position と Rotation(オイラー角・ラジアン)を1行で登録する。
/// 値は Camera::GetPosition / GetRotation で取得し、SetPosition / SetRotation で設定、最後に MakeMatrix を呼ぶ。
class CameraCurveBinder {
public:

	static void Bind(CurveBinder& binder, Camera* camera,
		const std::string& positionTrack = "Position", const std::string& rotationTrack = "Rotation");
};
