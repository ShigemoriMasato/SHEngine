#include "Grid.h"

void Grid::Initialize() {
	container_ = std::make_unique<SHEngine::BufferContainer>(3);

	auto positionBuffer = container_->Create(BufferType::CBV, sizeof(Vector3), 2, BufferNum::Single);
	configBuffer_ = container_->Create(BufferType::SRV, sizeof(LineConfig), lineNum_ * 2);
	vpBuffer_ = container_->Create(BufferType::CBV, sizeof(Matrix4x4));

	Mesh dummyMesh;
	dummyMesh.position.resize(2);

	renderer_ = std::make_unique<SHEngine::Renderer>(SHEngine::VertexType::Position, dummyMesh);
	renderer_->SetVS("Engine/Grid.VS.hlsl");
	renderer_->SetPS("Engine/Grid.PS.hlsl");
	renderer_->SetGPUBuffer(configBuffer_, ShaderType::VERTEX_SHADER, BufferType::SRV);
	renderer_->SetGPUBuffer(vpBuffer_, ShaderType::VERTEX_SHADER, BufferType::CBV);
	renderer_->SetTopology(SHEngine::PSO::Topology::Line);
	renderer_->instanceNum_ = lineNum_ * 2;

	vertical_.resize(lineNum_);
	horizontal_.resize(lineNum_);

	configs_.reserve(lineNum_ * 2);
}

void Grid::Update(Vector3 middle, const Matrix4x4& vpMatrix) {
	Vector3 middleGrided = {
		std::round(middle.x / interval_) * interval_,
		0.0f,
		std::round(middle.z / interval_) * interval_
	};

	configs_.clear();

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
	configs_.insert(configs_.end(), vertical_.begin(), vertical_.end());
	configs_.insert(configs_.end(), horizontal_.begin(), horizontal_.end());

	configBuffer_->CopyBuffer(configs_.data(), sizeof(LineConfig) * configs_.size());
	vpBuffer_->CopyBuffer(&vpMatrix, sizeof(Matrix4x4));
}

void Grid::Draw(DCC* dcc) {
	renderer_->Draw(dcc);
}
