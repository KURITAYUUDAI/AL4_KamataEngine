#pragma once
#include "KamataEngine.h"
#include "myMath.h"

using namespace KamataEngine;

class GameScene;

class MapChipField;

class Player;

class Enemy;

class IEnemyState {
public:
	virtual ~IEnemyState() = default;
	// 初期化
	virtual void Initialize(Enemy* enemy) = 0;
	// 更新
	virtual void Update(Enemy* enemy) = 0;
	// 描画
	virtual void Draw(Enemy* enemy) = 0;
	// 終了
	virtual void Shutdown(Enemy* enemy) = 0;
};

class Enemy 
{
public:

	enum class Behavior
	{
		kUnknown, // リクエストなし
		kRoot,    // 通常
		kDead,    // デッド状態
	};

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name = "model">モデル</param>
	/// <param name = "textureHandle">テクスチャハンドル</param>
	void Initialize(Model* model, Camera* camera, const Vector3& position);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	// 衝突応答
	void OnCollision(const Player* player);

	void BehaviorRootUpdate();

	void BehaviorDeadUpdate();

	// モード変更
	void ChangeBehavior(Behavior behavior);

	static GameScene* gameScene_;
	
	static void SetGameScene(GameScene* gameScene) { gameScene_ = gameScene; }

	/// ゲッター
	// ビヘイビアリクエスト
	Behavior GetBehaviorRequest() const { return behaviorRequest_; }
	// トランスフォーム
	const Vector3 GetScale() const { return worldTransform_.scale_; }
	const Vector3 GetRotation() const { return worldTransform_.rotation_; }
	const Vector3 GetTranslation() const { return worldTransform_.translation_; }
	// 死亡判定
	bool GetIsDead() const { return isDead_; }	
	// ワールドポジション
	Vector3 GetWorldPosition();
	// AABB
	AABB GetAABB();

	/// セッター
	// ビヘイビアリクエスト
	void SetBehaviorRequest(Behavior behaviorRequest) { behaviorRequest_ = behaviorRequest; }
	// トランスフォーム
	void SetScale(const Vector3& scale) { worldTransform_.scale_ = scale; }
	void SetRotation(const Vector3& rotation) { worldTransform_.rotation_ = rotation; }
	void SetTranslation(const Vector3& translation) { worldTransform_.translation_ = translation; }
	// 死亡判定
	void SetIsDead(bool isDead) { isDead_ = isDead; }
	// コリジョン無効判定
	void SetIsCollisionDisabled(bool isCollisionDisabled) { isCollisionDisabled_ = isCollisionDisabled; }
	
	bool IsCollisionDisabled() const { return isCollisionDisabled_; }

private:

	// ワールド変換データ
	WorldTransform worldTransform_;

	// モデル
	Model* model_ = nullptr;

	// カメラ
	Camera* camera_ = nullptr;

	// 速度
	Vector3 velocity_ = {};

	// 経過時間
	float walkTimer_ = 0.0f;

	// 歩行の速さ
	static inline const float kWalkSpeed = 0.025f;

	// 最初の角度[度]
	static inline const float kWalkMotionAngleStrat = 25.0f;
	// 最初の角度[度]
	static inline const float kWalkMotionAngleEnd = -25.0f;
	// アニメーションの周期となる時間[秒]
	static inline const float kWalkMotionTime = 1.5f;

	// キャラクターの当たり判定サイズ
	static inline const float kWidth_ = 1.8f;
	static inline const float kHeight_ = 1.8f;

	bool isDead_ = false; // デスフラグ

	bool isCollisionDisabled_ = false; // 衝突無効フラグ

	// 現在の状態オブジェクトへのポインタ
	std::unique_ptr<IEnemyState> currentState_;

	// 振る舞い
	Behavior behavior_ = Behavior::kRoot;

	Behavior behaviorRequest_ = Behavior::kUnknown;

	
};

class EnemyStateRoot : public IEnemyState 
{
public:
	void Initialize(Enemy* enemy) override;
	void Update(Enemy* enemy) override;
	void Draw(Enemy* enemy) override;
	void Shutdown(Enemy* enemy) override;
};

class EnemyStateDead : public IEnemyState 
{
public:
	void Initialize(Enemy* enemy) override;
	void Update(Enemy* enemy) override;
	void Draw(Enemy* enemy) override;
	void Shutdown(Enemy* enemy) override;

private:
	float animationTimer_ = 0.0f;

	float deathAnimation_ = 1.0f;
};