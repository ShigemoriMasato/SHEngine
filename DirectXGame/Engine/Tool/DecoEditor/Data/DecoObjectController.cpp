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

void Decorate::ObjController::Update(ObjManager* objManager, Camera* camera, DCC* dcc) {
	GetIDFromGPU(objManager, dcc);
	preClick_ = click_;
	click_ = bool(engine_->GetInput()->GetMouseButtonState()[0]);

	bool isImGuizmoActive = ImGuizmo::IsUsing() || ImGuizmo::IsOver();

	if (!isImGuizmoActive && !preClick_ && click_ && display_->IsHovering()) {
		currentID_ = prevID_;
	}

	EditObject(objManager, camera);


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

void Decorate::ObjController::EditObject(ObjManager* objManager, Camera* camera) {
	//currentID_が0のときは何もしない
	if (currentID_ == 0) return;

	if (currentID_ > transforms_.size()) {
		uint32_t preSize = static_cast<uint32_t>(transforms_.size());
		transforms_.resize(currentID_);
		for (uint32_t i = preSize; i < currentID_; ++i) {
			Matrix4x4 world = objManager->GetTransform(i);
			transforms_[currentID_ - 1].position = { world.m[3][0], world.m[3][1], world.m[3][2] };
		}
	}

	Transform& transform = transforms_[currentID_ - 1];

	//ImGuizmoを使って編集する
#ifdef USE_IMGUI

	ImGuizmo::Enable(true);

	static ImGuizmo::OPERATION op = ImGuizmo::OPERATION::TRANSLATE;
	ImGuizmo::MODE mode = ImGuizmo::MODE::WORLD;

	float view[16];
	float projection[16];

	std::memcpy(view, camera->GetViewMatrix().m, sizeof(float) * 16);
	std::memcpy(projection, camera->GetProjectionMatrix().m, sizeof(float) * 16);

	float world[16];
	Matrix4x4 mat = transform.Matrix();
	std::memcpy(world, mat.m, sizeof(float) * 16);

	ImGuizmo::Manipulate(view, projection, op, mode, world);

	float translation[3], rotation[3], scale[3];
	ImGuizmo::DecomposeMatrixToComponents(world, translation, rotation, scale);

	switch (op) {
	case ImGuizmo::OPERATION::TRANSLATE:
		transform.position = { translation[0], translation[1], translation[2] };
		break;
	case ImGuizmo::OPERATION::ROTATE:
		transform.rotate = { rotation[0], rotation[1], rotation[2] };
		break;
	case ImGuizmo::OPERATION::SCALE:
		transform.scale = { scale[0], scale[1], scale[2] };
		break;
	default:
		transform = {
			{scale[0], scale[1], scale[2]},
			{rotation[0], rotation[1], rotation[2] },
			{translation[0], translation[1], translation[2] }
		};
		break;
	}

	ImGui::Begin("DecoObjectController");
	ImGui::Text("IsWantCaptureMouse: %s", ImGui::GetIO().WantCaptureMouse ? "TRUE" : "FALSE");
	ImGui::Text("IsOver: %s", ImGuizmo::IsOver() ? "TRUE" : "FALSE");
	ImGui::Text("IsUsing: %s", ImGuizmo::IsUsing() ? "TRUE" : "FALSE");
	ImGui::Text("ViewManipulate: %s", ImGuizmo::IsViewManipulateHovered() ? "TRUE" : "FALSE");
	ImGuiIO& io = ImGui::GetIO();
	ImGui::Text("Mouse %.1f %.1f Down=%d", io.MousePos.x, io.MousePos.y, io.MouseDown[0]);
	ImGui::Text("WorldMatrix:");
	ImGui::Text("%.2f %.2f %.2f %.2f", world[0], world[1], world[2], world[3]);
	ImGui::Text("%.2f %.2f %.2f %.2f", world[4], world[5], world[6], world[7]);
	ImGui::Text("%.2f %.2f %.2f %.2f", world[8], world[9], world[10], world[11]);
	ImGui::Text("%.2f %.2f %.2f %.2f", world[12], world[13], world[14], world[15]);
	ImGui::End();


	ImGui::Begin("Transform");
	if (ImGui::Button("S")) op = ImGuizmo::OPERATION::SCALE;
	ImGui::SameLine();
	if (ImGui::Button("R")) op = ImGuizmo::OPERATION::ROTATE;
	ImGui::SameLine();
	if (ImGui::Button("T")) op = ImGuizmo::OPERATION::TRANSLATE;

	ImGui::DragFloat3("Scale", &transform.scale.x, 0.01f);
	ImGui::DragFloat3("Rotate", &transform.rotate.x, 0.01f);
	ImGui::DragFloat3("Position", &transform.position.x, 0.1f);

	ImGui::End();

#endif

	objManager->SetTransform(currentID_, transform);
}
