#include "SimplePostEffectData.h"
#include <imgui/imgui.h>

void Blur::DrawImGui() {
#ifdef USE_IMGUI
	int value = static_cast<int>(kernelSize);
	if (ImGui::SliderInt("Kernel Size", &value, 1, 30)) kernelSize = static_cast<uint32_t>(value);
#endif
}

void Blur::Save(BinaryManager& bin) const {
	bin.Register(kernelSize);
}

void Blur::Load(BinaryManager& bin) {
	bin.Reverse(kernelSize);
}

void GaussBlur::DrawImGui() {
#ifdef USE_IMGUI
	int value = static_cast<int>(kernelSize);
	if (ImGui::SliderInt("Kernel Size", &value, 1, 15)) kernelSize = static_cast<uint32_t>(value);
	ImGui::DragFloat("Sigma", &sigma, 0.01f);
#endif
}

void GaussBlur::Save(BinaryManager& bin) const {
	bin.Register(kernelSize);
	bin.Register(sigma);
}

void GaussBlur::Load(BinaryManager& bin) {
	bin.Reverse(kernelSize);
	bin.Reverse(sigma);
}

void Grayscale::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::DragFloat("Intensity", &intensity, 0.01f, 0.0f, 1.0f);
#endif
}

void Grayscale::Save(BinaryManager& bin) const {
	bin.Register(intensity);
}

void Grayscale::Load(BinaryManager& bin) {
	bin.Reverse(intensity);
}

void Vignette::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::ColorEdit4("Color", &color.x);
	ImGui::DragFloat("Intensity", &intensity, 0.01f);
	ImGui::DragFloat("Radius", &radius, 0.01f);
	ImGui::DragFloat("Softness", &softness, 0.01f);
#endif
}

void Vignette::Save(BinaryManager& bin) const {
	bin.Register(color);
	bin.Register(intensity);
	bin.Register(radius);
	bin.Register(softness);
}

void Vignette::Load(BinaryManager& bin) {
	bin.Reverse(color);
	bin.Reverse(intensity);
	bin.Reverse(radius);
	bin.Reverse(softness);
}

void Fade::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::ColorEdit4("Color", &color.x);
	ImGui::SliderFloat("Progress", &t, 0.0f, 1.0f);
#endif
}

void Fade::Save(BinaryManager& bin) const {
	bin.Register(color);
	bin.Register(t);
	bin.Register(pad);
}

void Fade::Load(BinaryManager& bin) {
	bin.Reverse(color);
	bin.Reverse(t);
	bin.Reverse(pad);
}

void EdgeDetection::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::Text("No editable parameters");
#endif
}

void EdgeDetection::Save(BinaryManager& bin) const {
}

void EdgeDetection::Load(BinaryManager& bin) {
}

void Outline::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::ColorEdit4("Color", &color.x);
	ImGui::DragFloat("Strength", &strength, 0.01f, 0.0f);
#endif
}

void Outline::Save(BinaryManager& bin) const {
	bin.Register(color);
	bin.Register(strength);
}

void Outline::Load(BinaryManager& bin) {
	bin.Reverse(color);
	bin.Reverse(strength);
}

void RadialBlur::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::DragFloat2("Center", &center.x, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat("Strength", &strength, 0.001f, 0.0f);
	ImGui::DragInt("Sample Count", &sampleCount, 1, 1, 64);
#endif
}

void RadialBlur::Save(BinaryManager& bin) const {
	bin.Register(center);
	bin.Register(strength);
	bin.Register(sampleCount);
}

void RadialBlur::Load(BinaryManager& bin) {
	bin.Reverse(center);
	bin.Reverse(strength);
	bin.Reverse(sampleCount);
}

void Dissolve::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::SliderFloat("Threshold", &threshold, 0.0f, 1.0f);
	ImGui::SliderFloat("Edge Threshold", &edgeThreshold, 0.0f, 1.0f);
	ImGui::ColorEdit3("Edge Color", &edgeColor.x);
#endif
}

void Dissolve::Save(BinaryManager& bin) const {
	bin.Register(threshold);
	bin.Register(edgeThreshold);
	bin.Register(edgeColor);
}

void Dissolve::Load(BinaryManager& bin) {
	bin.Reverse(threshold);
	bin.Reverse(edgeThreshold);
	bin.Reverse(edgeColor);
}
