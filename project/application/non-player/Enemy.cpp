#define NOMINMAX
#include "Enemy.h"
#include "GameScene.h"
#include "Player.h"
#include "PlayerBullet.h"
#include "Anchor.h"
#include "BulletManager.h"

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

	coolTimer_ = 0.0f;
	reloadTimer_ = 0.0f;

	bulletRemain_ = kMaxBullet;

	hitPoint_ = kMaxHitPoint;

	// 経過時間
	walkTimer_ = 0.0f;

	behavior_ = Behavior::kRoot;	// 初期モード
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

	isGrappled_ = false;

	currentState_->Update(this);

	if (damageTimer_ > 0.0f) 
	{
		damageTimer_ -= kDeltaTime;
	}
	if (damageTimer_ < 0.0f) 
	{
		damageTimer_ = 0.0f;
	}

	if (worldTransform_.translation_.z)

	// エネミーの更新処理を書く
	worldTransform_.matWorld_ = 
		MakeAffineMatrixB({1.0f, 1.0f, 1.0f}, worldTransform_.rotation_, worldTransform_.translation_);

	// 行列を定数バッファに転送
	worldTransform_.TransferMatrix();

#ifdef _DEBUG

	ImGui::Begin("enemy window");

	ImGui::DragFloat3("rotation", &worldTransform_.rotation_.x, 0.0f);

	ImGui::DragFloat3("translation", &worldTransform_.translation_.x, 0.0f);

	ImGui::Text("Behavior : %d", static_cast<int>(behavior_));

	ImGui::End();

#endif
}

void Enemy::Draw()
{
	if (static_cast<int>(damageTimer_ * 60.0f) % 5 == 0) 
	{
		// オブジェクトカラーを nullptr に設定して描画
		model_->Draw(worldTransform_, *camera_, nullptr);
	}
}

void Enemy::Shot(const Vector3& velocity, const float& bulletCoolTime, const bool& isNeedReload) 
{
	if (bulletRemain_ != 0) {
		if (coolTimer_ == 0.0f) 
		{
			// 弾の生成
			BulletManager::GetInstance()->CreateEnemyBullet(worldTransform_.translation_, velocity);

			coolTimer_ = bulletCoolTime;
			if (isNeedReload)
			{
				bulletRemain_--;

			}
			
			if (bulletRemain_ == 0) {
				reloadTimer_ = kReloadTime;
			}
		}
	}

	if (coolTimer_ != 0.0f) {
		coolTimer_ -= kDeltaTime;
		if (coolTimer_ < 0.0f) {
			coolTimer_ = 0.0f;
		}
	}

	if (reloadTimer_ != 0.0f) {
		reloadTimer_ -= kDeltaTime;
		if (reloadTimer_ < 0.0f) {
			reloadTimer_ = 0.0f;

			bulletRemain_ = kMaxBullet;
		}
	}
}

void Enemy::OnCollision(const PlayerBullet* bullet) 
{
	(void)bullet;

	if (damageTimer_ == 0.0f) 
	{
		hitPoint_--;
		damageTimer_ = kDamageInvincible_;
	}
	if (hitPoint_ <= 0)
	{
		behaviorRequest_ = Behavior::kDead;
		return;
	}
}

void Enemy::OnCollision(const Anchor* anchor) 
{ 
	behaviorRequest_ = Behavior::kGrappled;
	isGrappled_ = true;
	grappleAnchor_ = anchor;
}

void Enemy::OnCollision(const EnemyBullet* enemyBullet) 
{ 
	(void)enemyBullet;

	if (behavior_ == Behavior::kGrappled)
	{
		isGrappledHit_ = true;
		return;
	}

	behaviorRequest_ = Behavior::kDead;
}

void Enemy::OnCollision(const Enemy* enemy) 
{ 
	(void)enemy; 
	if (behavior_ == Behavior::kShoot) 
	{
		return;
	}

	if (damageTimer_ == 0.0f) 
	{
		hitPoint_ -= 3;
		damageTimer_ = kDamageInvincible_;
	}
	if (hitPoint_ <= 0) {
		behaviorRequest_ = Behavior::kDead;
		return;
	}
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
	worldTransform_.translation_ += velocity_ * kDeltaTime;

	//// タイマーを加算
	//walkTimer_ += 1.0f / 60.0f;

	//// 回転アニメーション
	//float param = std::sin(2.0f * std::numbers::pi_v<float> * walkTimer_ / kWalkMotionTime);
	//float degree = kWalkMotionAngleStrat + (kWalkMotionAngleEnd - kWalkMotionAngleStrat) * ((param + 1.0f) / 2.0f);
	//worldTransform_.rotation_.x = DegToRad(degree);

	if (worldTransform_.translation_.z < 5.0f) 
	{
		behaviorRequest_ = Behavior::kDead;
	}

	// 弾の速度
	Vector3 bulletVelocity(0, 0, kBulletSpeed);

	bulletVelocity = TransformNormal(bulletVelocity, worldTransform_.matWorld_);

	Shot(bulletVelocity, kBulletCoolTime, false);
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

	case Behavior::kGrappled:
		// Grappled状態の初期化処理
		currentState_ = std::make_unique<EnemyStateGrappled>();
		break;

	case Behavior::kShoot:
		// Shoot状態の初期化処理
		currentState_ = std::make_unique<EnemyStateShoot>();
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

//void EnemyStateApproach::Initialize(Enemy* enemy) 
//{
//	(void)enemy;
//
//	velocity_ = {0.0f, 0.0f, 5.0f};
//}
//
//void EnemyStateApproach::Update(Enemy* enemy) 
//{ 
//	enemy->SetTranslation(enemy->GetTranslation() + velocity_ * kDeltaTime); 
//
//	if (enemy->GetTranslation().z > 7.0f)
//	{
//		enemy->SetBehaviorRequest(Enemy::Behavior::kLeave);
//	}
//}
//
//void EnemyStateApproach::Draw(Enemy* enemy) 
//{ 
//	(void)enemy; 
//}
//
//void EnemyStateApproach::Shutdown(Enemy* enemy) 
//{ 
//	(void)enemy; 
//}
//
//void EnemyStateLeave::Initialize(Enemy* enemy) 
//{ 
//	(void)enemy;
//
//	velocity_ = {0.0f, 0.0f, -5.0f}; 
//}
//
//void EnemyStateLeave::Update(Enemy* enemy) 
//{
//	enemy->SetTranslation(enemy->GetTranslation() + velocity_ * kDeltaTime); 
//
//	if (enemy->GetTranslation().z < 2.0f) 
//	{
//		enemy->SetBehaviorRequest(Enemy::Behavior::kApproach);
//	}
//}
//
//void EnemyStateLeave::Draw(Enemy* enemy) 
//{ 
//	(void)enemy; 
//}
//
//void EnemyStateLeave::Shutdown(Enemy* enemy) 
//{ 
//	(void)enemy; 
//}

void EnemyStateGrappled::Initialize(Enemy* enemy) 
{ 
	(void)enemy;

	assert(enemy->GetGrappleAnchor()); 

	hitPoint_ = kMaxHitPoint;
}

void EnemyStateGrappled::Update(Enemy* enemy) 
{ 
	switch (enemy->GetGrappleAnchor()->GetMode())
	{ 
	case Anchor::Mode::kGrapple:
		enemy->SetTranslation(enemy->GetGrappleAnchor()->GetWorldPosition() + 
			TransformNormal({0.0f, 0.0f, enemy->GetWidth() / 2.0f}, enemy->GetGrappleAnchor()->GetWorldMatrix())); 

		break;

	case Anchor::Mode::kHold:

		enemy->SetTranslation(enemy->GetGrappleAnchor()->GetWorldPosition() 
			+ TransformNormal({0.0f, 0.0f, enemy->GetWidth() / 2.0f}, enemy->GetGrappleAnchor()->GetWorldMatrix())); 

		break;
	}

	//// 弾の速度
	//Vector3 bulletVelocity(0, 0, kBulletSpeed);

	//enemy->Shot(bulletVelocity, kBulletCoolTime, true);

	if (enemy->GetGrappleAnchor()->GetIsShoot()) 
{
		enemy->SetBehaviorRequest(Enemy::Behavior::kShoot);
	}

	if (enemy->GetIsGrappledHit())
	{
		hitPoint_ -= 1;
		enemy->SetIsGrappledHit(false);
	}

	if (hitPoint_ <= 0) 
	{
		enemy->SetBehaviorRequest(Enemy::Behavior::kDead);
	}
}

void EnemyStateGrappled::Draw(Enemy* enemy) 
{ 
	(void)enemy; 
}

void EnemyStateGrappled::Shutdown(Enemy* enemy) 
{
	(void)enemy; 
}

void EnemyStateShoot::Initialize(Enemy* enemy) 
{ 
	(void)enemy; 

	shootVelocity_ = TransformNormal({0.0f, 0.0f, shootSpeed_}, enemy->GetGrappleAnchor()->GetWorldMatrix());
}

void EnemyStateShoot::Update(Enemy* enemy) 
{
	shootTimer_ += 1.0f / 60.0f;

	enemy->SetTranslation(enemy->GetTranslation() + shootVelocity_);

	if (shootTimer_ >= maxShootTimer_) 
	{
		enemy->SetIsDead(true);
	}
}

void EnemyStateShoot::Draw(Enemy* enemy) 
{
	(void)enemy; 
}

void EnemyStateShoot::Shutdown(Enemy* enemy) 
{ 
	(void)enemy; 
}
