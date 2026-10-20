#include "SandCPUSample.h"

SandCPUSample::SandCPUSample(uint32_t particleCount, const ModelData* modelData) {
	positions_.resize(particleCount);
	velocities_.resize(particleCount);
	forces_.resize(particleCount);

	container_ = std::make_unique<SHEngine::BufferContainer>();
	particleData_.position = container_->Create(BufferType::SRV, sizeof(Vector3), particleCount);
	particleData_.scale = container_->Create(BufferType::CBV, sizeof(float));
	particleData_.color = container_->Create(BufferType::CBV, sizeof(Vector4));

	renderer_ = std::make_unique<Sand::Renderer>(&particleData_, modelData);
	renderer_->SetInstance(particleCount);
}

void SandCPUSample::Initialize() {
	std::uniform_real_distribution<float> dist(0.f, 1.f);

	float size = 0.03f;
	float height = 0.5f;

	for (uint32_t i = 0; i < positions_.size(); ++i) {
		positions_[i] = { dist(randomEngine_) * size, dist(randomEngine_) * size + height, dist(randomEngine_) * size };
		velocities_[i] = { 0.f, 0.f, 0.f };
		forces_[i] = { 0.f, 0.f, 0.f };
	}

	Vector4 initColor{ 1.f, 1.f, 0.f, 1.f };
	particleData_.color->CopyBuffer(&initColor, sizeof(Vector4));
	particleData_.scale->CopyBuffer(&scale_, sizeof(float));
}

void SandCPUSample::Update(float deltatime) {
	auto removeForce = [](const Vector3& a, const Vector3& b) {
		float dot = MyMath::dot(a, b);
		float lengthSq = MyMath::dot(b, b);

		if (lengthSq == 0.0f)
			return a;

		return a - b * (dot / lengthSq);
		};

	for (uint32_t i = 0; i < positions_.size(); ++i) {
		auto& pos = positions_[i];
		auto& vel = velocities_[i];
		auto& force = forces_[i];
		//重力
		force += { 0.f, -9.8f, 0.f };

		//その他の力

		//速度更新
		vel += force * deltatime;

		pos += vel * deltatime;
		force = { 0.f, 0.f, 0.f };

		if (pos.y < 0.f) {
			pos.y = 0.f;
			vel.y = 0.0f;
			vel *= 0.75f;
		}

		for (int colliderCount = 0; colliderCount < 10; ++colliderCount) {
			//当たり判定
			const float hitArea = scale_ * 1.5f;
			for (uint32_t j = 0; j < positions_.size(); ++j) {
				if (i == j) continue;

				const auto& other = positions_[j];
				Vector3 diff = other - pos;
				float dist = diff.Length();

				//衝突処理
				if (dist < hitArea) {
					pos -= diff.Normalize() * (hitArea - dist) * 0.5f;
					vel = removeForce(vel, diff.Normalize());
				}
			}

			// 速度が小さすぎる場合は0にする
			if (vel.Length() < 1.0f) {
				vel = {};
			}

			//補正後の座標を更新する
			pos += vel * deltatime;

			if (pos.y < 0.f) {
				pos.y = 0.f;
				vel.y = 0.0f;
			}
		}
	}

	particleData_.position->CopyBuffer(positions_.data(), sizeof(Vector3) * positions_.size());
}

void SandCPUSample::Draw(DCC* dcc) {
	renderer_->Draw(dcc);
}
