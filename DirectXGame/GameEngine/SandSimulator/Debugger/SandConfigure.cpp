#include "SandConfigure.h"
#include <imgui/imgui.h>

void Sand::Configure::Add(std::function<void()> drawImGui, std::string name) {
	drawImGuiList_[name] = drawImGui;
	confList_[name] = Conf();
}

void Sand::Configure::DrawImGui() {
#ifdef USE_IMGUI

	ImGui::Begin("Configure");
	ImGui::DragInt("Height", &height_, 1, 0, 100);

	for (auto& [name, conf] : confList_) {
		ImGui::PushID(name.c_str());
		ImGui::BeginDisabled(conf.isOpen);

		if (ImGui::Button(name.c_str())) {
			conf.isOpen = true;
		}
		ImGui::EndDisabled();

		if (conf.isOpen) {
			ImGui::SameLine();
			if (ImGui::Button("X")) {
				conf.isOpen = false;
			}
		}

		ImGui::PopID();
	}

	ImGui::End();

	for (auto& [name, drawImGui] : drawImGuiList_) {
		auto& conf = confList_[name];
		if (!conf.isOpen) {
			continue;
		}

		ImGui::Begin(name.c_str(), &conf.isOpen);
		drawImGui();
		ImGui::End();
	}

#endif
}
