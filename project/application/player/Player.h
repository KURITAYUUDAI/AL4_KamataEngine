#pragma once
#include "KamataEngine.h"
#include "myMath.h"
#include <functional>

using namespace KamataEngine;

class MapChipField;

class Enemy;

class Player;

class IPlayerState
{
public:
	virtual ~IPlayerState() = default;
	// 初期化
	virtual void Initialize(Player* player) = 0;
	// 更新
	virtual void Update(Player* player) = 0;
	// 描画
	virtual void Draw(Player* player) = 0;
	// 終了
	virtual void Shutdown(Player* player) = 0;
};

class Player 
{

public:

	// 振る舞い
	enum class Behavior 
	{
		kUnknown, // リクエストなし
		kRoot,    // 通常状態
		kAttack,  // 攻撃
		kGrapple,
	};

	enum class AttackBehavior 
	{
		kCharge, // 溜め
		kRush,   // 突進
		kAfter,  // 余韻
	};

	enum class GrappleBehavior 
	{
		kShot,
		kMove,
		kHang,
	};

	// マップとの当たり判定情報
	struct CollisionMapInfo 
	{
		bool hitCeiling = false;
		bool hitGround = false;
		bool hitWall = false;
		Vector3 moveVel;
	};

	void InitCollisionMapInfo(CollisionMapInfo& info) 
	{
		info.hitCeiling = false;
		info.hitGround = false;
		info.hitWall = false;
		info.moveVel = {0.0f, 0.0f, 0.0f};
	}

	// 角
	enum Corner 
	{
		kRightBottom, // 右下
		kLeftBottom,  // 左下
		kRightTop,    // 右上
		kLeftTop,     // 左上

		kNumCorner // 要素数
	};

	// 左右判別クラス
	enum class LRDirection 
	{
		kRight,
		kLeft,
	};

	~Player();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name = "model">モデル</param>
	/// <param name = "textureHandle">テクスチャハンドル</param>
	void Initialize(Model* model, Model* modelAttack, Camera* camera, const Vector3& position);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	void AttackDraw();

	const WorldTransform& GetWorldTransform() const { return worldTransform_; }

	void SetMapChipFiled(MapChipField* mapChipField) { mapChipField_ = mapChipField; }

	void Move();

	void CollisionMap(CollisionMapInfo& info);

	Vector3 CornerPosition(const Vector3& center, Corner corner);

	void CollisionMapTop(CollisionMapInfo& info, std::array<Vector3, 4> positionsNew);
	void CollisionMapBottom(CollisionMapInfo& info, std::array<Vector3, 4> positionsNew);
	void CollisionMapRight(CollisionMapInfo& info, std::array<Vector3, 4> positionsNew);
	void CollisionMapLeft(CollisionMapInfo& info, std::array<Vector3, 4> positionsNew);

	void ResultMove(const CollisionMapInfo& info);

	void CollisionCeiling(const CollisionMapInfo& info);
	void CollisionGround(const CollisionMapInfo& info);
	void CollisionWall(const CollisionMapInfo& info);

	void CorrectPositionByWall(const CollisionMapInfo& info);

	void OnCollision(const Enemy* enemy);

	void BehaviorRootUpdate();

	void BehaviorAttackUpdate();

	// モード変更
	void ChangeBehavior(Behavior behavior);

	Behavior GetBehaviorRequest() const { return behaviorRequest_; }
	void SetBehaviorRequest(Behavior behaviorRequest) { behaviorRequest_ = behaviorRequest; } 

	AttackBehavior GetAttackBehavior() const { return attackBehavior_; }
	void SetAttackBehavior(AttackBehavior attackBehavior) { attackBehavior_ = attackBehavior; }

	GrappleBehavior GetGrappleBehavior() const { return grappleBehavior_; }
	void SetGrappleBehavior(GrappleBehavior grappleBehavior) { grappleBehavior_ = grappleBehavior; }

	bool IsAttack() const;

	/// ゲッター
	// トランスフォーム
	const Vector3& GetScale() const { return worldTransform_.scale_; }
	const Vector3& GetRotation() const { return worldTransform_.rotation_; }
	const Vector3& GetTranslation() const { return worldTransform_.translation_; }
	// 速度
	const Vector3& GetVelocity() const { return velocity_; }
	// ワールドポジション
	const Vector3 GetWorldPosition() const;
	// AABB
	AABB GetAABB();
	LRDirection GetLRDirection() const { return lrDirection_; }
	// デスフラグ
	bool GetIsDead() const { return isDead_; } 
	// Player.h に追加
	const CollisionMapInfo& GetCollisionInfo() const { return collisionMapInfo_; }

	const Vector3& GetGrappleAnchor() const { return grappleAnchorPos_; }

	/// セッター
	// トランスフォーム
	void SetScale(const Vector3& scale) { worldTransform_.scale_ = scale; }
	void SetRotation(const Vector3& rotation) { worldTransform_.rotation_ = rotation; }
	void SetTranslation(const Vector3& translation) { worldTransform_.translation_ = translation; }
	// 速度
	void SetVelocity(const Vector3 velocity) { velocity_ = velocity; }
	
	void SetCollisionMove(Vector3 moveVel) { collisionMapInfo_.moveVel = moveVel; }

	// アンカー位置の設定・取得・リセット
	void SetGrappleAnchor(const Vector3& pos) 
	{
		grappleAnchorPos_ = pos;
		isGrappleAnchorActive_ = true;
	}
	void SetRayHitBlockFunction(const std::function<bool(const Ray&, Vector3&)>& func) 
	{ rayHitBlockFunc_ = func; }
	

	void ShotGrapple();
	void MoveGrapple(const Vector3& target);
	void EndGrapple();
	const Vector3& GetGrappleTarget() const { return grappleTarget_; }
	bool GetEndGrappling() const { return endGrappling_; }

	void ClearGrappleAnchor() { isGrappleAnchorActive_ = false; }
	
	bool IsGrappleAnchorActive() const { return isGrappleAnchorActive_; }

	
	bool RayHitsBlock(const Ray& ray, Vector3& outPos) const 
	{
		if (rayHitBlockFunc_) {
			return rayHitBlockFunc_(ray, outPos);
		}
		return false;
	}

private:

	// ワールド変換データ
	WorldTransform worldTransform_;

	// 攻撃用ワールド変換データ
	WorldTransform worldTransformAttack_;

	// モデル
	Model* model_ = nullptr;

	// 攻撃用モデル
	Model* modelAttack_ = nullptr;
	
	// カメラ
	Camera* camera_ = nullptr;

	// マップチップによるフィールド
	MapChipField* mapChipField_ = nullptr;

	// 移動速度
	Vector3 velocity_;

	// 横移動加速度
	static inline const float kAcceleration = 0.01f;
	// 横移動減速度
	static inline const float kAttenuation = 0.15f;
	// 横移動最大速度
	static inline const float kLimitSpeed = 0.5f;
	
	// 初期値は右
	LRDirection lrDirection_ = LRDirection::kRight;

	// 旋回開始時の角度
	float turnFirstRotationY_ = 0.0f;
	// 旋回タイマー
	float turnTimer_ = 0.0f;

	// 旋回時間<秒>
	static inline const float kTimeTurn = 0.3f;

	//  設置状態フラグ
	bool onGround_ = true;

	// 重力加速度（下方向）
	static inline const float kGravityAcceleration = 0.01f;
	// 最大落下速度（下方向）
	static inline const float kLimitFallSpeed = 0.3f;
	// ジャンプ初速（上方向）
	static inline const float kJumpAcceleration = 0.4f;

	CollisionMapInfo collisionMapInfo_;

	// キャラクターの当たり判定サイズ
	static inline const float kWidth_ = 1.8f;
	static inline const float kHeight_ = 1.8f;

	static inline const float kBlank = 0.001f;
	
	static inline const float kAttenuationLanding = 0.1f;
	static inline const float kBlankLanding = kBlank + 0.1f;

	static inline const float kEpsilonWall = 0.001f;
	static inline const float kAttenuationWall = 0.05f;

	// デスフラグ
	bool isDead_ = false;

	// 現在の状態オブジェクトへのポインタ
	std::unique_ptr<IPlayerState> currentState_;

	// 振る舞い
	Behavior behavior_ = Behavior::kRoot;

	Behavior behaviorRequest_ = Behavior::kUnknown;

	// 攻撃時の振る舞い
	AttackBehavior attackBehavior_ = AttackBehavior::kCharge;

	// ワイヤー移動時の振る舞い
	GrappleBehavior grappleBehavior_ = GrappleBehavior::kShot;

	Vector3 grappleTarget_{};
	bool endGrappling_ = false;
	
	std::function<bool(const Ray&, Vector3&)> rayHitBlockFunc_;

	// アンカーの着弾位置
	Vector3 grappleAnchorPos_{};

	// アンカーが有効かどうか（必要に応じて）
	bool isGrappleAnchorActive_ = false;
};

class PlayerStateRoot : public IPlayerState 
{
public:
	void Initialize(Player* player) override;
	void Update(Player* player) override;
	void Draw(Player* player) override;
	void Shutdown(Player* player) override;

private:

	// 重力加速度（下方向）
	static inline const float kGravityAcceleration = 0.01f;
	// 最大落下速度（下方向）
	static inline const float kLimitFallSpeed = 0.3f;
};

class PlayerStateAttack : public IPlayerState 
{
public:

	void Initialize(Player* player) override;
	void Update(Player* player) override;
	void Draw(Player* player) override;
	void Shutdown(Player* player) override;

private:

	// 攻撃ギミックの経過時間カウンター
	float attackParameter_ = 0.0f;

	Vector3 attackVelocity_ = { 1.0f, 0.0f, 0.0f };

	// 溜め動作時間
	float chargeTime_ = 0.1f;
	// 突進動作時間
	float rushTime_ = 0.2f;
	// 余韻動作時間
	float afterTime_ = 0.1f;

};

class PlayerStateGrapple : public IPlayerState 
{
public:

	void Initialize(Player* player) override;
	void Update(Player* player) override;
	void Draw(Player* player) override;
	void Shutdown(Player* player) override;

	

private:

	float t_ = 0.0f;
	float speed_ = 1.0f;
	// ワイヤー移動の最大速度
	static inline const float kLimitSpeed = 0.7f;
	// 重力加速度（下方向）
	static inline const float kGravityAcceleration = 0.01f;
	// ワイヤー巻き取り加速度
	float kGrapplePullAcceleration = 0.03f; // ワイヤー巻き取り加速度

	Vector3 anchorPos_;     // アンカーの現在位置（飛行中に使う）
	Vector3 direction_;     // 発射方向（normalize済み）
	float traveled_ = 0.0f; // アンカーが飛んだ距離

	// 最大落下速度（下方向）
	static inline const float kLimitFallSpeed = 0.3f;
};