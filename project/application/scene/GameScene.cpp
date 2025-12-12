#define NOMINMAX
#include "GameScene.h"
#include <limits>
#include "3d/AxisIndicator.h"

#include "PlayerBullet.h"
#include "SeedManager.h"

using namespace KamataEngine;

GameScene::~GameScene() 
{
	delete player_;

	for (Enemy* enemy : enemies_)
	{
		delete enemy;
	}
	enemies_.clear();

	delete skydome_;

	
	delete deathParticles_;
	
	delete modelPlayer_;
	
	delete modelSkydome_;
	delete modelDeathParticle_;
	delete modelBackGround_;

	/*delete EX1Sprite_;
	delete EX2Sprite_;*/

	delete this->debugCamera_;
}

void GameScene::Initialize()
{
	// メンバ変数への代入処理

	// テクスチャの読み込み
	textureHandle_ = TextureManager::Load("block.png");

	// 3Dモデルの生成
	modelPlayer_ = Model::CreateFromOBJ("player", true);

	modelEnemy_ = Model::CreateFromOBJ("enemy", true);
	
	modelSkydome_ = Model::CreateFromOBJ("skydome", true);
	modelDeathParticle_ = Model::CreateFromOBJ("deathParticles", true);

	modelBackGround_ = Model::CreateFromOBJ("backGround", true);

	// 天球の生成
	skydome_ = new Skydome();
	// 天球の初期化
	skydome_->Initialize(modelSkydome_, &camera_);

	

	// 自キャラの生成
	player_ = new Player();

	// 座標をマップチップ番号で指定
	Vector3 playerPosition = { 0.0f, 0.0f,0.0f };

	// 自キャラの初期化
	player_->Initialize(modelPlayer_, &camera_, playerPosition);

	// 敵キャラの生成
	for (int i = 0; i < 1; ++i)
	{
		Enemy* enemy = new Enemy();

		Vector3 pos = 
		{
			SeedManager::GetInstance()->GenerateFloat(-5.0f, 5.0f), 
			SeedManager::GetInstance()->GenerateFloat(-5.0f, 5.0f), 
			7.0f
		};

		enemy->Initialize(modelEnemy_, &camera_, pos);

		enemies_.push_back(enemy);
	}

	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, 1.0f);

	// カメラの初期化
	camera_.farZ = 1000.0f;
	camera_.Initialize();
	

	// デバッグカメラの生成
	debugCamera_ = new DebugCamera(1280, 720);

	// カメラコントローラーの初期化
	cameraController_ = new CameraController();
	cameraController_->Initialize();
	cameraController_->SetTarget(player_->GetWorldTransform().translation_);
	cameraController_->Reset();

	cameraController_->SetMovableArea({-100.0f, -100.0f, -100.0f}, {100.0f,100.0f, 100.0f});

	// 軸方向表示を有効にする
	AxisIndicator::GetInstance()->SetVisible(true);
	// 軸方向表示が参照するカメラを指定する
	AxisIndicator::GetInstance()->SetTargetCamera(&camera_);

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

		//// 背景の更新
		//backGround_->Update();

		// 自キャラの更新
		player_->Update();

		for (Enemy* enemy : enemies_)
		{
			enemy->Update();
		}

		enemies_.remove_if([](Enemy * enemy)
		{ 
			if (enemy->GetIsDead())
			{
				delete enemy;
				return true;
			}
			return false;
		});

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

		

		fade_->Update();

		if (fade_->IsFinished()) 
		{
			phase_ = Phase::kPlay;
		}

		break;

	case Phase::kPlay:

		// 天球の更新
		skydome_->Update();

		//// 背景の更新
		//backGround_->Update();

		// 自キャラの更新
		player_->Update();

		if (Input::GetInstance()->TriggerKey(DIK_F4)) 
		{
			Enemy* enemy = new Enemy();

			Vector3 pos = 
			{
				SeedManager::GetInstance()->GenerateFloat(-5.0f, 5.0f), 
				SeedManager::GetInstance()->GenerateFloat(-5.0f, 5.0f), 
				7.0f
			};

			enemy->Initialize(modelEnemy_, &camera_, pos);

			enemies_.push_back(enemy);
		}

		for (Enemy* enemy : enemies_) 
		{
			enemy->Update();
		}

		enemies_.remove_if([](Enemy* enemy) 
		{
			if (enemy->GetIsDead()) 
			{
				delete enemy;
				return true;
			}
			return false;
		});

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

		// 衝突判定
		CheckAllCollisions();

		if (player_->GetIsDead() || Input::GetInstance()->PushKey(DIK_F2)) {
			// 死亡演出フェーズに切り換え
			phase_ = Phase::kDeath;
			// 自キャラの座標を取得
			const Vector3& deathParticlesPosition = player_->GetWorldPosition();

			// 死亡パーティクルを生成、初期化
			deathParticles_ = new DeathParticles;
			deathParticles_->Initialize(modelDeathParticle_, &camera_, deathParticlesPosition);
		}

		if (Input::GetInstance()->PushKey(DIK_F3)) {
			phase_ = Phase::kFadeOut;
			fade_->Start(Fade::Status::FadeOut, 1.0f);
		}

		break;

	case Phase::kDeath:

		// 天球の更新
		skydome_->Update();

		for (Enemy* enemy : enemies_) {
			enemy->Update();
		}

		enemies_.remove_if([](Enemy* enemy) {
			if (enemy->GetIsDead()) {
				delete enemy;
				return true;
			}
			return false;
		});

		//// 背景の更新
		//backGround_->Update();

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

		if (deathParticles_ && deathParticles_->GetIsFinished()) 
		{
			phase_ = Phase::kFadeOut;
			fade_->Start(Fade::Status::FadeOut, 1.0f);
		}

		break;

	case Phase::kFadeOut:

		// 天球の更新
		skydome_->Update();

		for (Enemy* enemy : enemies_) {
			enemy->Update();
		}

		enemies_.remove_if([](Enemy* enemy) {
			if (enemy->GetIsDead()) {
				delete enemy;
				return true;
			}
			return false;
		});

		//// 背景の更新
		//backGround_->Update();

		// カメラの処理
		if (isDebugCameraActive_) {
			debugCamera_->Update();
			camera_.matView = debugCamera_->GetCamera().matView;
			camera_.matProjection = debugCamera_->GetCamera().matProjection;
			// ビュープロジェクション行列の転送
			camera_.TransferMatrix();
		}

		if (player_)
		{
			// 自キャラの更新
			player_->Update();
		}

	    fade_->Update();

		if (fade_->IsFinished())
		{
			finished_ = true;
		}

		break;

	}


	ImGui::Begin("game scene window");

	ImGui::DragFloat3("camera translation", &camera_.translation_.x, 0.0f);

	ImGui::End();
	

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

		//// 背景の描画
		//backGround_->Draw();

		// 自キャラの描画
		player_->Draw();

		// 敵キャラの描画
		for (Enemy* enemy : enemies_)
		{
			enemy->Draw();
		}

		Sprite::PreDraw(dxCommon->GetCommandList());

		/*EX1Sprite_->Draw();
		EX2Sprite_->Draw();*/

		Sprite::PostDraw();

		fade_->Draw();

		break;

	case Phase::kPlay:

		// 天球の描画
		skydome_->Draw();

		//// 背景の描画
		//backGround_->Draw();

		// 自キャラの描画
		player_->Draw();

		// 敵キャラの描画
		for (Enemy* enemy : enemies_) {
			enemy->Draw();
		}

		Sprite::PreDraw(dxCommon->GetCommandList());

		/*EX1Sprite_->Draw();
		EX2Sprite_->Draw();*/

		Sprite::PostDraw();

		break;

	case Phase::kDeath:

		// 天球の描画
		skydome_->Draw();

		//// 背景の描画
		//backGround_->Draw();

		if (deathParticles_)
		{
			// 死亡エフェクトの描画
			deathParticles_->Draw();
		}

		// 敵キャラの描画
		for (Enemy* enemy : enemies_) {
			enemy->Draw();
		}
		

		Sprite::PreDraw(dxCommon->GetCommandList());

		/*EX1Sprite_->Draw();
		EX2Sprite_->Draw();*/

		Sprite::PostDraw();

		break;

	case Phase::kFadeOut:

		// 天球の描画
		skydome_->Draw();

		//// 背景の描画
		//backGround_->Draw();

		if (deathParticles_)
		{
			// 死亡エフェクトの描画
			deathParticles_->Draw();
		}

		if (player_) 
		{
			//// 自キャラの描画
			//player_->Draw();
		}

		// 敵キャラの描画
		for (Enemy* enemy : enemies_) {
			enemy->Draw();
		}

		Sprite::PreDraw(dxCommon->GetCommandList());

		/*EX1Sprite_->Draw();
		EX2Sprite_->Draw();*/

		Sprite::PostDraw();

		fade_->Draw();

		break;

	}

	AxisIndicator::GetInstance()->Draw();

	// スプライトの描画処理

	
	// 3Dモデル描画後処理
	Model::PostDraw();

}

void GameScene::CheckAllCollisions()
{
	#pragma region 自キャラと敵Mobの当たり判定
	{
		//// 判定対象1と2の距離
		//AABB aabb1, aabb2;

		//// 自キャラの座標
		//aabb1 = player_->GetAABB();

		//// 自キャラと敵Mob全ての当たり判定
		//for (Enemy* enemy : enemies_)
		//{
		//	if (enemy->IsCollisionDisabled())
		//	{
		//		// 衝突無効フラグが立っている場合はスキップ
		//		continue;
		//	}

		//	// 敵Mobの座標
		//	aabb2 = enemy->GetAABB();

		//	// AABB同士の交差判定
		//	if (IsCollision(aabb1, aabb2))
		//	{
		//		// 自キャラの衝突時間数を呼び出す
		//		player_->OnCollision(enemy);
		//		// 敵の衝突時関数を呼び出す
		//		enemy->OnCollision(player_);
		//	}
		//}
	}
	#pragma endregion

	#pragma region 自キャラとゴールの当たり判定
	{
		//// 判定対象1と2の距離
		//AABB aabb1, aabb2;

		//// 自キャラの座標
		//aabb1 = player_->GetAABB();


		//// 敵Mobの座標
		//aabb2 = goal_->GetAABB();

		//// AABB同士の交差判定
		//if (IsCollision(aabb1, aabb2)) 
		//{
		//	// 敵の衝突時関数を呼び出す
		//	goal_->OnCollision(player_);
		//}
		
	}
	#pragma endregion

	#pragma region 自弾と敵Mobの当たり判定
	{
		// 判定対象1と2の距離
		AABB aabb1, aabb2;

		// 自弾の座標
		for (PlayerBullet* bullet : player_->GetBullets())
		{
			aabb1 = bullet->GetAABB();

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
					// 自弾の衝突時間数を呼び出す
					bullet->OnCollision(enemy);
					// 敵の衝突時関数を呼び出す
					enemy->OnCollision(bullet);
				}
			}
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