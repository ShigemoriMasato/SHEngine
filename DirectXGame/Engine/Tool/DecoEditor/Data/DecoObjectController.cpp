#include "DecoObjectController.h"
#include <Utility/DirectUtilFuncs.h>

Decorate::ObjController::ObjController(SHEngine::Screen::Display* display, SHEngine::Engine* engine) {
	display_ = display;
	engine_ = engine;

	//ReadBackをSwapChainに合わせて作成する
	int bufferCount = device_->GetBufferCount();
	readBacks_.resize(bufferCount);
	for (int i = 0; i < bufferCount; ++i) {
		readBacks_[i].res.Attach(SHEngine::Func::CreateReadBackResource(device_->GetDevice(), sizeof(uint32_t)));
	}

	container_ = std::make_unique<SHEngine::BufferContainer>();
	cursorBuffer_ = container_->Create(BufferType::CBV, sizeof(Vector2));
	ansBuffer_ = container_->Create(BufferType::UAV, sizeof(uint32_t));
	auto textureIndexBuffer = container_->Create(BufferType::CBV, sizeof(int));
	int textureIndex = display_->GetTextureAllData()[1]->GetHandle();
	textureIndexBuffer->CopyBuffer(&textureIndex, sizeof(int));

	idGetter_ = std::make_unique<SHEngine::ComputeObject>();
	idGetter_->Initialize();
	idGetter_->SetShader("Deco/GetID.CS.hlsl");
	idGetter_->SetThreadGroupSize(1, 1, 1);
	idGetter_->SetGPUBuffer(BufferType::CBV, cursorBuffer_);
	idGetter_->SetGPUBuffer(BufferType::CBV, textureIndexBuffer);
	idGetter_->SetGPUBuffer(BufferType::UAV, ansBuffer_);
	idGetter_->SetUseTexture(true);
	idGetter_->SetSamplerID(SHEngine::PSO::SamplerID::Point);
}

void Decorate::ObjController::Update(ObjManager* objManager, DCC* dcc) {
	GetIDFromGPU(objManager, dcc);
	preClick_ = click_;
	click_ = bool(engine_->GetInput()->GetMouseButtonState()[0]);

	if (!preClick_ && click_ && display_->IsHovering()) {
		currentID_ = prevID_;
	}

	EditObject(objManager);


}

// =======================================================================================================================

void Decorate::ObjController::GetIDFromGPU(ObjManager* objManager, DCC* dcc) {
	auto cmdObj = dcc->GetCurrentCmdObj();
	auto readBack = readBacks_[cmdObj->GetCurrentID()];
	auto cmdList = cmdObj->GetCommandList();

	display_->ToNonPixel(cmdObj);

	//前回のReadBackの内容を取得する
	{
		void* mappedData = nullptr;
		readBack.res->Map(0, nullptr, &mappedData);
		prevID_ = *static_cast<uint32_t*>(mappedData);
	}

	//結果の取得を命令する
	ansBuffer_->TransitionBarrier(D3D12_RESOURCE_STATE_COPY_SOURCE);
	ansBuffer_->Flush(cmdObj);
	cmdList->CopyBufferRegion(readBack.res.Get(), 0, ansBuffer_->GetResource(), 0, sizeof(uint32_t));

	//マウスカーソルの座標を取得
	Vector2 cursorPos = engine_->GetInput()->GetCursorPos();
	cursorPos = display_->GetCursorPos(cursorPos);

	//ComputeShaderに渡す
	cursorBuffer_->CopyBuffer(&cursorPos, sizeof(cursorPos));

	//computeShaderを起動
	idGetter_->Execute(cmdObj);

	//ReadBackしてIDを取得
	uint32_t id = 0;

	//Barrier張替
	ansBuffer_->TransitionBarrier(D3D12_RESOURCE_STATE_COPY_SOURCE);
	ansBuffer_->Flush(cmdObj);

	cmdList->CopyBufferRegion(readBack.res.Get(), 0, ansBuffer_->GetResource(), 0, sizeof(uint32_t));

	//Computeの起動時勝手にバリアが切り替わるため、バリアは戻さない
}

void Decorate::ObjController::EditObject(ObjManager* objManager) {
	//currentID_が0のときは何もしない
	if (currentID_ == 0) return;

	if (currentID_ > transforms_.size()) {
		uint32_t preSize = static_cast<uint32_t>(transforms_.size());
		transforms_.resize(currentID_);
		for (int i = preSize; i < currentID_; ++i) {
			Matrix4x4 world = objManager->GetTransform(i);
			transforms_[currentID_ - 1].position = { world.m[3][0], world.m[3][1], world.m[3][2] };
		}
	}

	Transform& transform = transforms_[currentID_ - 1];

	//仮でImGui::DragFloat3を使って編集する
#ifdef USE_IMGUI

	ImGui::Begin("Object Editor");

	ImGui::DragFloat3("Scale", &transform.scale.x, 0.01f);
	ImGui::DragFloat3("Rotate", &transform.rotate.x, 0.01f);
	ImGui::DragFloat3("Position", &transform.position.x, 0.1f);

	ImGui::End();

#endif

	objManager->SetTransform(currentID_, transform);
}
