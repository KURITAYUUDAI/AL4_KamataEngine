#pragma once
#include "KamataEngine.h"

using namespace KamataEngine;

class AimCursor 
{
public:

	void Initialize(Model* model, Camera* camera);
	void Update(const Vector3& hitPosition);
	void Draw();

private:
	WorldTransform worldTransform_;
	Model* model_ = nullptr;
	Camera* camera_ = nullptr;

};
