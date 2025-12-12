#define NOMINMAX
#include "Enemy.h"
#include "GameScene.h"
#include "Player.h"
#include "PlayerBullet.h"

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
	/*worldTransform_.rotation_.y = 1.0f / 2.0f * std::numbers::pi_v<float>;*/

	// 速度を設定する
	velocity_ = { 0.0f, 0.0f, -kWalkSpeed};

	// 経過時間
	walkTimer_ = 0.0f;

	behavior_ = Behavior::kApproach;	// 初期モード
	behaviorRequest_ = behavior_;		// リクエストは初期モードと同様

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

	if (worldTransform_.translation_.z)

	// エネミーの更新処理を書く
	worldTransform_.matWorld_ = 
		MakeAffineMatrixB({1.0f, 1.0f, 1.0f}, worldTransform_.rotation_, worldTransform_.translation_);

	// 行列を定数バッファに転送
	worldTransform_.TransferMatrix();

	ImGui::Begin("enemy window");

	ImGui::DragFloat3("rotation", &worldTransform_.rotation_.x, 0.0f);

	ImGui::DragFloat3("translation", &worldTransform_.translation_.x, 0.0f);

	ImGui::Text("Behavior : %d", static_cast<int>(behavior_));

	ImGui::End();
}

void Enemy::Draw()
{
	// オブジェクトカラーを nullptr に設定して描画
	model_->Draw(worldTransform_, *camera_, nullptr);
}

void Enemy::OnCollision(const PlayerBullet* bullet) 
{
	(void)bullet;

	behaviorRequest_ = Behavior::kDead;
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

//void Enemy::OnCollision(const Player* player)
//{
//	(void)player;
//
//	if (behavior_ == Behavior::kDead) 
//	{
//		return;
//	} 
//
//}

void Enemy::BehaviorRootUpdate() 
{
	// 移動
	worldTransform_.translation_ += velocity_;

	//// タイマーを加算
	//walkTimer_ += 1.0f / 60.0f;

	//// 回転アニメーション
	//float param = std::sin(2.0f * std::numbers::pi_v<float> * walkTimer_ / kWalkMotionTime);
	//float degree = kWalkMotionAngleStrat + (kWalkMotionAngleEnd - kWalkMotionAngleStrat) * ((param + 1.0f) / 2.0f);
	//worldTransform_.rotation_.x = DegToRad(degree);
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

	case Behavior::kApproach:
		// Root状態の初期化処理
		currentState_ = std::make_unique<EnemyStateApproach>();
		break;

	case Behavior::kLeave:
		// Root状態の初期化処理
		currentState_ = std::make_unique<EnemyStateLeave>();
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
	(void)enemy; 
}

void EnemyStateDead::Shutdown(Enemy* enemy) 
{ 
	(void)enemy; 
}

void EnemyStateApproach::Initialize(Enemy* enemy) 
{
	(void)enemy;

	velocity_ = {0.0f, 0.0f, 5.0f};
}

void EnemyStateApproach::Update(Enemy* enemy) 
{ 
	enemy->SetTranslation(enemy->GetTranslation() + velocity_ * kDeltaTime); 

	if (enemy->GetTranslation().z > 7.0f)
	{
		enemy->SetBehaviorRequest(Enemy::Behavior::kLeave);
	}
}

void EnemyStateApproach::Draw(Enemy* enemy) 
{ 
	(void)enemy; 
}

void EnemyStateApproach::Shutdown(Enemy* enemy) 
{ 
	(void)enemy; 
}

void EnemyStateLeave::Initialize(Enemy* enemy) 
{ 
	(void)enemy;

	velocity_ = {0.0f, 0.0f, -5.0f}; 
}

void EnemyStateLeave::Update(Enemy* enemy) 
{
	enemy->SetTranslation(enemy->GetTranslation() + velocity_ * kDeltaTime); 

	if (enemy->GetTranslation().z < 2.0f) 
	{
		enemy->SetBehaviorRequest(Enemy::Behavior::kApproach);
	}
}

void EnemyStateLeave::Draw(Enemy* enemy) 
{ 
	(void)enemy; 
}

void EnemyStateLeave::Shutdown(Enemy* enemy) 
{ 
	(void)enemy; 
}
