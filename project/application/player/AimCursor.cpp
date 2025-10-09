#include "AimCursor.h"
#include "myMath.h"

void AimCursor::Initialize(Model* model, Camera* camera) 
{
	model_ = model;
	camera_ = camera;
	worldTransform_.Initialize();
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};
	worldTransform_.rotation_ = {0.0f, 90.0f / 180.0f * std::numbers::pi_v < float > , 0.0f};
}

void AimCursor::Update(const Vector3& hitPosition) 
{
	worldTransform_.translation_ = hitPosition;
	worldTransform_.matWorld_ = MakeAffineMatrixB(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();
}

void AimCursor::Draw() 
{
	if (model_) 
	{
		model_->Draw(worldTransform_, *camera_, nullptr);
	}
}