#pragma once
#include "KamataEngine.h"
#include "myMath.h"
#include "SpriteDraw.h"
#include <array>

class StageTime 
{
public:

	// シングルトンインスタンスの取得
	static StageTime* GetInstance();
	// 終了
	void Finalize();

public:

	void Initialize();

	void Update();

	void Draw();

	void SetTimeDigit();

	void AddScore();

	void SetScpreDigit();

	void ResultDraw();

private: // シングルトンインスタンス
	static StageTime* instance_;

	StageTime() = default;
	~StageTime() = default;
	StageTime(StageTime&) = delete;
	StageTime& operator=(StageTime&) = delete;

	SpriteDraw* spriteDraw_ = SpriteDraw::GetInstance();

private:

	//// テクスチャを読み込む
	uint32_t timeHandle = TextureManager::Load("Sprite/Time.png");
	uint32_t scoreHandle = TextureManager::Load("Sprite/Score.png");
	uint32_t numberHandle = TextureManager::Load("SpriteSheet/number.png");

private:

	float time_;

	const float kMaxTime = 90.0f;

	std::array<int, 3> timeDigits_ {};
	
	// スプライトを生成
	Sprite* TimeSprite_ = nullptr;

	std::vector<Sprite*> numberTimeSprites_;

private:

	float score_;

	std::array<int, 7> scoreDigits_ {}; 

	Sprite* ScoreSprite_ = nullptr;

	std::vector<Sprite*> numberScoreSprites_;

};
