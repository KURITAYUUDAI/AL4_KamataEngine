#pragma once
#include "KamataEngine.h"
#include "myMath.h"

class EnemyBullet;

class BulletManager 
{
public:
	// シングルトンインスタンスの取得
	static BulletManager* GetInstance();
	// 終了
	void Finalize();

public:
	// 初期化
	void Initialize(Model* model, Camera* camera);

	// 更新
	void Update();

	// 描画
	void Draw();

	// 敵弾の生成
	void CreateEnemyBullet(const Vector3& position, const Vector3& velocity);

public:  // 外部入出力

	std::list<EnemyBullet*>& GetBullets() { return enemyBullets_; }

	const Matrix4x4 GetBillboardWorldMatrix(const Vector3& scale, const Vector3& translate) const;

private: // シングルトンインスタンス
	static BulletManager* instance_;

	BulletManager() = default;
	~BulletManager() = default;
	BulletManager(BulletManager&) = delete;
	BulletManager& operator=(BulletManager&) = delete;

private:

	// モデル
	Model* modelEnemyBullet_ = nullptr;

	// カメラ
	Camera* camera_ = nullptr;

	// 弾のコンテナ
	std::list<EnemyBullet*> enemyBullets_;

	// 弾の最大数
	static inline const int kMaxEnemyBullet = 100;

	// ビルボード行列
	Matrix4x4 billboardMatrix_;
};
