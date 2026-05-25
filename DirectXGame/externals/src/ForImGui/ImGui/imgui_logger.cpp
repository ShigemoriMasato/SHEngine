#include "imgui_logger.h"
#include <filesystem>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>

namespace fs = std::filesystem;

namespace {
	void archiveOldLogs(const std::string& logDir, size_t maxFiles) {
		namespace fs = std::filesystem;
		std::vector<fs::directory_entry> logFiles;

		for (const auto& entry : fs::directory_iterator(logDir)) {
			if (entry.is_regular_file() && entry.path().extension() == ".log") {
				logFiles.push_back(entry);
			}
		}

		std::sort(logFiles.begin(), logFiles.end(),
			[](const auto& a, const auto& b) {
				return fs::last_write_time(a) < fs::last_write_time(b);
			});

		if (logFiles.size() > maxFiles) {
			fs::create_directories(logDir + "/archive");
			for (size_t i = 0; i < logFiles.size() - maxFiles; ++i) {
				fs::rename(logFiles[i], logDir + "/archive/" + logFiles[i].path().filename().string());
			}
		}

	}

	std::shared_ptr<spdlog::logger> logger;
}

void imgui_logger::Initialize() {
	std::vector<spdlog::sink_ptr> sinks;

	//現在時刻を取得
	auto now = std::chrono::system_clock::now();
	auto t = std::chrono::system_clock::to_time_t(now);
	std::tm tm;
	localtime_s(&tm, &t);

	//ログファイル名を生成
	std::ostringstream fileName;
	fileName << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S") << ".log";

	//ログファイルのパスを生成
	std::string logDir = "Logs/" + logName + "/" + fileName.str();
	sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(logDir, true));

	//コンソールにも出力
	sinks.push_back(std::make_shared<spdlog::sinks::msvc_sink_mt>());

	//loggerを作成して登録
	logger = std::make_shared<spdlog::logger>(logName, sinks.begin(), sinks.end());
	spdlog::register_logger(logger);

	logger->set_level(spdlog::level::debug);
	logger->flush_on(spdlog::level::warn);

	logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%n] %v");

	activeWindow.reserve(8);
}

void imgui_logger::Log(const std::string& message) {
	std::string indent = "";
	for (int i = 0; i < activeWindow.size() + 1; ++i) {
		indent += "|  ";
	}

	logger->info("{} {}", indent, message);
}

bool imgui_logger::Begin(const std::string& windowName) {
	activeWindow.push_back(windowName);

	std::string indent;
	for (int i = 0; i < activeWindow.size() - 1; ++i) {
		indent += "|  ";
	}

	logger->info("{}Begin [{}]", indent, windowName);

	indent += "|  ";
	logger->info("{}", indent);

	return true;
}

bool imgui_logger::End() {
	if (activeWindow.empty()) {
		logger->warn("End called without an active window.");
		return false;
	}

	std::string indent;
	for (int i = 0; i < activeWindow.size() - 1; ++i) {
		indent += "|  ";
	}

	logger->info("{}End [{}]", indent, activeWindow.back());
	logger->info("{}", indent);

	activeWindow.pop_back();

	return true;
}
