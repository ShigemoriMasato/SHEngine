#include "Grid.h"

void Grid::Initialize(SHEngine::DrawDataManager* drawDataManager) {
	static int drawDataIndex = -1;
	if (drawDataIndex == -1) {
		std::vector<Vector3> vertices = std::vector<Vector3>(2, {});
		drawDataManager->AddVertexBuffer(vertices);
		drawDataIndex = drawDataManager->CreateDrawData();
	}

	auto drawData = drawDataManager->GetDrawData(drawDataIndex);

	render_ = std::make_unique<SHEngine::RenderObject>("Grid Tool");
	render_->Initialize();
	render_->SetDrawData(drawData);
	render_->psoConfig_.vs = "Engine/Grid.VS.hlsl";
	render_->psoConfig_.ps = "Engine/Grid.PS.hlsl";
	render_->psoConfig_.inputLayoutID = SHEngine::PSO::InputLayoutID::Vector3;
	render_->psoConfig_.topology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;

	render_->CreateSRV(sizeof(LineConfig), lineNum_ * 2, ShaderType::VERTEX_SHADER, "Config");
	render_->CreateCBV(sizeof(Matrix4x4), ShaderType::VERTEX_SHADER, "VP");
	render_->instanceNum_ = lineNum_ * 2;

	vertical_.resize(lineNum_);
	horizontal_.resize(lineNum_);

	gpuBuffer_.reserve(lineNum_ * 2);
}

void Grid::Update(Vector3 middle, const Matrix4x4& vpMatrix) {
	Vector3 middleGrided = {
		std::round(middle.x / interval_) * interval_,
		0.0f,
		std::round(middle.z / interval_) * interval_
	};

	for(int i = 0; i < lineNum_; i++) {
		float offset = (i - lineNum_ / 2) * interval_;
		bool isThick = int(middleGrided.x + offset) % int(thickInterval_ * interval_) == 0;
		vertical_[i] = {
			{ middleGrided.x + offset, middleGrided.y, middleGrided.z - length_ / 2 },
			middleGrided.y,
			{ middleGrided.x + offset, middleGrided.y, middleGrided.z + length_ / 2 },
			isThick ? 0xffff00ff : 0x808080ff
		};
		isThick = int(middleGrided.z + offset) % int(thickInterval_ * interval_) == 0;
		horizontal_[i] = {
			{ middleGrided.x - length_ / 2, middleGrided.y, middleGrided.z + offset },
			middleGrided.y,
			{ middleGrided.x + length_ / 2, middleGrided.y, middleGrided.z + offset },
			isThick ? 0xffff00ff : 0x808080ff
		};
	}

	//GPUに転送
	gpuBuffer_.insert(gpuBuffer_.end(), vertical_.begin(), vertical_.end());
	gpuBuffer_.insert(gpuBuffer_.end(), horizontal_.begin(), horizontal_.end());
	render_->CopyBufferData(0, gpuBuffer_.data(), sizeof(LineConfig) * gpuBuffer_.size());

	gpuBuffer_.clear();

	render_->CopyBufferData(1, &vpMatrix, sizeof(Matrix4x4));
}

void Grid::Draw(CmdObj* cmdObj) {
	render_->Draw(cmdObj);
}
