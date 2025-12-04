#include "WorldTransformAssist.h"

using namespace KamataEngine;

void WorldTransformUpdate(WorldTransform& worldTransform) 
{ 
	worldTransform.matWorld_ = MakeAffineMatrixB(worldTransform.scale_, worldTransform.rotation_, worldTransform.translation_); 

	worldTransform.TransferMatrix();
}
