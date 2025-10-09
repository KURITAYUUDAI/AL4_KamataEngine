#pragma once
#include "KamataEngine.h"
#include "myMath.h"

using namespace KamataEngine;

class Player;

class Anchor;

class Wire 
{
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(Model* model, Camera* camera, Player* player, Anchor* anchor);
	/// <summary>
	/// 更新
	/// </summary>
	void Update();
	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

private:

	// ワールド変換データ
	WorldTransform worldTransform_;
	// モデル
	Model* model_ = nullptr;
	// カメラ
	Camera* camera_ = nullptr;
	// プレイヤー
	Player* player_ = nullptr;
	// アンカー
	Anchor* anchor_ = nullptr;

};
