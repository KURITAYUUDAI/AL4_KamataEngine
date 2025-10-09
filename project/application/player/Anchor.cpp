#include "Anchor.h"

void Anchor::Initialize(Model* model, Camera* camera, const Vector3& position, const Vector3& target) 
{
	model_ = model;
	camera_ = camera;
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;

	direction_ = Normalize(target - position);
	velocity_ = direction_ * speed_;
	traveled_ = 0.0f;
	isActive_ = true;
}

void Anchor::Update() 
{
	if (isActive_) 
	{
		worldTransform_.translation_ += velocity_;
		traveled_ += speed_;
		// 回転：X+方向を direction に向ける
		Vector3 dirNorm = Normalize(direction_);
		Matrix4x4 rotMat = MakeLookRotationXAxis(dirNorm, {0, 1, 0}); // ← X+基準で回転

		// 回転行列 → オイラー角
		worldTransform_.rotation_ = GetEulerFromMatrix(rotMat);

		worldTransform_.rotation_.z *= -1.0f;
	
	}
	worldTransform_.matWorld_ = MakeAffineMatrixB(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();
}

void Anchor::Draw() 
{
	model_->Draw(worldTransform_, *camera_);
}