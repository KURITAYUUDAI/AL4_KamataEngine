#include "BulletManager.h"
#include "EnemyBullet.h"

BulletManager* BulletManager::instance_ = nullptr;

BulletManager* BulletManager::GetInstance() 
{
	if (instance_ == nullptr) 
	{
		instance_ = new BulletManager;
	}
	return instance_;
}

void BulletManager::Finalize() 
{
	for (auto bullet : enemyBullets_) 
	{
		delete bullet;
	}
	enemyBullets_.clear();
	delete instance_;
	instance_ = nullptr;
}

void BulletManager::Initialize(Model* model, Camera* camera) 
{
	modelEnemyBullet_ = model;
	camera_ = camera;
}

void BulletManager::Update() 
{
	Matrix4x4 backToFrontMatrix = MakeRotateYMatrix(pi);
	billboardMatrix_ = Multiply(backToFrontMatrix, Inverse(camera_->matView));
	billboardMatrix_.m[3][0] = 0.0f;
	billboardMatrix_.m[3][1] = 0.0f;
	billboardMatrix_.m[3][2] = 0.0f;

	for (auto it = enemyBullets_.begin(); it != enemyBullets_.end();) 
	{
		EnemyBullet* bullet = *it;
		bullet->Update();
		if (bullet->GetIsDead()) 
		{
			delete bullet;
			it = enemyBullets_.erase(it);
		}
		else 
		{
			++it;
		}
	}
}

void BulletManager::Draw() 
{
	for (auto& bullet : enemyBullets_) 
	{
		bullet->Draw();
	}
}

void BulletManager::CreateEnemyBullet(const Vector3& position, const Vector3& velocity) 
{
	if (enemyBullets_.size() >= kMaxEnemyBullet) 
	{
		return;
	}
	EnemyBullet* bullet = new EnemyBullet;
	bullet->Initialize(modelEnemyBullet_, camera_, position, velocity);
	enemyBullets_.push_back(bullet);
}

const Matrix4x4 BulletManager::GetBillboardWorldMatrix(const Vector3& scale, const Vector3& translate) const
{
	Matrix4x4 scaleMatrix = MakeScaleMatrix(scale);
	Matrix4x4 translateMatrix = MakeTranslateMatrix(translate);

	Matrix4x4 worldMatrix = scaleMatrix * billboardMatrix_ * translateMatrix;

	return worldMatrix;
}
