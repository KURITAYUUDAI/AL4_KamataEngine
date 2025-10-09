#pragma once
#include "KamataEngine.h"
#include "myMath.h"

using namespace KamataEngine;

class GameScene;

class MapChipField;

class Player;

class Goal
{
public:
	/// <summary>
	/// 初期化
	/// </summary>
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

	static GameScene* gameScene_;

	// トランスフォーム
	const Vector3 GetScale() const { return worldTransform_.scale_; }
	const Vector3 GetRotation() const { return worldTransform_.rotation_; }
	const Vector3 GetTranslation() const { return worldTransform_.translation_; }

	// ワールドポジション
	Vector3 GetWorldPosition();
	AABB GetAABB(); 

	bool GetIsGoal() const { return isGoal_; }

	// トランスフォーム
	void SetScale(const Vector3& scale) { worldTransform_.scale_ = scale; }
	void SetRotation(const Vector3& rotation) { worldTransform_.rotation_ = rotation; }
	void SetTranslation(const Vector3& translation) { worldTransform_.translation_ = translation; }

	static void SetGameScene(GameScene* gameScene) { gameScene_ = gameScene; }
	
private:

	// ワールド変換データ
	WorldTransform worldTransform_;
	// モデル
	Model* model_ = nullptr;
	// カメラ
	Camera* camera_ = nullptr;

	// キャラクターの当たり判定サイズ
	static inline const float kWidth_ = 1.8f;
	static inline const float kHeight_ = 1.8f;

	bool isGoal_ = false;
};