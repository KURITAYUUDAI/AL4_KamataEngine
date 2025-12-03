#define NOMINMAX
#include "Enemy.h"
#include "GameScene.h"
#include "MapChipField.h"
#include "Player.h"

#include "numbers"

using namespace KamataEngine;

GameScene* Enemy::gameScene_ = nullptr;

void Enemy::Initialize(Model* model, Camera* camera, const Vector3& position)
{
	
	// NULLポインタチェック
	assert(model);

	model_ = model;

	camera_ = camera;

	// ワールドトランスフォームの初期化
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.rotation_.y = 1.0f / 2.0f * std::numbers::pi_v<float>;

	// 速度を設定する
	velocity_ = { -kWalkSpeed, 0.0f, 0.0f };

	// 経過時間
	walkTimer_ = 0.0f;

	behavior_ = Behavior::kRoot;  // 初期モードはルート
	behaviorRequest_ = behavior_; // リクエストモードもルート

	// モード変更
	ChangeBehavior(behaviorRequest_);
}

void Enemy::Update()
{
	if (behaviorRequest_ != behavior_) 
	{
		// モード変更
		ChangeBehavior(behaviorRequest_);
	}

	currentState_->Update(this);

	// エネミーの更新処理を書く
	worldTransform_.matWorld_ = 
		MakeAffineMatrixB({1.0f, 1.0f, 1.0f}, worldTransform_.rotation_, worldTransform_.translation_);

	// 行列を定数バッファに転送
	worldTransform_.TransferMatrix();
}

void Enemy::Draw()
{
	// オブジェクトカラーを nullptr に設定して描画
	model_->Draw(worldTransform_, *camera_, nullptr);
}

Vector3 Enemy::GetWorldPosition() 
{
	// ワールド座標を入れる変数
	Vector3 worldPos;

	// ワールド行列の平行移動成分を取得（ワールド座標）
	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];

	return worldPos;
}

AABB Enemy::GetAABB() 
{
	Vector3 worldPos = GetWorldPosition();

	AABB aabb;

	aabb.min = {worldPos.x - kWidth_ / 2.0f, worldPos.y - kHeight_ / 2.0f, worldPos.z - kWidth_ / 2.0f};
	aabb.max = {worldPos.x + kWidth_ / 2.0f, worldPos.y + kHeight_ / 2.0f, worldPos.z + kWidth_ / 2.0f};

	return aabb;
}

void Enemy::OnCollision(const Player* player)
{
	(void)player;

	if (behavior_ == Behavior::kDead) 
	{
		return;
	} 

}

void Enemy::BehaviorRootUpdate() 
{
	// 移動
	worldTransform_.translation_ += velocity_;

	// タイマーを加算
	walkTimer_ += 1.0f / 60.0f;

	// 回転アニメーション
	float param = std::sin(2.0f * std::numbers::pi_v<float> * walkTimer_ / kWalkMotionTime);
	float degree = kWalkMotionAngleStrat + (kWalkMotionAngleEnd - kWalkMotionAngleStrat) * ((param + 1.0f) / 2.0f);
	worldTransform_.rotation_.x = DegToRad(degree);
}

void Enemy::BehaviorDeadUpdate() 
{

}

void Enemy::ChangeBehavior(Behavior behavior)
{
	if (currentState_) 
	{
		currentState_->Shutdown(this);
		currentState_.reset();
	}

	// 現在の状態を変更する
	behavior_ = behavior;

	// 状態に応じた処理を行う
	switch (behavior_) 
	{
	case Behavior::kRoot:
		// Root状態の初期化処理
		currentState_ = std::make_unique<EnemyStateRoot>();
		break;

	case Behavior::kDead:
		// Attack状態の初期化処理
		currentState_ = std::make_unique<EnemyStateDead>();
		break;

	default:
		break;
	}

	if (!currentState_) 
	{
		return;
	}
	currentState_->Initialize(this);
}

void EnemyStateRoot::Initialize(Enemy* enemy) 
{
	enemy = enemy;
}

void EnemyStateRoot::Update(Enemy* enemy) 
{ 
	enemy->BehaviorRootUpdate(); 
}

void EnemyStateRoot::Draw(Enemy* enemy) 
{
	enemy = enemy;
}

void EnemyStateRoot::Shutdown(Enemy* enemy) 
{ 
	enemy = enemy; 
}

void EnemyStateDead::Initialize(Enemy* enemy) 
{ 
	enemy = enemy; 
	enemy->SetIsCollisionDisabled(true);
}

void EnemyStateDead::Update(Enemy* enemy) 
{ 
	enemy->BehaviorDeadUpdate(); 

	animationTimer_ += 1.0f / 60.0f;
	float t = animationTimer_ / deathAnimation_;

	enemy->SetRotation(
		{ 
		  EaseOut(0.0f, std::numbers::pi_v<float> / 2.0f, t), 
		  EaseOut(0.0f, 2.0f * std::numbers::pi_v<float>, t), 
		  0.0f
		}
	);

	if (animationTimer_ >= deathAnimation_) 
	{
		enemy->SetIsDead(true);
	}
}

void EnemyStateDead::Draw(Enemy* enemy) 
{ 
	enemy = enemy; 
}

void EnemyStateDead::Shutdown(Enemy* enemy) 
{ 
	enemy = enemy; 
}