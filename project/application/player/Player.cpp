#define NOMINMAX
#include "Player.h"
#include "Enemy.h"

#include <numbers>
#include <algorithm>

#include "WorldTransformAssist.h"
#include "PlayerBullet.h"

using namespace KamataEngine;

Player::~Player()
{ 
	for (PlayerBullet* bullet : bullets_)
	{
		delete bullet;
	}
	bullets_.clear();

	delete modelBullet_;
}

void Player::Initialize(Model* model, Camera* camera, const Vector3& position)
{
	// プレイヤーの初期化処理を書く

	// NULLポインタチェック
	assert(model);

	model_ = model;

	camera_ = camera;

	// ワールドトランスフォームの初期化
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.rotation_.y = 0.0f;

	velocity_ = { 0.0f,0.0f, 0.0f };

	coolTimer_ = 0.0f;
	reloadTimer_ = 0.0f;

	bulletRemain_ = kMaxBullet;

	behavior_ = Behavior::kRoot; // 初期モードはルート
	behaviorRequest_ = behavior_; // リクエストモードもルート

	// モード変更
	ChangeBehavior(behaviorRequest_);

	modelBullet_ = Model::Create();
}

void Player::Update() 
{
	if (behaviorRequest_ != behavior_)
	{
		// モード変更
		ChangeBehavior(behaviorRequest_);
	}


	currentState_->Update(this);

	for (PlayerBullet* bullet : bullets_) 
	{
		bullet->Update();
	}

	bullets_.remove_if([](PlayerBullet* bullet) 
	{
		if (bullet->GetIsDead()) 
		{
			delete bullet;
			return true;
		}
		return false;
	});

	WorldTransformUpdate(worldTransform_);

	ImGui::Begin("player window");

	ImGui::DragFloat3("rotation", &worldTransform_.rotation_.x, 0.0f);

	ImGui::DragFloat3("translation", &worldTransform_.translation_.x, 0.0f);

	ImGui::Text("bullet num : %d", static_cast<int>(bullets_.size()));

	ImGui::Text("bulletRemain: %d", static_cast<int>(bulletRemain_));

	ImGui::Text("reloadTimer : %f", reloadTimer_);


	ImGui::End();
}

void Player::Draw() 
{
	for (PlayerBullet* bullet : bullets_) {
		bullet->Draw();
	}

	// プレイヤーの描画処理を書く
	// オブジェクトカラーを nullptr に設定して描画
	model_->Draw(worldTransform_, *camera_, nullptr);
}

void Player::Rotate() 
{
	// 押した方向で移動ベクトルを変更
	if (Input::GetInstance()->PushKey(DIK_A)) 
	{
		worldTransform_.rotation_.y -= kRotateSpeed;
	} 
	else if (Input::GetInstance()->PushKey(DIK_D)) 
	{
		worldTransform_.rotation_.y += kRotateSpeed;
	}
}

void Player::Move()
{
	// キャラクターの移動ベクトル
	Vector3 move = { 0.0f, 0.0f, 0.0f };

	// 押した方向で移動ベクトルを変更
	if (Input::GetInstance()->PushKey(DIK_LEFT))
	{
		move.x -= kCharacterSpped;
	} 
	else if (Input::GetInstance()->PushKey(DIK_RIGHT))
	{
		move.x += kCharacterSpped;
	}

	if (Input::GetInstance()->PushKey(DIK_DOWN)) 
	{
		move.y -= kCharacterSpped;
	} 
	else if (Input::GetInstance()->PushKey(DIK_UP)) 
	{
		move.y += kCharacterSpped;
	}

	if (Input::GetInstance()->PushKey(DIK_S)) 
	{
		move.z -= kCharacterSpped;
	} 
	else if (Input::GetInstance()->PushKey(DIK_W)) 
	{
		move.z += kCharacterSpped;
	}

	worldTransform_.translation_ += move;

	worldTransform_.translation_.x = std::clamp(worldTransform_.translation_.x, -kMoveLimitX, kMoveLimitX);
	worldTransform_.translation_.y = std::clamp(worldTransform_.translation_.y, -kMoveLimitY, kMoveLimitY);
	worldTransform_.translation_.z = std::clamp(worldTransform_.translation_.z, -kMoveLimitZ, kMoveLimitZ);

	
}

void Player::Shot() 
{
	if (bulletRemain_ != 0)
	{
		if (Input::GetInstance()->PushKey(DIK_SPACE) && coolTimer_ == 0.0f) 
		{
			Attack();
			coolTimer_ = kBulletCoolTime;
			bulletRemain_--;

			if (bulletRemain_ == 0) 
			{
				reloadTimer_ = kReloadTime;
			}
		}
	}

	if (coolTimer_ != 0.0f) 
	{
		coolTimer_ -= kDeltaTime;
		if (coolTimer_ < 0.0f) 
		{
			coolTimer_ = 0.0f;
		}
	}

	if (reloadTimer_ != 0.0f)
	{
		reloadTimer_ -= kDeltaTime;
		if (reloadTimer_ < 0.0f)
		{
			reloadTimer_ = 0.0f;

			bulletRemain_ = kMaxBullet;
		}
	}
}

void Player::Attack() 
{
	// 弾の速度
	Vector3 velocity(0, 0, kBulletSpeed);

	velocity = TransformNormal(velocity, worldTransform_.matWorld_);

	PlayerBullet* bullet = new PlayerBullet;

	bullet->Initialize(modelBullet_, camera_, worldTransform_.translation_, velocity);

	bullets_.push_back(bullet);
}

const Vector3 Player::GetWorldPosition() const
{
	// ワールド座標を入れる変数
	Vector3 worldPos;

	// ワールド行列の平行移動成分を取得（ワールド座標）
	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];

	return worldPos;
}

AABB Player::GetAABB()
{ 
	Vector3 worldPos = GetWorldPosition();
	
	AABB aabb;

	aabb.min = {worldPos.x - kWidth_ / 2.0f, worldPos.y - kHeight_ / 2.0f, worldPos.z - kWidth_ / 2.0f };
	aabb.max = {worldPos.x + kWidth_ / 2.0f, worldPos.y + kHeight_ / 2.0f, worldPos.z + kWidth_ / 2.0f };

	return aabb;
}

void Player::DestroyBullet(PlayerBullet* bullet) 
{
	for (auto it = bullets_.begin(); it != bullets_.end(); ++it) 
	{
		if (*it == bullet) 
		{
			delete *it;
			bullets_.erase(it);
			break;
		}
	}
}

void Player::OnCollision(const Enemy* enemy)
{
	//// ジャンプさせる
	//velocity_ = velocity_ + Vector3(0, kJumpAcceleration, 0);

	(void)enemy;

	isDead_ = true;
}



void Player::BehaviorRootUpdate() 
{

	Rotate();

	Move();

	Shot();
	
}

void Player::ChangeBehavior(Behavior behavior) 
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
		currentState_ = std::make_unique<PlayerStateRoot>();
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

void PlayerStateRoot::Initialize(Player* player) 
{ 
	player = player;
}

void PlayerStateRoot::Update(Player* player)
{ 
	player->BehaviorRootUpdate();
	//if (Input::GetInstance()->TriggerKey(DIK_SPACE)) 
	//{
	//	// 攻撃状態に変更
	//	player->SetBehaviorRequest(Player::Behavior::kAttack);
	//	return;
	//}
}

void PlayerStateRoot::Draw(Player* player) 
{ 
	player = player; 
}

void PlayerStateRoot::Shutdown(Player* player) 
{
	player = player;
}