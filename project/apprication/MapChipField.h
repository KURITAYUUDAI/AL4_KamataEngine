#pragma once
#include "KamataEngine.h"
#include "myMath.h"
#include <map>

using namespace KamataEngine;

enum class MapChipType
{
	kBlank, // 空白
	kBlock, // ブロック
};

struct MapChipData
{
	std::vector<std::vector<MapChipType>> date;
};



namespace 
{
	std::map<std::string, MapChipType> mapChipTable = 
	{
		{"0", MapChipType::kBlank},
		{"1", MapChipType::kBlock},
	};
}

class MapChipField 
{
public:

	struct IndexSet 
	{
		uint32_t xIndex;
		uint32_t yIndex;
	};

	struct Rect 
	{
		float left = 0.0f;   // 左端
		float right = 1.0f;  // 右端
		float bottom = 0.0f; // 下端
		float top = 1.0f;    // 上端
	};

	void ResetMapChipData();
	void LoadMapChipCsv(const std::string& filePath);
	MapChipType GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex);
	Vector3 GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex);

	uint32_t GetNumBlockVertical() const { return kNumBlockVertical; }
	uint32_t GetNumBlockHorizontal() const { return kNumBlockHorizontal; }

	IndexSet GetMapChipIndexSetByPosition(const Vector3& position);

	Rect GetRectByIndex(uint32_t xIndex, uint32_t yIndex);


	// 1ブロックのサイズ
	static inline const float kBlockWidth = 2.0f;
	static inline const float kBlockHeight = 2.0f;	

	// ブロックの個数
	static inline const uint32_t kNumBlockVertical = 100;
	static inline const uint32_t kNumBlockHorizontal = 15;

	MapChipData mapChipData_;
};
