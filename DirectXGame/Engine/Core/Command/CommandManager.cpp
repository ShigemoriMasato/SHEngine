#include "CommandManager.h"

using namespace SHEngine::Command;

void Manager::Initialize(DXDevice* device) {
	device_ = device;
	logger_ = getLogger("Engine");

	//=========================================
	//コマンドキューの生成
	//=========================================
	auto& directQueue = queue_[Type::Direct].emplace_back();
	directQueue = std::make_unique<Queue>(device, Type::Direct);

	queue_[Type::Compute].resize(6);
	for (int i = 0; i < 6; ++i) {
		auto& computeQueue = queue_[Type::Compute][i];
		computeQueue = std::make_unique<Queue>(device, Type::Compute);
	}

	//=========================================
	//コマンドオブジェクトの初期化
	//=========================================
	objects_[Type::Direct].resize(1);
	objects_[Type::Compute].resize(6);
}

std::unique_ptr<Object> SHEngine::Command::Manager::CreateCommandObject(Type type, int index, int listNum) {
	QueueChecker(type);

	auto queue = queue_[type][index].get();
	std::unique_ptr<Object> commandObject = std::make_unique<Object>(device_, this, type, queue, listNum);

	queue->RegisterObject(commandObject.get());

	objects_[type][index].push_back(commandObject.get());

	return std::move(commandObject);
}

SHEngine::Command::WaitFence SHEngine::Command::Manager::Execute(Type type, int index, std::vector<CmdObj*> cmdObj) {
	QueueChecker(type);

	auto& cmdQueue = queue_[type][index];
	return cmdQueue->Execute(cmdObj);
}

void SHEngine::Command::Manager::ReleaseObject(Queue* queue, Object* obj) {
	for (auto& [type, queues] : queue_) {
		for (const auto& q : queues) {
			if (q.get() == queue) {
				auto& objects = objects_[type][&q - &queues[0]];
				objects.erase(std::remove(objects.begin(), objects.end(), obj), objects.end());
				return;
			}
		}
	}
}

void SHEngine::Command::Manager::WaitFence(const SHEngine::Command::WaitFence& waitFence, Type type, int index) {
	QueueChecker(type);
	auto& cmdQueue = queue_[type][index];
	cmdQueue->WaitForFence(waitFence);
}

void SHEngine::Command::Manager::QueueChecker(Type type) {
	if (queue_.find(type) == queue_.end()) {
		logger_->error("Invalid Command Queue Type: {}", static_cast<int>(type));
		throw std::runtime_error("Invalid Command Queue Type");
	}
}
