#pragma once

#include "KamataEngine.h"
#include "myMath.h"

class HitEffect;

class Enemy;

class IHitEffectState 
{
public:
	virtual ~IHitEffectState() = default;
	// 初期化
	virtual void Initialize(HitEffect* hitEffect) = 0;
	// 更新
	virtual void Update(HitEffect* hitEffect) = 0;
	// 描画
	virtual void Draw(HitEffect* hitEffect) = 0;
	// 終了
	virtual void Shutdown(HitEffect* hitEffect) = 0;
};


class HitEffect 
{
public:

	enum class Behavior 
	{
		kUnknown, // リクエストなし
		kSpred,   // スプレッド
		kFade,    // フェード
	};

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name = "model">モデル</param>
	/// <param name = "textureHandle">テクスチャハンドル</param>
	void Initialize(const Vector3& position);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	// インスタンス生成と初期化
	static HitEffect* Create(const Vector3& position);



	static void SetModel(Model* model) { model_ = model; }
	static void SetCamera(Camera* camera) { camera_ = camera; }



	void BehaviorSpredUpdate();

	void BehaviorFeadUpdate();

	// モード変更
	void ChangeBehavior(Behavior behavior);

	/// ゲッター
	// ビヘイビアリクエスト
	Behavior GetBehaviorRequest() const { return behaviorRequest_; }
	//// トランスフォーム
	//const Vector3 GetScale() const { return worldTransform_.scale_; }
	//const Vector3 GetRotation() const { return worldTransform_.rotation_; }
	//const Vector3 GetTranslation() const { return worldTransform_.translation_; }
	// 死亡判定
	bool GetIsDead() const { return isDead_; }	

	/// セッター
	// ビヘイビアリクエスト
	void SetBehaviorRequest(Behavior behaviorRequest) { behaviorRequest_ = behaviorRequest; }
	//// トランスフォーム
	//void SetScale(const Vector3& scale) { worldTransform_.scale_ = scale; }
	//void SetRotation(const Vector3& rotation) { worldTransform_.rotation_ = rotation; }
	//void SetTranslation(const Vector3& translation) { worldTransform_.translation_ = translation; }
	// 死亡判定
	void SetIsDead(bool isDead) { isDead_ = isDead; }

private:

	static Model* model_;

	static Camera* camera_;



	// 円形のワールドトランスフォーム
	WorldTransform circleWorldTransform_;

	// 楕円のワールドトランスフォーム
	std::array<WorldTransform, 2> ellipseWorldTransform_;

	bool isDead_ = false; // デスフラグ

	// 現在の状態オブジェクトへのポインタ
	std::unique_ptr<IHitEffectState> currentState_;

	// 振る舞い
	Behavior behavior_ = Behavior::kSpred;

	Behavior behaviorRequest_ = Behavior::kUnknown;

	ObjectColor objectColor_;


};

class HitEffectStateSpred : public IHitEffectState 
{
public:
	void Initialize(HitEffect* hitEffect) override;
	void Update(HitEffect* hitEffect) override;
	void Draw(HitEffect* hitEffect) override;
	void Shutdown(HitEffect* hitEffect) override;

private:
	float animationTimer_ = 0.0f;

	float spredAnimation_ = 0.2f;

};

class HitEffectStateFade : public IHitEffectState
{
public:
	void Initialize(HitEffect* hitEffect) override;
	void Update(HitEffect* hitEffect) override;
	void Draw(HitEffect* hitEffect) override;
	void Shutdown(HitEffect* hitEffect) override;

private:
	float animationTimer_ = 0.0f;

	float fadeAnimation_ = 0.2f;
};