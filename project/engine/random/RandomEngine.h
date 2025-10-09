#pragma once
#include <random>

class RandomEngine 
{
public:
	/// <summary>
	/// 初期化
	/// </summary>
	static void Initialize();
	/// <summary>
	/// 乱数を生成
	/// </summary>
	/// <param name="min">最小値</param>
	/// <param name="max">最大値</param>
	/// <returns>生成された乱数</returns>
	static int GenerateInt(int min, int max);
	
	/// <summary>
	/// 浮動小数点数の乱数を生成
	/// </summary>
	/// <param name="min">最小値</param>
	/// <param name="max">最大値</param>
	/// <returns>生成された乱数 (浮動小数点数)</returns>
	static float GenerateFloat(float min, float max);

private:

	/// <summary>
	/// メルセンヌ・ツイスターエンジン
	/// </summary>
	static std::mt19937_64 randomEngine_;
	


};
