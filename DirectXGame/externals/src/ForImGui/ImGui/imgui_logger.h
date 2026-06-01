#pragma once
#include <string>
#include <vector>

class imgui_logger {
public:

	static void Initialize(bool isDebugString, bool isLogFile);
	static void Log(const std::string& message);

	//ImGui::Begin()を呼び出したときに出力するログ
	static bool Begin(const std::string& windowName);
	//ImGui::End()を呼び出したときに出力するログ
	static bool End();

private:

	static inline const std::string logName = "ImGui_Logger";
	static inline std::vector<std::string> activeWindow = {};

};
