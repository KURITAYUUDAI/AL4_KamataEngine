#include "RandomEngine.h"

// ここで static メンバを定義（リンク対象として実体を用意）
std::mt19937_64 RandomEngine::randomEngine_;

void RandomEngine::Initialize() 
{
	// 乱数生成エンジン
	std::random_device seedGenerator; // シード生成器
	// メルセンヌ・ツイスターエンジンの初期化
	randomEngine_.seed(seedGenerator()); // シード値を設定
}

int RandomEngine::GenerateInt(int min, int max) 
{

	// 範囲が逆転していたら交換
	if (min > max)
		std::swap(min, max);

	// 一様分布で乱数生成
	std::uniform_int_distribution<int> dist(min, max);
	return dist(randomEngine_);
}

float RandomEngine::GenerateFloat(float min, float max) 
{
	// 範囲が逆転していたら交換
	if (min > max)
		std::swap(min, max);

	// 浮動小数点の一様分布で乱数生成
	std::uniform_real_distribution<float> dist(min, max);
	return dist(randomEngine_);
}
