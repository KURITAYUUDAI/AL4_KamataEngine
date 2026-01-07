#include "SpriteDraw.h"

SpriteDraw* SpriteDraw::GetInstance() {

	static SpriteDraw instance;
	return &instance;

}

SpriteDraw::SpriteDraw() {
	
}

SpriteDraw::~SpriteDraw() {

}

void SpriteDraw::Draw(Sprite* sprite_, const SRT2D& SRT, 
	const Vector2& anchorPoint, const Vector2& baseSize) 
{
	sprite_->SetAnchorPoint(anchorPoint);
	sprite_->SetPosition(SRT.translate);
	sprite_->SetRotation(SRT.rotate);
	sprite_->SetSize(baseSize * SRT.scale);
	sprite_->Draw();
}
