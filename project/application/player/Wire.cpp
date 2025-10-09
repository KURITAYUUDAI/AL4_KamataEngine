#include "Wire.h"
#include "Player.h"
#include "Anchor.h"

void Wire::Initialize(Model* model, Camera* camera, Player* player, Anchor* anchor) 
{
	model_ = model;
	camera_ = camera;
	player_ = player;
	anchor_ = anchor;
	worldTransform_.Initialize();
	worldTransform_.scale_ = {0.1f, 0.1f, 0.1f};
	worldTransform_.rotation_ = {0.0f, 90.0f / 180.0f * std::numbers::pi_v<float>, 0.0f};
}

void Wire::Update() 
{
	if (!anchor_ || !player_)
		return;

	Vector3 start = player_->GetTranslation();
	Vector3 end = anchor_->GetTranslation();
	Vector3 direction = end - start;
	float length = Length(direction);
	if (length < 0.001f) {
		length = 0.001f; // 零長防止
	}

	// ワイヤーの中心に配置（中心が原点のモデル前提）
	worldTransform_.translation_ = Lerp(start, end, 0.5f);

	// スケール：X方向に長さを伸ばす（X+モデル前提）
	worldTransform_.scale_ = {length, 1.0f, 1.0f};

	// 回転：X+方向を direction に向ける
	Vector3 dirNorm = Normalize(direction);
	Matrix4x4 rotMat = MakeLookRotationXAxis(dirNorm, {0, 1, 0}); // ← X+基準で回転

	// 回転行列 → オイラー角
	worldTransform_.rotation_ = GetEulerFromMatrix(rotMat);

	worldTransform_.rotation_.z *= -1.0f;

	// ワールド行列作成
	worldTransform_.matWorld_ = MakeAffineMatrixB(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();
}

void Wire::Draw() 
{
	if (model_) 
	{
		model_->Draw(worldTransform_, *camera_, nullptr);
	}
}

