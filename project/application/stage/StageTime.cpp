#include "StageTime.h"

StageTime* StageTime::instance_ = nullptr;

StageTime* StageTime::GetInstance() {
	if (instance_ == nullptr) {
		instance_ = new StageTime;
	}
	return instance_;
}

void StageTime::Finalize() {
	for (auto numberSprite : numberTimeSprites_) 
	{
		delete numberSprite;
	}
	numberTimeSprites_.clear();
	
	for (auto numberSprite : numberScoreSprites_) {
		delete numberSprite;
	}
	numberScoreSprites_.clear();

	delete TimeSprite_;

	delete ScoreSprite_;

	delete instance_;
	instance_ = nullptr;
}

void StageTime::Initialize()
{
	TimeSprite_ = Sprite::Create(timeHandle, {0.0f, 0.0f});
	ScoreSprite_ = Sprite::Create(scoreHandle, {0.0f, 0.0f});

	time_ = kMaxTime;

	numberTimeSprites_.clear();
	for (int i = 0; i < 3; ++i) {
		Sprite* numberSprite = Sprite::Create(numberHandle, {0.0f, 0.0f});
		numberTimeSprites_.push_back(numberSprite);
	}

	numberScoreSprites_.clear();
	for (int i = 0; i < 7; ++i) {
		Sprite* numberSprite = Sprite::Create(numberHandle, {0.0f, 0.0f});
		numberScoreSprites_.push_back(numberSprite);
	}
}

void StageTime::Update()
{
	time_ -= kDeltaTime;

	SetTimeDigit();
	SetScpreDigit();
}

void StageTime::Draw()
{ 
	spriteDraw_->Draw(TimeSprite_, 
		SRT2D{{1.0f, 1.0f}, 0.0f, {50.0f, 100.0f}}, {0.0f, 0.0f}, {100.0f, 28.0f});

	for (int i = 0; i < 3; i++)
	{
		spriteDraw_->DrawSpriteSheet(numberTimeSprites_[i], 
			SRT2D{{1.0f, 1.0f}, 0.0f, {150.0f + (i * 20.0f), 100.0f}}, 
			{20.0f, 28.0f}, {20.0f * static_cast<float>(timeDigits_[i]) , 0.0f}, {20.0f, 28.0f});
	}

	spriteDraw_->Draw(ScoreSprite_,
		SRT2D{{1.0f, 1.0f}, 0.0f, {50.0f, 150.0f}}, {0.0f, 0.0f}, {120.0f, 28.0f});

	for (int i = 0; i < 7; i++)
	{
		spriteDraw_->DrawSpriteSheet(numberScoreSprites_[i], 
			SRT2D{{1.0f, 1.0f}, 0.0f, {170.0f + (i * 20.0f), 150.0f}}, 
			{20.0f, 28.0f}, {20.0f * static_cast<float>(scoreDigits_[i]) , 0.0f}, {20.0f, 28.0f});
	}
}

void StageTime::SetTimeDigit() 
{ 
	// 1. 整数部を取り出す
	long long ip = static_cast<long long>(std::fabs(time_)); 
	
	// 2. 文字列化する
	std::string s = std::to_string(ip);

	// 3. 下位3桁のみ仕様
	if (s.size() > 3)
	{
		s = s.substr(s.size() - 3);
	}

	// 4. 右詰めで格納
	const size_t start = 3 - s.size();
	for (size_t i = 0; i < s.size(); ++i)
	{
		timeDigits_[start + i] = s[i] - '0';
	}
}

void StageTime::AddScore() 
{
	score_ += 1000.0f;
}

void StageTime::SetScpreDigit() 
{
	// 1. 整数部を取り出す
	long long ip = static_cast<long long>(std::fabs(score_)); 
	
	// 2. 文字列化する
	std::string s = std::to_string(ip);

	// 3. 下位7桁のみ仕様
	if (s.size() > 7)
	{
		s = s.substr(s.size() - 7);
	}

	// 4. 右詰めで格納
	const size_t start = 7 - s.size();
	for (size_t i = 0; i < s.size(); ++i)
	{
		scoreDigits_[start + i] = s[i] - '0';
	}
}

void StageTime::ResultDraw() 
{

	spriteDraw_->Draw(ScoreSprite_,
		SRT2D{{2.0f, 2.0f}, 0.0f, {400.0f, 400.0f}}, {0.0f, 0.0f}, {120.0f, 28.0f});

	for (int i = 0; i < 7; i++)
	{
		spriteDraw_->DrawSpriteSheet(numberScoreSprites_[i], 
			SRT2D{{2.0f, 2.0f}, 0.0f, {640.0f + (i * 40.0f), 400.0f}}, 
			{20.0f, 28.0f}, {20.0f * static_cast<float>(scoreDigits_[i]) , 0.0f}, {20.0f, 28.0f});
	}
}
