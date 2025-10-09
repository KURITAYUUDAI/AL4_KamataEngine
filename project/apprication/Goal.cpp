#include "Goal.h"
#include "GameScene.h"
#include "MapChipField.h"
#include "Player.h"

using namespace KamataEngine;

GameScene* Goal::gameScene_ = nullptr;

void Goal::Initialize(Model* model, Camera* camera, const Vector3& position) 
{
	// NULLポインタチェック
	assert(model);
	assert(gameScene_);
	model_ = model;
	camera_ = camera;
	// ワールドトランスフォームの初期化
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;
	
	isGoal_ = false;
}

void Goal::Update() 
{
	if(isGoal_)
	{
		worldTransform_.rotation_.y += 0.3f;
	}


	// ゴールの更新処理を書く
	worldTransform_.matWorld_ = 
		MakeAffineMatrixB(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();
}

void Goal::Draw() 
{
	// ゴールの描画処理を書く
	model_->Draw(worldTransform_, *camera_, nullptr);
}

Vector3 Goal::GetWorldPosition() 
{
	// ワールド座標を入れる変数
	Vector3 worldPos;

	// ワールド行列の平行移動成分を取得（ワールド座標）
	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];

	return worldPos;
}

AABB Goal::GetAABB() 
{
	Vector3 worldPos = GetWorldPosition();

	AABB aabb;

	aabb.min = {worldPos.x - kWidth_ / 2.0f, worldPos.y - kHeight_ / 2.0f, worldPos.z - kWidth_ / 2.0f};
	aabb.max = {worldPos.x + kWidth_ / 2.0f, worldPos.y + kHeight_ / 2.0f, worldPos.z + kWidth_ / 2.0f};

	return aabb;
}

void Goal::OnCollision(const Player* player)
{
	(void)player;

	isGoal_ = true;
}