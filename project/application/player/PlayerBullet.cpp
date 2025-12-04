#include "PlayerBullet.h"
#include "assert.h"
#include "WorldTransformAssist.h"

void PlayerBullet::Initialize(Model* model, const Camera* camera, 
	const Vector3& position, const Vector3& velocity) 
{
	assert(model);

	model_ = model;
	// テクスチャ読み込み
	textureHandle_ = TextureManager::Load("uvChecker.png");

	camera_ = camera;

	worldTransform_.Initialize();
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};
	worldTransform_.rotation_ = {0.0f, 0.0f, 0.0f};
	worldTransform_.translation_ = position;

	velocity_ = velocity;
}

void PlayerBullet::Update() 
{ 
	worldTransform_.translation_ += velocity_;


	WorldTransformUpdate(worldTransform_);

	if (--deathTimer_ <= 0)
	{
		isDead_ = true;
	}
}

void PlayerBullet::Draw()
{
	model_->Draw(worldTransform_, *camera_, textureHandle_);
}
