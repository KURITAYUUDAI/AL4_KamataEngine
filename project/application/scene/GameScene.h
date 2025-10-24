#pragma once
#include "KamataEngine.h"
#include "myMath.h"
#include <vector>
#include "Player.h"
#include "Enemy.h"
#include "Skydome.h"
#include "MapChipField.h"
#include "CameraController.h"
#include "DeathParticles.h"
#include "Fade.h"
#include "HitEffect.h"
#include "AimCursor.h"
#include "Anchor.h"
#include "Wire.h"
#include "Goal.h"
#include "BackGround.h"


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

	// マップブロック生成
	void GenerateBlocks();

	// ワールドトランスフォーム
	std::vector<std::vector<WorldTransform*>> worldTransformBlocks_;

	// カメラ
	Camera camera_;

	// マウスカーソル
	AimCursor* aimCursor_ = nullptr;

	bool isAimCursorActive_ = true;

	// アンカー
	Anchor* anchor_ = nullptr;

	// ワイヤー
	Wire* wire_ = nullptr;

	// デバッグカメラ有効
	bool isDebugCameraActive_ = false;

	// デバッグカメラ
	DebugCamera* debugCamera_ = nullptr;

	// 自キャラ
	Player* player_ = nullptr;

	// 敵Mob
	std::list<Enemy*> enemies_;

	// ヒットエフェクト
	std::list<HitEffect*> hitEffects_;

	// ゴール
	Goal* goal_ = nullptr;

	void CreateHitEffect(const Vector3& position);

	// 天球
	Skydome* skydome_ = nullptr;

	// 背景
	BackGround* backGround_ = nullptr;

	// マップチップフィールド
	MapChipField* mapChipField_ = nullptr;

	// カメラ操作
	CameraController* cameraController_ = nullptr;

	// 全ての当たり判定を行う
	void CheckAllCollisions();

	DeathParticles* deathParticles_ = nullptr;

	Phase phase_;

	void ChangePhase();

	// デスフラグのgetter
	bool IsFinished() const { return finished_; }

	bool RayHitsBlock(const Ray& ray, Vector3& outPos);

private:

	// テクスチャハンドル
	uint32_t textureHandle_ = 0;

	// 自キャラモデル
	Model* modelPlayer_ = nullptr;

	// 自キャラ攻撃用モデル
	Model* modelPlayerAttack_ = nullptr;

	// マウスカーソルモデル
	Model* modelAimCursor_ = nullptr;

	// アンカーモデル
	Model* modelAnchor_ = nullptr;

	// ワイヤーモデル
	Model* modelWire_ = nullptr;

	// 敵Mobモデル
	Model* modelEnemy_ = nullptr;

	// ブロックモデル
	Model* modelBlock_ = nullptr;

	// 天球モデル
	Model* modelSkydome_ = nullptr;

	// 背景モデル
	Model* modelBackGround_ = nullptr;

	// ヒットエフェクトモデル
	Model* modelHitEffect_ = nullptr;

	// ゴールモデル
	Model* modelGoal_ = nullptr;

	int32_t enemyPop_ = 3;

	int32_t maxHitEffect_ = 3;

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