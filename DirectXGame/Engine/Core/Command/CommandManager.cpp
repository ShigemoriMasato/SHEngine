#include "CommandManager.h"

using namespace SHEngine::Command;

void Manager::Initialize(DXDevice* device) {
	device_ = device;
	logger_ = getLogger("Engine");

	//=========================================
	//コマンドキューの生成
	//=========================================
	auto& directQueue = queue_[Type::Direct];
	directQueue = std::make_unique<Queue>(device, Type::Direct);

	auto& computeQueue = queue_[Type::Compute];
	computeQueue = std::make_unique<Queue>(device, Type::Compute);

	//=========================================
	//コマンドオブジェクトの初期化
	//=========================================
	objects_[Type::Direct].resize(1);
	objects_[Type::Compute].resize(6);
}

std::unique_ptr<Object> SHEngine::Command::Manager::CreateCommandObject(Type type, int listNum) {
	QueueChecker(type);

	auto queue = queue_[type].get();
	std::unique_ptr<Object> commandObject = std::make_unique<Object>(device_, type, listNum);

	objects_[type].push_back(commandObject.get());

	return std::move(commandObject);
}

SHEngine::Command::WaitFence SHEngine::Command::Manager::Execute(Type type, std::vector<CmdObj*> cmdObj) {
	QueueChecker(type);

	auto& cmdQueue = queue_[type];
	return cmdQueue->Execute(cmdObj);
}

void SHEngine::Command::Manager::ReleaseObject(Queue* queue, Object* obj) {
	for (auto& [type, queues] : queue_) {
		if (queues.get() == queue) {
			for (int i = 0; i < objects_[type].size(); ++i) {
				if (objects_[type][i] == obj) {
					objects_[type].erase(objects_[type].begin() + i);
					return;
				}
			}
		}
	}
}

void SHEngine::Command::Manager::WaitFence(const SHEngine::Command::WaitFence& waitFence, Type type) {
	QueueChecker(type);
	auto& cmdQueue = queue_[type];
	cmdQueue->WaitFenceInGPU(waitFence);
}

void SHEngine::Command::Manager::QueueChecker(Type type) {
	if (queue_.find(type) == queue_.end()) {
		logger_->error("Invalid Command Queue Type: {}", static_cast<int>(type));
		throw std::runtime_error("Invalid Command Queue Type");
	}
}
