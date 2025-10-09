#pragma once
#include "KamataEngine.h"
#include "myMath.h"

class Anchor 
{
public:

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name = "model">モデル</param>
	/// <param name = "textureHandle">テクスチャハンドル</param>
	void Initialize(Model* model, Camera* camera, const Vector3& position, const Vector3& target);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	bool GetActive() const { return isActive_; }
	void Deactivate() { isActive_ = false; }

	const Vector3& GetTranslation() const { return worldTransform_.translation_; }
	const Vector3& GetDirection() const { return direction_; }
	float GetDistanceTraveled() const { return traveled_; }

private:

	// ワールド変換データ
	WorldTransform worldTransform_;

	// モデル
	Model* model_ = nullptr;

	// カメラ
	Camera* camera_ = nullptr;

	// 速度
	Vector3 velocity_ = {};

	Vector3 direction_{};
	float speed_ = 1.0f;
	float traveled_ = 0.0f;
	bool isActive_ = false;

};
