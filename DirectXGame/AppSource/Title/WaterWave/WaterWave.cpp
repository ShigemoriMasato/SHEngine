#include "WaterWave.h"
#include <imgui/imgui.h>

using namespace SHEngine;

WaterWave::~WaterWave() {
	Save();
}

void WaterWave::Initialize(const DrawData& drawData, Camera* camera) {
	camera_ = camera;
	renderObject_ = std::make_unique<RenderObject>("WaterWave");
	renderObject_->Initialize();
	renderObject_->psoConfig_.vs = "Water/WaterPlane.VS.hlsl";
	renderObject_->psoConfig_.ps = "Water/WaterPlane.PS.hlsl";
	renderObject_->SetDrawData(drawData);
	renderObject_->CreateCBV(sizeof(UniqueData), ShaderType::VERTEX_SHADER, "WaterWave::UniqueData");
	renderObject_->CreateCBV(sizeof(DirectionalLight), ShaderType::PIXEL_SHADER, "WaterWave::DirectionalLight");
	renderObject_->CreateCBV(sizeof(Vector4), ShaderType::PIXEL_SHADER, "WaterWave::BaseColor");
	renderObject_->instanceNum_ = 1;
	binaryManager_ = std::make_unique<BinaryManager>();
	Load();

	uniqueData_.world = Matrix::MakeAffineMatrix(scale, rotation, position);
}

void WaterWave::Update(float deltaTime) {
	uniqueData_.time += deltaTime;
	uniqueData_.cameraPos = {};
	uniqueData_.waveCount = 0;
	uniqueData_.vp = camera_->GetVPMatrix();
	for (int i = 0; i < 8; ++i) {
		uniqueData_.waves[i] = waves_[i];
		if (waves_[i].amplitude == 0.0f && waves_[i].wavelength == 0.0f) {
			continue;
		}
		uniqueData_.waveCount++;
	}

	renderObject_->CopyBufferData(0, &uniqueData_, sizeof(UniqueData));
	renderObject_->CopyBufferData(1, &light_, sizeof(DirectionalLight));
	renderObject_->CopyBufferData(2, &baseColor_, sizeof(Vector4));
}

void WaterWave::Draw(CmdObj* window) {

	renderObject_->Draw(window);
}

void WaterWave::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::Begin("WaterPlaneResource");

	time_ += 0.0166666667f; //約60fps

	ImGui::ColorEdit4("Light Color", &light_.color.x);
	ImGui::DragFloat3("Light Direction", &light_.direction.x, 0.01f);
	light_.direction = light_.direction.Normalize();
	ImGui::DragFloat("Light Intensity", &light_.intensity, 0.1f);

	ImGui::Separator();

	ImGui::DragFloat3("Scale", &scale.x, 0.1f, 0.1f);
	ImGui::DragFloat3("Rotation", &rotation.x, 0.1f);
	ImGui::DragFloat3("Position", &position.x, 0.1f);
	ImGui::ColorEdit4("Base Color", &baseColor_.x);
	uniqueData_.world = Matrix::MakeAffineMatrix(scale, rotation, position);

	ImGui::Separator();

	static int currentWave = 0;
	ImGui::Text("%d", currentWave);
	ImGui::SameLine();
	if (ImGui::Button("+")) {
		currentWave++;
	}
	ImGui::SameLine();
	if (ImGui::Button("-")) {
		currentWave--;
	}
	currentWave = std::clamp(currentWave, 0, static_cast<int>(waves_.size() - 1));

	ImGui::DragFloat2("Direction", &waves_[currentWave].direction.x, 0.1f, -1.0f, 1.0f);
	waves_[currentWave].direction = waves_[currentWave].direction.Normalize();
	ImGui::DragFloat("Amplitude", &waves_[currentWave].amplitude, 0.01f, 0.0f, 1000.0f);
	ImGui::DragFloat("Wavelength", &waves_[currentWave].wavelength, 0.1f, 0.1f, 100.0f);
	ImGui::DragFloat("Speed", &waves_[currentWave].speed, 0.1f, 0.0f, 1000.0f);

	ImGui::End();

	for (int i = 0; i < 8; ++i) {
		uniqueData_.waves[i] = waves_[i];
		if (waves_[i].amplitude == 0.0f && waves_[i].wavelength == 0.0f) {
			continue;
		}
		uniqueData_.waveCount++;
	}
#endif
}

void WaterWave::Save() {
	binaryManager_->Register(&scale);
	binaryManager_->Register(&rotation);
	binaryManager_->Register(&position);
	int waveSize = static_cast<int>(waves_.size());
	binaryManager_->Register(&waveSize);
	for (size_t i = 0; i < waves_.size(); i++) {
		binaryManager_->Register(&waves_[i].direction.x);
		binaryManager_->Register(&waves_[i].direction.y);
		binaryManager_->Register(&waves_[i].amplitude);
		binaryManager_->Register(&waves_[i].wavelength);
		binaryManager_->Register(&waves_[i].speed);
	}
	binaryManager_->Register(&light_.color);
	binaryManager_->Register(&light_.direction);
	binaryManager_->Register(&light_.intensity);
	binaryManager_->Register(&baseColor_);
	binaryManager_->Write("WaterPlaneResource.bin");
}

void WaterWave::Load() {
	//初期値設定
	if (binaryManager_->Boot("WaterPlaneResource.bin")) {
		scale = binaryManager_->Reverse<Vector3>();
		rotation = binaryManager_->Reverse<Vector3>();
		position = binaryManager_->Reverse<Vector3>();
		int waveSize = binaryManager_->Reverse<int>();
		waves_.resize(waveSize);
		for (size_t i = 0; i < waves_.size(); i++) {
			waves_[i].direction.x = binaryManager_->Reverse<float>();
			waves_[i].direction.y = binaryManager_->Reverse<float>();
			waves_[i].amplitude = binaryManager_->Reverse<float>();
			waves_[i].wavelength = binaryManager_->Reverse<float>();
			waves_[i].speed = binaryManager_->Reverse<float>();
		}
		light_.color = binaryManager_->Reverse<Vector4>();
		light_.direction = binaryManager_->Reverse<Vector3>();
		light_.intensity = binaryManager_->Reverse<float>();
		baseColor_ = binaryManager_->Reverse<Vector4>();

	} else {
		waves_.resize(8);
	}
}
