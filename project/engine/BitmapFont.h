#pragma once
#include "KamataEngine.h"
#include <memory>
#include <string>
#include <unordered_map>
#include "myMath.h"

using namespace KamataEngine;

class BitmapFont 
{
public:
	

	static BitmapFont& GetInstance() 
	{
		static BitmapFont instance;
		return instance;
	}

	bool Load(const std::string& fntPath, const std::string& textureName);
	void DrawText(const std::string& text, const Vector2& position, float size);

private:
	BitmapFont() = default;
	~BitmapFont();
	BitmapFont(const BitmapFont&) = delete;
	BitmapFont& operator=(const BitmapFont&) = delete;

	struct Glyph 
	{
		Vector2 texBase; // テクスチャ上の左上座標
		Vector2 texSize; // 描画サイズ（ピクセル）
		Vector2 offset;  // 描画位置調整
		float xAdvance;  // 次の文字までの移動距離
	};

	std::unordered_map<char, Glyph> glyphs_;
	uint32_t textureHandle_ = 0;

	 float lineHeight_ = 16.0f;
};
