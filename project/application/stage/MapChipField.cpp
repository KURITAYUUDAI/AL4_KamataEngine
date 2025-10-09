#include "MapChipField.h"
#include <fstream>
#include <sstream>

void MapChipField::ResetMapChipData() 
{
	// マップチップデータをリセット
	mapChipData_.date.clear();
	mapChipData_.date.resize(kNumBlockVertical);
	for (std::vector<MapChipType>& mapChipDataLine : mapChipData_.date) {
		mapChipDataLine.resize(kNumBlockHorizontal);
	}
}

void MapChipField::LoadMapChipCsv(const std::string& filePath)
{
	// マップチップデータをリセット
	ResetMapChipData();

	// ファイルを開く
	std::ifstream file;
	file.open(filePath);
	assert(file.is_open());

	// マップチップcsv
	std::stringstream mapChipCsv;
	// ファイルの内容を文字列ストリームにコピー
	mapChipCsv << file.rdbuf();
	file.close();

	// CSVからマップチップデータを読み込む
	for (uint32_t i = 0; i < kNumBlockVertical; ++i)
	{
		std::string line;
		// 1行ずつ読み込む
		if (std::getline(mapChipCsv, line)) 
		{
			// 先頭にBOM (0xEF,0xBB,0xBF) があれば削除
			if (line.size() >= 3 
				&& static_cast<unsigned char>(line[0]) == 0xEF 
				&& static_cast<unsigned char>(line[1]) == 0xBB 
				&& static_cast<unsigned char>(line[2]) == 0xBF) 
			{
				line.erase(0, 3);
			}
		}


		// 1行分の文字列をストリームに変換して解析しやすくなる
		std::istringstream line_stream(line);

		for (uint32_t j = 0; j < kNumBlockHorizontal; ++j)
		{
			std::string word;
			getline(line_stream, word, ',');

			if (mapChipTable.contains(word)) 
			{
				mapChipData_.date[i][j] = mapChipTable[word];
			}
		}
	}
}

MapChipType MapChipField::GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex) 
{
	// インデックスが範囲内かチェック
	if (xIndex < 0 || kNumBlockHorizontal -1 < xIndex) {
		return MapChipType::kBlank; // 範囲外の場合は空白を返す
	}
	if (yIndex < 0 || kNumBlockVertical - 1 < yIndex) {
		return MapChipType::kBlank; // 範囲外の場合は空白を返す
	}

	return mapChipData_.date[yIndex][xIndex];
}

Vector3 MapChipField::GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex) 
{
	return Vector3(kBlockWidth * xIndex, kBlockHeight * (kNumBlockVertical - 1 - yIndex), 0);
}

MapChipField::IndexSet MapChipField::GetMapChipIndexSetByPosition(const Vector3& position)
{ 
	IndexSet indexSet = {};
	indexSet.xIndex = static_cast<uint32_t>((position.x + kBlockWidth / 2) / kBlockWidth); 
	indexSet.yIndex = kNumBlockVertical - 1 - static_cast<uint32_t>((position.y + kBlockHeight / 2) / kBlockHeight);
	return indexSet;
}

MapChipField::Rect MapChipField::GetRectByIndex (uint32_t xIndex, uint32_t yIndex) 
{
	// 指定ブロックの中心座標を取得する
	Vector3 center = GetMapChipPositionByIndex(xIndex, yIndex);

	Rect rect;
	rect.left = center.x - kBlockWidth / 2.0f;
	rect.right = center.x + kBlockWidth / 2.0f;
	rect.bottom = center.y - kBlockHeight / 2.0f;
	rect.top = center.y + kBlockHeight / 2.0f;

	return rect;
}