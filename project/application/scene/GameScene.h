#pragma once
#include "KamataEngine.h"
#include "myMath.h"
#include <vector>
#include "Player.h"
#include "Skydome.h"
#include "CameraController.h"
#include "DeathParticles.h"
#include "Fade.h"
#include "BackGround.h"
#include "Enemy.h"


/// <summary>
/// ゲームシーン
/// </summary>
class GameScene 
{
public:
	// ゲームのフェーズ(型)
	enum class Phase
	{
		kFadeIn,	// フェードイン
		kPlay,		// ゲームプレイ
		kDeath,		// デス演出
		kFadeOut,	// フェードアウト
	};

	// デストラクタ
	~GameScene();

	// 初期化
	void Initialize();

	// 更新
	void Update();

	// 描画
	void Draw();

	// ワールドトランスフォーム
	std::vector<std::vector<WorldTransform*>> worldTransformBlocks_;

	// カメラ
	Camera camera_;

	// デバッグカメラ有効
	bool isDebugCameraActive_ = false;

	// デバッグカメラ
	DebugCamera* debugCamera_ = nullptr;

	// 自キャラ
	Player* player_ = nullptr;

	std::list<Enemy*> enemies_;

	// 天球
	Skydome* skydome_ = nullptr;

	// 背景
	BackGround* backGround_ = nullptr;

	// カメラ操作
	CameraController* cameraController_ = nullptr;

	// 全ての当たり判定を行う
	void CheckAllCollisions();

	DeathParticles* deathParticles_ = nullptr;

	Phase phase_;

	void ChangePhase();

	// デスフラグのgetter
	bool IsFinished() const { return finished_; }

private:

	// テクスチャハンドル
	uint32_t textureHandle_ = 0;

	// 自キャラモデル
	Model* modelPlayer_ = nullptr;

	// 敵キャラモデル
	Model* modelEnemy_ = nullptr;
	
	// 天球モデル
	Model* modelSkydome_ = nullptr;

	// 背景モデル
	Model* modelBackGround_ = nullptr;

	/*int32_t maxHitEffect_ = 3;*/

	// 死亡パーティクルモデル
	Model* modelDeathParticle_ = nullptr;

	// 終了フラグ
	bool finished_ = false;

	Fade* fade_ = nullptr;

	//// テクスチャを読み込む
	//uint32_t EX1Handle = TextureManager::Load("Sprite/EX1.png");
	//uint32_t EX2Handle = TextureManager::Load("Sprite/EX2.png");
	//uint32_t BGHandle = TextureManager::Load("backGround/backGround.png");

	//// スプライトを生成
	//Sprite* EX1Sprite_ = Sprite::Create(EX1Handle, {0.0f, 0.0f});
	//Sprite* EX2Sprite_ = Sprite::Create(EX2Handle, {1090.0f, 0.0f});
	//Sprite* BGSprite_ = Sprite::Create(BGHandle, {0.0f, 0.0f});
};