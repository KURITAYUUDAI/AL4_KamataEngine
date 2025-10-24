#define NOMINMAX
#include "Player.h"
#include "MapChipField.h"
#include "Enemy.h"

#include <numbers>
#include <algorithm>


using namespace KamataEngine;

Player::~Player()
{

}

void Player::Initialize(Model* model, Model* modelAttack, Camera* camera, const Vector3& position)
{
	// プレイヤーの初期化処理を書く

	// NULLポインタチェック
	assert(model);

	model_ = model;

	modelAttack_ = modelAttack;

	camera_ = camera;

	// ワールドトランスフォームの初期化
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;

	// 攻撃用ワールドトランスフォームの初期化
	worldTransformAttack_.Initialize();

	velocity_ = { 0.0f,0.0f, 0.0f };

	behavior_ = Behavior::kRoot; // 初期モードはルート
	behaviorRequest_ = behavior_; // リクエストモードもルート

	// モード変更
	ChangeBehavior(behaviorRequest_);
}

void Player::Update() 
{
	if (behaviorRequest_ != behavior_)
	{
		// モード変更
		ChangeBehavior(behaviorRequest_);
	}

	// 衝突情報を初期化
	InitCollisionMapInfo(collisionMapInfo_);

	currentState_->Update(this);

	// 旋回制御
	if (turnTimer_ > 0.0f) 
	{
		turnTimer_ -= 1.0f / 60.0f;

		// 左右の自キャラ角度テーブル
		float destinationRotationYTable[] = 
		{
		    std::numbers::pi_v<float> / 2.0f,
		    std::numbers::pi_v<float> * 3.0f / 2.0f,
		};

		// 状況に応じた角度を取得する
		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];
		// 自キャラの角度を設定する
		worldTransform_.rotation_.y = Lerp(destinationRotationY, turnFirstRotationY_, turnTimer_ / kTimeTurn);
	}

	

	// マップ衝突チェック
	CollisionMap(collisionMapInfo_);

	ResultMove(collisionMapInfo_);

	CollisionCeiling(collisionMapInfo_);
	CollisionGround(collisionMapInfo_);
	CollisionWall(collisionMapInfo_);

	// プレイヤーの更新処理を書く
	worldTransform_.matWorld_ = MakeAffineMatrixB(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	// トランスフォームの値を攻撃用トランスフォームにコピー
	worldTransformAttack_.rotation_ = worldTransform_.rotation_;
	worldTransformAttack_.translation_ = worldTransform_.translation_;

	// 攻撃用ワールドトランスフォームの行列を計算
	worldTransformAttack_.matWorld_ = MakeAffineMatrixB(worldTransformAttack_.scale_, worldTransformAttack_.rotation_, worldTransformAttack_.translation_);

	// 行列を定数バッファに転送
	worldTransform_.TransferMatrix();

	// 攻撃用ワールドトランスフォームを定数バッファに転送
	worldTransformAttack_.TransferMatrix();
}

void Player::Draw() 
{
	// プレイヤーの描画処理を書く

	// オブジェクトカラーを nullptr に設定して描画
	model_->Draw(worldTransform_, *camera_, nullptr);

	currentState_->Draw(this);

}

void Player::Move()
{
	// 左右移動操作
	if (Input::GetInstance()->PushKey(DIK_D) || Input::GetInstance()->PushKey(DIK_A)) {
		// 左右加速
		Vector3 acceleration = {};
		if (Input::GetInstance()->PushKey(DIK_D)) {
			// 右移動中の左入力
			if (velocity_.x < 0.0f) {
				velocity_.x *= (1.0f - kAttenuation);
			}
			acceleration.x += kAcceleration;

			if (lrDirection_ != LRDirection::kRight) {
				lrDirection_ = LRDirection::kRight;
				turnFirstRotationY_ = worldTransform_.rotation_.y;
				turnTimer_ = kTimeTurn;
			}
		} else if (Input::GetInstance()->PushKey(DIK_A)) {
			// 左移動中の右入力
			if (velocity_.x > 0.0f) {
				velocity_.x *= (1.0f - kAttenuation);
			}
			acceleration.x -= kAcceleration;

			if (lrDirection_ != LRDirection::kLeft) {
				lrDirection_ = LRDirection::kLeft;
				turnFirstRotationY_ = worldTransform_.rotation_.y;
				turnTimer_ = kTimeTurn;
			}
		}
		// 加速／減速
		velocity_ = velocity_ + acceleration;

		// 最大速度制限
		velocity_.x = std::clamp(velocity_.x, -kLimitSpeed, kLimitSpeed);
	} else {
		velocity_.x = velocity_.x * (1.0f - kAttenuation);
	}

	if (onGround_)
	{
		if (Input::GetInstance()->TriggerKey(DIK_W)) {
			// ジャンプ初速
			velocity_ = velocity_ + Vector3(0, kJumpAcceleration, 0);
		}
	}	
	// 空中
    // 設置状態
	else
	{
		// 落下速度
		velocity_ = velocity_ + Vector3(0, -kGravityAcceleration, 0);
		// 落下速度制限
		velocity_.y = std::max(velocity_.y, -kLimitFallSpeed);
	}
}

void Player::CollisionMap(CollisionMapInfo& info)
{
	// 移動後の4つの角の座標
	std::array<Vector3, 4> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size (); ++i)
	{
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.moveVel, static_cast<Corner>(i));
	}

	CollisionMapTop(info, positionsNew);
	CollisionMapBottom(info, positionsNew);
	CollisionMapRight(info, positionsNew);
	CollisionMapLeft(info, positionsNew);
}

Vector3 Player::CornerPosition(const Vector3& center, Corner corner)
{ 
	Vector3 offsetTable[kNumCorner] = 
	{
	    {+kWidth_ / 2.0f, -kHeight_ / 2.0f, 0},
	    {-kWidth_ / 2.0f, -kHeight_ / 2.0f, 0},
	    {+kWidth_ / 2.0f, +kHeight_ / 2.0f, 0},
	    {-kWidth_ / 2.0f, +kHeight_ / 2.0f, 0},
	};

	return center + offsetTable[static_cast<uint32_t>(corner)];
}

void Player::CollisionMapTop(CollisionMapInfo& info, std::array<Vector3, 4> positionsNew) 
{ 
	// 上昇している場合はreturn
	if (info.moveVel.y <= 0) {
		return;
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;
	// 真上の当たり判定を行う
	bool hit = false;
	// 左上点の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex + 1);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) 
	{
		hit = true;
	}
	// 右上点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex + 1);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) 
	{
		hit = true;
	}

	if (hit) 
	{
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
		// 現在座標が壁の外か判定
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(
			CornerPosition(worldTransform_.translation_, Corner::kLeftTop));
		if (indexSetNow.yIndex != indexSet.yIndex)
		{
			// めり込み先ブロックの範囲矩形
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.moveVel.y = std::max(0.0f, (rect.bottom - worldTransform_.translation_.y) - (kHeight_ / 2.0f + kBlank));
			// 天井に当たったことを記録する
			info.hitCeiling = true;
		}
	}
}

void Player::CollisionMapBottom (CollisionMapInfo& info, std::array<Vector3, 4> positionsNew) 	
{
	// 下降しているか
	if (info.moveVel.y >= 0)
	{
		return;
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;
	// 真下の当たり判定を行う
	bool hit = false;

	MapChipField::IndexSet indexSet;
	// 左下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) 
	{
		hit = true;
	}
	// 右下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) 
	{
		hit = true;
	}

	// ブロックにヒット？
	if (hit) 
	{
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
		// 現在座標が壁の外か判定
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(
			CornerPosition(worldTransform_.translation_, Corner::kLeftBottom));
		if (indexSetNow.yIndex != indexSet.yIndex)
		{
			// めり込み先ブロックの範囲矩形
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.moveVel.y = std::min(0.0f, (rect.top - worldTransform_.translation_.y) + (kHeight_ / 2.0f + kBlank));
			// 地面に当たったことを記録する
			info.hitGround = true;
		}
		
	}
}

void Player::CollisionMapRight (CollisionMapInfo& info, std::array<Vector3, 4> positionsNew)
{
	// 右移動があるか
	if (info.moveVel.x <= 0)
	{
		return;
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;
	// 右の当たり判定を行う
	bool hit = false;

	MapChipField::IndexSet indexSet;
	// 右上点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex - 1, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) 
	{
		hit = true;
	}
	// 右下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex - 1, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) 
	{
		hit = true;
	}

	// ブロックにヒット？
	if (hit) 
	{
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
		// 現在座標が壁の外か判定
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(
			CornerPosition(worldTransform_.translation_, Corner::kRightTop));
		if (indexSetNow.xIndex != indexSet.xIndex)
		{
			// めり込み先ブロックの範囲矩形
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.moveVel.x = std::max(0.0f, (rect.left - worldTransform_.translation_.x) - (kWidth_ / 2.0f + kBlank));
			// 地面に当たったことを記録する
			info.hitWall = true;
		}
	}
}

void Player::CollisionMapLeft (CollisionMapInfo& info, std::array<Vector3, 4> positionsNew)
{
	// 左移動があるか
	if (info.moveVel.x >= 0) 
	{
		return;
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;
	// 左の当たり判定を行う
	bool hit = false;

	MapChipField::IndexSet indexSet;
	// 左上点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex + 1, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) 
	{
		hit = true;
	}
	// 右下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex + 1, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) 
	{
		hit = true;
	}

	// ブロックにヒット？
	if (hit) 
	{
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
		// 現在座標が壁の外か判定
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(
			CornerPosition(worldTransform_.translation_, Corner::kLeftBottom));
		if (indexSetNow.xIndex != indexSet.xIndex)
		{
			// めり込み先ブロックの範囲矩形
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.moveVel.x = std::min(0.0f, (rect.right - worldTransform_.translation_.x) + (kWidth_ / 2.0f + kBlank));
			// 地面に当たったことを記録する
			info.hitWall = true;
		}
	}
}

void Player::ResultMove(const CollisionMapInfo& info) 	
{
	// 移動
	worldTransform_.translation_ += info.moveVel;
}

void Player::CollisionCeiling(const CollisionMapInfo& info)
{
	// 天井に当たったか
	if (info.hitCeiling)
	{
		velocity_.y = 0;
	}
}

void Player::CollisionGround(const CollisionMapInfo& info) 
{
	// 自キャラが設置状態か
	if (onGround_)
	{
		// ジャンプ開始
		if (velocity_.y > 0.0f)
		{
			onGround_ = false;
		}
		else
		{
			// 落下判定
			MapChipType mapChipType;
			// 真下の当たり判定を行う
			bool hit = false;

			MapChipField::IndexSet indexSet;

			// 左下点の判定
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(
				CornerPosition(worldTransform_.translation_, Corner::kLeftBottom) 
				+ Vector3(0, -kBlankLanding, 0));
			mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
			if (mapChipType == MapChipType::kBlock) 
			{
				hit = true;
			}

			// 右下点の判定
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(
				CornerPosition(worldTransform_.translation_, Corner::kRightBottom) 
				+ Vector3(0, -kBlankLanding, 0));
			mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
			if (mapChipType == MapChipType::kBlock) 
			{
				hit = true;
			}
			
			// 落下なら空中状態に切り換え
			// 落下開始
			if (!hit) 
			{
				// 空中状態に切り替える
				onGround_ = false;
			}
		}
	}
	else
	{
		// 着地フラグ
		if (info.hitGround)
		{
			// 着地状態に切り替える（落下を止める）
			onGround_ = true;
			// 着地時にX速度を減衰
			velocity_.x *= (1.0f - kAttenuationLanding);
			// Y速度をゼロにする
			velocity_.y = 0.0f;
		}
	}
}

void Player::CollisionWall(const CollisionMapInfo& info)
{
	// 壁接触による減速
	if (info.hitWall)
	{
		velocity_.x *= (1.0f - kAttenuationWall);
		if (std::abs(velocity_.x) < kBlank)
		{
			velocity_.x = 0;
		}
	}
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

void Player::OnCollision(const Enemy* enemy)
{
	//// ジャンプさせる
	//velocity_ = velocity_ + Vector3(0, kJumpAcceleration, 0);

	if (IsAttack())
	{
		return;
	}

	(void)enemy;

	isDead_ = true;
}

void Player::BehaviorRootUpdate() 
{
	
	Move();

	
	// 移動量に速度の値をコピー
	collisionMapInfo_.moveVel = velocity_;
	
}

void Player::BehaviorAttackUpdate()
{ 
	


	float kAttackSpeed = 0.5f;

	// 向いている方向に向かって突進する
	if (lrDirection_ == LRDirection::kRight)
	{
		velocity_ = { kAttackSpeed, 0.0f, 0.0f };
	} 
	else
	{
		velocity_ = {-kAttackSpeed, 0.0f, 0.0f};
	}

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

	case Behavior::kAttack:
		// Attack状態の初期化処理
		currentState_ = std::make_unique<PlayerStateAttack>();
		break;

	case Behavior::kGrapple:
		// Grapple状態の初期化処理
		currentState_ = std::make_unique<PlayerStateGrapple>();
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

bool Player::IsAttack() const
{ 
	if (behavior_ == Player::Behavior::kAttack && attackBehavior_ == Player::AttackBehavior::kRush)
	{
		return true;
	}
	return false;
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

void PlayerStateAttack::Initialize(Player* player) 
{ 
	player = player;
	attackParameter_ = 0;
	player->SetAttackBehavior(Player::AttackBehavior::kCharge);
	player->SetVelocity({0.0f, 0.0f, 0.0f});

	
}

void PlayerStateAttack::Update(Player* player) 
{ 
	// 予備動作
	attackParameter_ += 1.0f / 60.0f;

	float t;

	Vector3 velocity {};

	switch (player->GetAttackBehavior()) {

	case Player::AttackBehavior::kCharge:

	default: 
	{
		t = static_cast<float>(attackParameter_) / chargeTime_;
		player->SetScale({ 1.0f, EaseOut(1.0f, 1.6f, t), EaseOut(1.0f, 0.3f, t) });
		if (attackParameter_ >= chargeTime_)
		{
			player->SetAttackBehavior(Player::AttackBehavior::kRush);
			attackParameter_ = 0;
		}

		break;
	}

	case Player::AttackBehavior::kRush:

		if (player->GetLRDirection() == Player::LRDirection::kRight)
		{
			velocity = + attackVelocity_;
		} 
		else
		{
			velocity = - attackVelocity_;
		}

		t = static_cast<float>(attackParameter_) / rushTime_;
		player->SetScale({1.0f, EaseOut(1.6f, 0.7f, t), EaseOut(0.3f, 1.3f, t)});
		if (attackParameter_ >= rushTime_) 
		{
			player->SetAttackBehavior(Player::AttackBehavior::kAfter);
			attackParameter_ = 0;
		}

		break;

	case Player::AttackBehavior::kAfter :

		t = static_cast<float>(attackParameter_) / afterTime_;
		player->SetScale({1.0f, EaseOut(0.7f, 1.0f, t), EaseOut(1.3f, 1.0f, t)});
		if (attackParameter_ >= afterTime_) 
		{
			player->SetBehaviorRequest(Player::Behavior::kRoot);
			attackParameter_ = 0;
		}

		break;
	}

	player->SetCollisionMove(velocity);

}

void Player::AttackDraw()
{ 
	modelAttack_->Draw(worldTransformAttack_, *camera_, nullptr); 
}

void PlayerStateAttack::Draw(Player* player) 
{ 
	if (player->GetAttackBehavior() == Player::AttackBehavior::kRush) 
	{
		player->AttackDraw();
	}
}

void PlayerStateAttack::Shutdown(Player* player) 
{ 
	player = player; 
}

void Player::ShotGrapple()
{
	SetBehaviorRequest(Behavior::kGrapple);
	endGrappling_ = false;
}

void Player::MoveGrapple(const Vector3& target) 
{
	grappleBehavior_ = GrappleBehavior::kMove;
	grappleTarget_ = target;
}

void Player::EndGrapple() 
{ 
	endGrappling_ = true; 
}

void PlayerStateGrapple::Initialize(Player* player)
{
	t_ = 0.0f;
	player->SetVelocity({});
	player->SetGrappleBehavior(Player::GrappleBehavior::kShot);
}

void PlayerStateGrapple::Update(Player* player) 
{
	// 初期速度は維持
	Vector3 vel;

	switch (player->GetGrappleBehavior())
	{

	case Player::GrappleBehavior::kShot:

		// 初期速度は維持
		vel = player->GetVelocity();

		// 移動処理
		player->SetVelocity(vel);
		player->SetCollisionMove(vel);

		break;

	case Player::GrappleBehavior::kMove:

		Vector3 pos = player->GetTranslation();
		Vector3 target = player->GetGrappleTarget();

		// 初期速度は維持
		vel = player->GetVelocity();

		// 引っ張り方向
		Vector3 pullDir = Normalize(target - pos);

		// ターゲットまでの距離
		float dist = Length(target - pos);

		// 巻き取り力（距離に応じて調整可能）
		Vector3 pullForce = pullDir * 0.2f; // ← この数値を調整すると引き寄せ具合が変化

		// 引っ張り力を加算して慣性を維持
		vel += pullForce;

		// 最大速度制限
		float maxSpeed = 0.5f;
		if (Length(vel) > maxSpeed) 
		{
			vel = Normalize(vel) * maxSpeed;
		}

		// 移動処理
		player->SetVelocity(vel);
		player->SetCollisionMove(vel);

		// 終了条件（近づいた or 停止）
		if (dist < 2.0f || Length(vel) < 0.01f) 
		{
			player->EndGrapple();
			player->SetBehaviorRequest(Player::Behavior::kRoot);
		}

		if (Input::GetInstance()->TriggerKey(DIK_F3)) {
			player->EndGrapple();
			player->SetBehaviorRequest(Player::Behavior::kRoot);
		}

		break;

	//case Player::GrappleBehavior::kHang:

	//	t_ += 1.0f / 60.0f;
	//	// 振り子運動の計算
	//	float length = Length(player->GetGrappleAnchor() - player->GetTranslation());
	//	float gravity = 9.8f;
	//	float omega = std::sqrt(gravity / length);          // 角周波数
	//	float angle = initialAngle_ * std::cos(omega * t_); // 角度
	//	// 位置の更新
	//	Vector3 offset = {length * std::sin(angle), -length * std::cos(angle) + length, 0.0f};
	//	if (player->GetLRDirection() == Player::LRDirection::kLeft) {
	//		offset.x = -offset.x;
	//	}
	//	Vector3 newPos = player->GetGrappleAnchor() + offset;
	//	Vector3 move = newPos - player->GetTranslation();
	//	player->SetVelocity(move);
	//	player->SetCollisionMove(move);
	//	if (std::abs(angle) < 0.1f) {
	//		player->EndGrapple();
	//		player->SetBehaviorRequest(Player::Behavior::kRoot);
	//	}
	//	break;

	}

	
}

void PlayerStateGrapple::Draw(Player* player) 
{ 
	player = player;
}

void PlayerStateGrapple::Shutdown(Player* player) 
{ 
	player = player;
}
