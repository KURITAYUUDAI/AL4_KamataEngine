#define NOMINMAX
#include "GameScene.h"
#include <limits>

using namespace KamataEngine;

GameScene::~GameScene() 
{
	delete player_;

	for (auto it = enemies_.begin(); it != enemies_.end(); ++it)  // auto == std::list<Enemy*>::iterator
	{
		Enemy* enemy = *it; // イテレータが指す要素を取り出す
		delete enemy;
	}
	enemies_.clear();
	
	for (auto it = hitEffects_.begin(); it != hitEffects_.end(); ++it) // auto == std::list<HitEffect*>::iterator
	{
		HitEffect* hitEffect = *it; // イテレータが指す要素を取り出す
		delete hitEffect;
	}

	delete skydome_;

	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) 
	{
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine)
		{
			delete worldTransformBlock;
		}
	}
	worldTransformBlocks_.clear();

	delete deathParticles_;

	delete modelAimCursor_;
	delete modelAnchor_;
	delete modelWire_;
	delete modelPlayer_;
	delete modelPlayerAttack_;
	delete modelEnemy_;
	delete modelBlock_;
	delete modelSkydome_;
	delete modelDeathParticle_;
	delete modelHitEffect_;
	delete mapChipField_;
	delete modelGoal_;
	delete modelBackGround_;

	delete EX1Sprite_;
	delete EX2Sprite_;

	delete this->debugCamera_;
}

void GameScene::Initialize()
{
	// メンバ変数への代入処理

	// テクスチャの読み込み
	textureHandle_ = TextureManager::Load("block.png");

	// 3Dモデルの生成
	modelPlayer_ = Model::CreateFromOBJ("player", true);
	modelPlayerAttack_ = Model::CreateFromOBJ("playerAttack", true);
	modelAimCursor_ = Model::CreateFromOBJ("aimCursor", true);
	modelAnchor_ = Model::CreateFromOBJ("anchor", true);
	modelWire_ = Model::CreateFromOBJ("wire", true);
	modelEnemy_ = Model::CreateFromOBJ("enemy", true);
	modelBlock_ = Model::Create();
	modelSkydome_ = Model::CreateFromOBJ("skydome", true);
	modelDeathParticle_ = Model::CreateFromOBJ("deathParticles", true);
	modelHitEffect_ = Model::CreateFromOBJ("hitEffect", true);
	modelGoal_ = Model::CreateFromOBJ("goal", true);
	modelBackGround_ = Model::CreateFromOBJ("backGround", true);

	HitEffect::SetModel(modelHitEffect_);
	HitEffect::SetCamera(&camera_);

	// 天球の生成
	skydome_ = new Skydome();
	// 天球の初期化
	skydome_->Initialize(modelSkydome_, &camera_);

	// マップチップフィールドの生成
	mapChipField_ = new MapChipField();
	// マップチップフィールドの初期化
	mapChipField_->LoadMapChipCsv("Resources/mapChip/blocks.csv");
	
	// 背景の生成
	backGround_ = new BackGround();

	// 座標をマップチップ番号で指定
	Vector3 backGroundPosition = mapChipField_->GetMapChipPositionByIndex(7, 49);

	// 背景の初期化
	backGround_->Initialize(modelBackGround_, &camera_, backGroundPosition + Vector3{0.0f, 0.0f, 1.1f});

	// 自キャラの生成
	player_ = new Player();

	// 座標をマップチップ番号で指定
	Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(4, 98);

	// 自キャラの初期化
	player_->Initialize(modelPlayer_, modelPlayerAttack_, &camera_, playerPosition);

	player_->SetMapChipFiled(mapChipField_);

	player_->SetRayHitBlockFunction([this](const Ray& ray, Vector3& outPos) 
		{ return this->RayHitsBlock(ray, outPos); });
	
	aimCursor_ = new AimCursor();
	aimCursor_->Initialize(modelAimCursor_, &camera_);

	for (int32_t i = 0; i < enemyPop_; ++i)
	{
		// 敵Mobの生成
		Enemy* newEnemy = new Enemy();

		// 座標をマップチップ番号で指定
		Vector3 enemyPosition = mapChipField_->GetMapChipPositionByIndex(10 - i, 18 - i * 2);
		newEnemy->Initialize(modelEnemy_, &camera_, enemyPosition);

		newEnemy->SetGameScene(this); // 敵MobにGameSceneのポインタをセット

		enemies_.push_back(newEnemy);
	}

	// ゴールの生成
	goal_ = new Goal();

	goal_->SetGameScene(this); // ゴールにGameSceneのポインタをセット
	// 座標をマップチップ番号で指定
	Vector3 goalPosition = mapChipField_->GetMapChipPositionByIndex(7, 3);
	goal_->Initialize(modelGoal_, &camera_, goalPosition);

	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, 1.0f);

	// インゲームの初期化処理を書く
	GenerateBlocks();

	// カメラの初期化
	camera_.farZ = 1000.0f;
	camera_.Initialize();
	

	// デバッグカメラの生成
	debugCamera_ = new DebugCamera(1280, 720);

	// カメラコントローラーの初期化
	cameraController_ = new CameraController();
	cameraController_->Initialize();
	cameraController_->SetTarget(player_);
	cameraController_->Reset();

	cameraController_->SetMovableArea(
		mapChipField_->GetMapChipPositionByIndex(0, 0), mapChipField_->GetMapChipPositionByIndex(14, 99));

	

	phase_ = Phase::kFadeIn;

}

void GameScene::Update()
{
	// インゲームの更新処理を書く

	switch (phase_)
	{ 
	
	case Phase::kFadeIn:

		// 天球の更新
		skydome_->Update();

		// 背景の更新
		backGround_->Update();

		// 自キャラの更新
		player_->Update();

		// 敵Mobの更新
		for (auto it = enemies_.begin(); it != enemies_.end(); ++it) // auto == std::list<Enemy*>::iterator
		{
			Enemy* enemy = *it;
			enemy->Update();
		}

		// カメラの処理
		if (isDebugCameraActive_) 
		{
			debugCamera_->Update();
			camera_.matView = debugCamera_->GetCamera().matView;
			camera_.matProjection = debugCamera_->GetCamera().matProjection;
			// ビュープロジェクション行列の転送
			camera_.TransferMatrix();
		} 
		else 
		{
			cameraController_->Update();
			camera_.matView = cameraController_->GetCamera().matView;
			camera_.matProjection = cameraController_->GetCamera().matProjection;
			camera_.TransferMatrix();
		}

		// ブロックの更新
		for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) 
		{
			for (WorldTransform* worldTransformBlock : worldTransformBlockLine) 
			{
				if (!worldTransformBlock)
					continue;

				Matrix4x4 mat = worldTransformBlock->matWorld_;

				worldTransformBlock->matWorld_ = MakeAffineMatrixB(worldTransformBlock->scale_, worldTransformBlock->rotation_, worldTransformBlock->translation_);

				// 定数バッファに転送
				worldTransformBlock->TransferMatrix();
			}
		}

		// ゴールの更新
		goal_->Update();

		fade_->Update();

		if (fade_->IsFinished()) 
		{
			phase_ = Phase::kPlay;
		}

		break;

	case Phase::kPlay:

		// 天球の更新
		skydome_->Update();

		// 背景の更新
		backGround_->Update();
		
		Vector2 mousePos = Input::GetInstance()->GetMousePosition(); // 画面座標
		Ray ray = ConvertScreenToWorldRay(mousePos, camera_);

		Vector3 hitPos;
		if (RayHitsBlock(ray, hitPos)) 
		{
			isAimCursorActive_ = true;
			aimCursor_->Update(hitPos + Vector3{0.0f, 0.0f, -1.01f});

			if (Input::GetInstance()->IsTriggerMouse(0)) 
			{
				if (!anchor_)
				{
					anchor_ = new Anchor();
					anchor_->Initialize(modelAnchor_, &camera_, player_->GetWorldPosition(), hitPos);
					wire_ = new Wire();
					wire_->Initialize(modelWire_, &camera_, player_, anchor_);
					player_->ShotGrapple();
				}
			}
		} 
		else
		{
			isAimCursorActive_ = false;
		}
		
		if (anchor_) 
		{
			anchor_->Update();

			if (anchor_->GetActive())
			{
				// アンカーの当たり判定用AABB（小さめの立方体）
				AABB anchorAABB{
				    .min = anchor_->GetTranslation() - Vector3{0.1f, 0.1f, 0.1f},
				    .max = anchor_->GetTranslation() + Vector3{0.1f, 0.1f, 0.1f},
				};

				// 全ブロックとの衝突チェック
				for (uint32_t y = 0; y < worldTransformBlocks_.size(); ++y) {
					for (uint32_t x = 0; x < worldTransformBlocks_[y].size(); ++x) {
						auto* block = worldTransformBlocks_[y][x];
						if (!block)
							continue;

						Vector3 center = block->translation_;
						AABB blockAABB{
						    .min = center - Vector3{1.0f, 1.0f, 1.0f},
						    .max = center + Vector3{1.0f, 1.0f, 1.0f},
						};

						if (IsCollision(anchorAABB, blockAABB)) {
							Vector3 hitPosAnchor = center;
							anchor_->Deactivate();
							player_->MoveGrapple(hitPosAnchor);
							return;
						}
					}
				}
			}

			wire_->Update();
		}


		// 自キャラの更新
		player_->Update();

		if (player_->GetEndGrappling()) 
		{
			if (anchor_) {
				delete anchor_;
				anchor_ = nullptr;
				delete wire_;
				wire_ = nullptr;
			}
		}

		if (anchor_)
		{
			if (player_->GetGrappleBehavior() == Player::GrappleBehavior::kShot && !anchor_->GetActive()) {
				player_->EndGrapple();
			}
		}
		

		// 敵Mobの更新
		for (auto it = enemies_.begin(); it != enemies_.end(); ++it) // auto == std::list<Enemy*>::iterator
		{
			Enemy* enemy = *it;
			enemy->Update();
		}

		// カメラの処理
		if (isDebugCameraActive_) {
			debugCamera_->Update();
			camera_.matView = debugCamera_->GetCamera().matView;
			camera_.matProjection = debugCamera_->GetCamera().matProjection;
			// ビュープロジェクション行列の転送
			camera_.TransferMatrix();
		} else {
			cameraController_->Update();
			camera_.matView = cameraController_->GetCamera().matView;
			camera_.matProjection = cameraController_->GetCamera().matProjection;
			camera_.TransferMatrix();
		}

		// ブロックの更新
		for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
			for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
				if (!worldTransformBlock)
					continue;

				Matrix4x4 mat = worldTransformBlock->matWorld_;

				worldTransformBlock->matWorld_ = MakeAffineMatrixB(worldTransformBlock->scale_, worldTransformBlock->rotation_, worldTransformBlock->translation_);

				// 定数バッファに転送
				worldTransformBlock->TransferMatrix();
			}
		}

		// ゴールの更新
		goal_->Update();

		// 衝突判定
		CheckAllCollisions();

		// ヒットエフェクトの更新
		for (auto it = hitEffects_.begin(); it != hitEffects_.end(); ++it) // auto == std::list<HitEffect*>::iterator
		{
			HitEffect* hitEffect = *it;
			hitEffect->Update();
		}

		// ゴールの更新
		goal_->Update();

		if (player_->GetIsDead() || Input::GetInstance()->PushKey(DIK_F2)) {
			// 死亡演出フェーズに切り換え
			phase_ = Phase::kDeath;
			// 自キャラの座標を取得
			const Vector3& deathParticlesPosition = player_->GetWorldPosition();

			// 死亡パーティクルを生成、初期化
			deathParticles_ = new DeathParticles;
			deathParticles_->Initialize(modelDeathParticle_, &camera_, deathParticlesPosition);
		}

		if (goal_->GetIsGoal())
		{
			// ゴール演出フェーズに切り換え
			phase_ = Phase::kFadeOut;
			fade_->Start(Fade::Status::FadeOut, 1.0f);
		}

		enemies_.remove_if([this](Enemy* enemy) 
		{
			if (enemy->GetIsDead() == true)
			{
				delete enemy;
				return true;
			}
			
			return false; 
		});

		hitEffects_.remove_if([](HitEffect* hitEffect) {
			if (hitEffect->GetIsDead() == true) {
				delete hitEffect;
				return true;
			}

			return false;
		});

		break;

	case Phase::kDeath:

		// 天球の更新
		skydome_->Update();

		// 背景の更新
		backGround_->Update();


		// 敵Mobの更新
		for (auto it = enemies_.begin(); it != enemies_.end(); ++it) // auto == std::list<Enemy*>::iterator
		{
			Enemy* enemy = *it;
			enemy->Update();
		}

		// 死亡パーティクルの更新
		deathParticles_->Update();

		// カメラの処理
		if (isDebugCameraActive_) {
			debugCamera_->Update();
			camera_.matView = debugCamera_->GetCamera().matView;
			camera_.matProjection = debugCamera_->GetCamera().matProjection;
			// ビュープロジェクション行列の転送
			camera_.TransferMatrix();
		}

		// ブロックの更新
		for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
			for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
				if (!worldTransformBlock)
					continue;

				Matrix4x4 mat = worldTransformBlock->matWorld_;

				worldTransformBlock->matWorld_ = MakeAffineMatrixB(worldTransformBlock->scale_, worldTransformBlock->rotation_, worldTransformBlock->translation_);

				// 定数バッファに転送
				worldTransformBlock->TransferMatrix();
			}
		}

		// ゴールの更新
		goal_->Update();

		if (deathParticles_ && deathParticles_->GetIsFinished()) 
		{
			phase_ = Phase::kFadeOut;
			fade_->Start(Fade::Status::FadeOut, 1.0f);
		}

		break;

	case Phase::kFadeOut:

		// 天球の更新
		skydome_->Update();

		// 背景の更新
		backGround_->Update();


		// 敵Mobの更新
		for (auto it = enemies_.begin(); it != enemies_.end(); ++it) // auto == std::list<Enemy*>::iterator
		{
			Enemy* enemy = *it;
			enemy->Update();
		}

		// カメラの処理
		if (isDebugCameraActive_) {
			debugCamera_->Update();
			camera_.matView = debugCamera_->GetCamera().matView;
			camera_.matProjection = debugCamera_->GetCamera().matProjection;
			// ビュープロジェクション行列の転送
			camera_.TransferMatrix();
		}

		// ブロックの更新
		for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
			for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
				if (!worldTransformBlock)
					continue;

				Matrix4x4 mat = worldTransformBlock->matWorld_;

				worldTransformBlock->matWorld_ = MakeAffineMatrixB(worldTransformBlock->scale_, worldTransformBlock->rotation_, worldTransformBlock->translation_);

				// 定数バッファに転送
				worldTransformBlock->TransferMatrix();
			}
		}

		if (player_)
		{
			// 自キャラの更新
			player_->Update();
		}

		// ゴールの更新
		goal_->Update();

	    fade_->Update();

		if (fade_->IsFinished())
		{
			finished_ = true;
		}

		break;

	}


	
	

#ifdef _DEBUG
	if (Input::GetInstance()->TriggerKey(DIK_F1)) {
		isDebugCameraActive_ = !isDebugCameraActive_;
	}
#endif

	
}

void GameScene::Draw()
{
	// DirectXCommonインスタンスの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// 3Dモデル描画前処理
	Model::PreDraw(dxCommon->GetCommandList());


	switch (phase_) 
	{

	case Phase::kFadeIn:
		
		// 天球の描画
		skydome_->Draw();

		// 背景の描画
		backGround_->Draw();

		// ブロックの描画
		for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) 
		{
			for (WorldTransform* worldTransformBlock : worldTransformBlockLine) 
			{
				if (!worldTransformBlock)
					continue;

				Matrix4x4 mat = worldTransformBlock->matWorld_;

				modelBlock_->Draw(*worldTransformBlock, camera_, textureHandle_);
			}
		}

		// 敵Mobの描画
		for (auto it = enemies_.begin(); it != enemies_.end(); ++it) // auto == std::list<Enemy*>::iterator
		{
			Enemy* enemy = *it;
			enemy->Draw();
		}

		// ゴールの描画
		goal_->Draw();	

		// 自キャラの描画
		player_->Draw();

		Sprite::PreDraw(dxCommon->GetCommandList());

		EX1Sprite_->Draw();
		EX2Sprite_->Draw();

		Sprite::PostDraw();

		fade_->Draw();

		break;

	case Phase::kPlay:

		// 天球の描画
		skydome_->Draw();

		// 背景の描画
		backGround_->Draw();

		// ブロックの描画
		for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) 
		{
			for (WorldTransform* worldTransformBlock : worldTransformBlockLine) 
			{
				if (!worldTransformBlock)
					continue;

				Matrix4x4 mat = worldTransformBlock->matWorld_;

				modelBlock_->Draw(*worldTransformBlock, camera_, textureHandle_);
			}
		}

		if (wire_)
		{
			wire_->Draw();
		}

		if (anchor_) {
			anchor_->Draw();
		}

		// ゴールの描画
		goal_->Draw();	

		// 自キャラの描画
		player_->Draw();

		if (isAimCursorActive_) {
			aimCursor_->Draw();
		}

		// 敵Mobの描画
		for (auto it = enemies_.begin(); it != enemies_.end(); ++it) // auto == std::list<Enemy*>::iterator
		{
			Enemy* enemy = *it;
			enemy->Draw();
		}

		// ヒットエフェクトの描画
		for (auto it = hitEffects_.begin(); it != hitEffects_.end(); ++it) // auto == std::list<HitEffect*>::iterator
		{
			HitEffect* hitEffect = *it;
			hitEffect->Draw();
		}

		Sprite::PreDraw(dxCommon->GetCommandList());

		EX1Sprite_->Draw();
		EX2Sprite_->Draw();

		Sprite::PostDraw();

		break;

	case Phase::kDeath:

		// 天球の描画
		skydome_->Draw();

		// 背景の描画
		backGround_->Draw();

		// ブロックの描画
		for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
			for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
				if (!worldTransformBlock)
					continue;

				Matrix4x4 mat = worldTransformBlock->matWorld_;

				modelBlock_->Draw(*worldTransformBlock, camera_, textureHandle_);
			}
		}

		// 敵Mobの描画
		for (auto it = enemies_.begin(); it != enemies_.end(); ++it) // auto == std::list<Enemy*>::iterator
		{
			Enemy* enemy = *it;
			enemy->Draw();
		}

		// ゴールの描画
		goal_->Draw();	

		if (deathParticles_)
		{
			// 死亡エフェクトの描画
			deathParticles_->Draw();
		}

		

		Sprite::PreDraw(dxCommon->GetCommandList());

		EX1Sprite_->Draw();
		EX2Sprite_->Draw();

		Sprite::PostDraw();

		break;

	case Phase::kFadeOut:

		// 天球の描画
		skydome_->Draw();

		// 背景の描画
		backGround_->Draw();

		// ブロックの描画
		for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
			for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
				if (!worldTransformBlock)
					continue;

				Matrix4x4 mat = worldTransformBlock->matWorld_;

				modelBlock_->Draw(*worldTransformBlock, camera_, textureHandle_);
			}
		}

		// 敵Mobの描画
		for (auto it = enemies_.begin(); it != enemies_.end(); ++it) // auto == std::list<Enemy*>::iterator
		{
			Enemy* enemy = *it;
			enemy->Draw();
		}

		// ゴールの描画
		goal_->Draw();	

		if (deathParticles_)
		{
			// 死亡エフェクトの描画
			deathParticles_->Draw();
		}

		if (player_) 
		{
			// 自キャラの描画
			player_->Draw();
		}

		Sprite::PreDraw(dxCommon->GetCommandList());

		EX1Sprite_->Draw();
		EX2Sprite_->Draw();

		Sprite::PostDraw();

		fade_->Draw();

		break;

	}

	

	// スプライトの描画処理

	
	// 3Dモデル描画後処理
	Model::PostDraw();

}

void GameScene::GenerateBlocks()
{
	// 要素数
	uint32_t numBlockVertical = mapChipField_->GetNumBlockVertical();
	uint32_t numBlockHorizontal = mapChipField_->GetNumBlockHorizontal();

	// 要素数を変更する
	// 列数を設定（縦方向のブロック数）
	worldTransformBlocks_.resize(numBlockVertical);
	for (uint32_t i = 0; i < numBlockVertical; ++i) 
	{
		worldTransformBlocks_[i].resize(numBlockHorizontal);
	}

	// ブロックの生成
	for (uint32_t i = 0; i < numBlockVertical; ++i) 
	{
		for (uint32_t j = 0; j < numBlockHorizontal; ++j) 
		{

			if (mapChipField_->GetMapChipTypeByIndex(j, i) == MapChipType::kBlock) 
			{
				WorldTransform* worldTransform = new WorldTransform();
				worldTransform->Initialize();
				worldTransformBlocks_[i][j] = worldTransform;
				worldTransformBlocks_[i][j]->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);
			}
		}
	}
}

void GameScene::CheckAllCollisions()
{
	#pragma region 自キャラと敵Mobの当たり判定
	{
		// 判定対象1と2の距離
		AABB aabb1, aabb2;

		// 自キャラの座標
		aabb1 = player_->GetAABB();

		// 自キャラと敵Mob全ての当たり判定
		for (Enemy* enemy : enemies_)
		{
			if (enemy->IsCollisionDisabled())
			{
				// 衝突無効フラグが立っている場合はスキップ
				continue;
			}

			// 敵Mobの座標
			aabb2 = enemy->GetAABB();

			// AABB同士の交差判定
			if (IsCollision(aabb1, aabb2))
			{
				// 自キャラの衝突時間数を呼び出す
				player_->OnCollision(enemy);
				// 敵の衝突時関数を呼び出す
				enemy->OnCollision(player_);
			}
		}
	}
	#pragma endregion

	#pragma region 自キャラとゴールの当たり判定
	{
		// 判定対象1と2の距離
		AABB aabb1, aabb2;

		// 自キャラの座標
		aabb1 = player_->GetAABB();


		// 敵Mobの座標
		aabb2 = goal_->GetAABB();

		// AABB同士の交差判定
		if (IsCollision(aabb1, aabb2)) 
		{
			// 敵の衝突時関数を呼び出す
			goal_->OnCollision(player_);
		}
		
	}
	#pragma endregion
}

void GameScene::ChangePhase()
{ 
	switch (phase_) 
	{ 
		case Phase::kPlay:


			break;

		case Phase::kDeath:

			

			break;
	}
}

void GameScene::CreateHitEffect(const Vector3& position) 
{
	
	// ヒットエフェクトの生成
	HitEffect* newHitEffect = HitEffect::Create(position);

	if (hitEffects_.size() >= maxHitEffect_) 
	{
		// ヒットエフェクトの数が上限に達している場合、最初の要素を削除
		delete hitEffects_.front();
		hitEffects_.pop_front();
	}

	hitEffects_.push_back(newHitEffect);
	
}

bool GameScene::RayHitsBlock(const Ray& ray, Vector3& outPos) {
	const float expand = 0.5f; // 判定拡張量

	bool hitAny = false;
	float minDistanceSq = std::numeric_limits<float>::max();
	Vector3 closestCenter{};

	Segment seg = {ray.origin, ray.diff * 100.0f};

	for (uint32_t y = 0; y < worldTransformBlocks_.size(); ++y) {
		for (uint32_t x = 0; x < worldTransformBlocks_[y].size(); ++x) {
			auto* block = worldTransformBlocks_[y][x];
			if (!block)
				continue;

			Vector3 center = block->translation_;
			AABB aabb{
			    .min = center - Vector3{0.5f + expand, 0.5f + expand, 0.5f + expand},
			    .max = center + Vector3{0.5f + expand, 0.5f + expand, 0.5f + expand},
			};

			if (IsCollision(aabb, seg)) {
				float distSq = Length(center - ray.origin); // レイ始点との距離²

				if (distSq < minDistanceSq) {
					minDistanceSq = distSq;
					closestCenter = center;
					hitAny = true;
				}
			}
		}
	}

	if (hitAny) {
		outPos = closestCenter; // ← ずらさない
		return true;
	}

	return false;
}
