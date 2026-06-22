#include "DecoObjectController.h"
#include <Utility/DirectUtilFuncs.h>

Decorate::ObjController::ObjController(SHEngine::Screen::Display* display, SHEngine::Engine* engine, DataManager* dataManager) {
	display_ = display;
	engine_ = engine;
	dataManager_ = dataManager;

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

void Decorate::ObjController::Update(Camera* camera, DCC* dcc) {
	GetIDFromGPU(dcc);
	preClick_ = click_;
	click_ = bool(engine_->GetInput()->GetMouseButtonState()[0]);

	bool isImGuizmoActive = ImGuizmo::IsUsing() || ImGuizmo::IsOver();

	if (!isImGuizmoActive && !preClick_ && click_ && display_->IsHovering()) {
		uint32_t preID = dataManager_->GetCurrentID();
		dataManager_->Begin(HistoryType::ID, &preID);
		dataManager_->End(&selectedID_);
	}

	EditObject(camera);
}

// =======================================================================================================================

void Decorate::ObjController::GetIDFromGPU(DCC* dcc) {
	auto cmdObj = dcc->GetCurrentCmdObj();
	auto readBack = readBacks_[cmdObj->GetCurrentID()];
	auto cmdList = cmdObj->GetCommandList();

	display_->ToNonPixel(cmdObj);

	//前回のReadBackの内容を取得する
	{
		void* mappedData = nullptr;
		readBack.res->Map(0, nullptr, &mappedData);
		selectedID_ = *static_cast<uint32_t*>(mappedData);
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

void Decorate::ObjController::EditObject(Camera* camera) {
	//currentID_が0のときは何もしない
	if (dataManager_->GetCurrentID() == 0) return;

	const Transform& transform = dataManager_->GetCurrentTransform();

	//ImGuizmoを使って編集する
#ifdef USE_IMGUI

	ImGuizmo::Enable(true);

	static ImGuizmo::OPERATION op = ImGuizmo::OPERATION::TRANSLATE;
	ImGuizmo::MODE mode = ImGuizmo::MODE::WORLD;

	float view[16];
	float projection[16];
	float world[16];
	float cache[16];

	std::memcpy(view, camera->GetViewMatrix().m, sizeof(float) * 16);
	std::memcpy(projection, camera->GetProjectionMatrix().m, sizeof(float) * 16);
	std::memcpy(world, transform.Matrix().m, sizeof(float) * 16);
	std::memcpy(cache, world, sizeof(float) * 16);

	ImGuizmo::Manipulate(view, projection, op, mode, world);

	float translation[3], rotation[3], scale[3];
	ImGuizmo::DecomposeMatrixToComponents(world, translation, rotation, scale);

	rotation[0] *= 3.14159265358979323846f / 180.0f;
	rotation[1] *= 3.14159265358979323846f / 180.0f;
	rotation[2] *= 3.14159265358979323846f / 180.0f;

	//ImGuizmoを触っているとき
	if (isImGuizmoActive_) {
		//クリックしてないときは、編集終了
		if (!click_) {
			dataManager_->End(&transform);
			isImGuizmoActive_ = false;
		} else {
			dataManager_->Update(&transform);
		}
	}

	bool different = false;

	switch (op) {
	case ImGuizmo::TRANSLATE:
		for (int i = 0; i < 3; ++i) {
			if ((&transform.position.x)[i] != translation[i]) {
				different = true;
				break;
			}
		}
		break;
	case ImGuizmo::ROTATE:
		for (int i = 0; i < 3; ++i) {
			if ((&transform.rotate.x)[i] != rotation[i]) {
				different = true;
				break;
			}
		}
		break;
	case ImGuizmo::SCALE:
		for (int i = 0; i < 3; ++i) {
			if ((&transform.scale.x)[i] != scale[i]) {
				different = true;
				break;
			}
		}
		break;
	}

	//ギズモ触ってなくて、クリックしていて、カーソルがギズモの上にあるとき
	if (!isImGuizmoActive_ && ImGuizmo::IsOver() && click_) {
		//値が変わってたら
		if (different) {
			dataManager_->Begin(HistoryType::Transform, &transform);
			isImGuizmoActive_ = true;
		}
	}

	ImGui::Begin("Transform");
	if (ImGui::Button("S")) op = ImGuizmo::OPERATION::SCALE;
	ImGui::SameLine();
	if (ImGui::Button("R")) op = ImGuizmo::OPERATION::ROTATE;
	ImGui::SameLine();
	if (ImGui::Button("T")) op = ImGuizmo::OPERATION::TRANSLATE;

	ImGui::Text("Scale:    %.2f, %.2f, %.2f", transform.scale.x, transform.scale.y, transform.scale.z);
	ImGui::Text("Rotate:   %.2f, %.2f, %.2f", transform.rotate.x, transform.rotate.y, transform.rotate.z);
	ImGui::Text("Position: %.2f, %.2f, %.2f", transform.position.x, transform.position.y, transform.position.z);

	ImGui::End();

#endif
}
