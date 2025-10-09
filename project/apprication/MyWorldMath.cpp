#include "MyWorldMath.h"

void WorldTransformUpdate(KamataEngine::WorldTransform& worldTransform)
{
	// 行列を計算
	worldTransform.matWorld_ = MakeAffineMatrixB(worldTransform.scale_, worldTransform.rotation_, worldTransform.translation_);
	// 定数バッファに転送
	worldTransform.TransferMatrix();
}