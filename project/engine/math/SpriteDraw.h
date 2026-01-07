#pragma once
#include "KamataEngine.h"
#include "myMath.h"

struct SRT2D {

	Vector2 scale;
	float rotate;
	Vector2 translate;
};

class SpriteDraw final {

private:

	// コンストラクタ
	SpriteDraw();

public:

	// デストラクタ
	~SpriteDraw();
	SpriteDraw(const SpriteDraw& obj) = delete;
	SpriteDraw& operator=(const SpriteDraw& obj) = delete;
	
	static SpriteDraw* GetInstance();

	/// <summary>
	/// スプライトを描画
	/// </summary>
	/// <param name="sprite_">スプライト</param>
	/// <param name="SRT">Vector2の拡縮倍率、floatの回転、Vector2の平行移動の構造体</param>
	/// <param name="baseSize">画像の横幅と縦幅</param>
	void Draw(Sprite* sprite_, const SRT2D& SRT, const Vector2& anchorPoint, const Vector2& baseSize);
};
