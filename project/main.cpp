#include <Windows.h>
#include "KamataEngine.h"
#include "TitleScene.h"
#include "GameScene.h"

using namespace KamataEngine;

// 確認用
TitleScene* titleScene = nullptr;
GameScene* gameScene = nullptr;

// シーン（型）
enum class Scene
{
	kUnknown = 0,

	kTitle,
	kGame,
};

// 現在シーン（型）
Scene scene = Scene::kUnknown;

void ChangeScene();
void UpdateScene();
void DrawScene();

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) 
{
	// 初期化処理
	// エンジンの初期化
	KamataEngine::Initialize(L"AL4");

	// DirectXCommonインスタンスの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// 最初のシーンの初期化
	scene = Scene::kTitle;

#ifdef _DEBUG

	scene = Scene::kGame;

#endif // DEBUG

	// タイトルシーンのインスタンス作成
	titleScene = new TitleScene();
	// タイトルシーンの初期化
	titleScene->Initialize();

	// ゲームシーンのインスタンス作成
	gameScene = new GameScene();
	// ゲームシーンの初期化
	gameScene->Initialize();

	// メインループ
	while (true)
	{
		// エンジンの更新
		if (KamataEngine::Update())
		{
			break;
		}

		// シーン切り替え
		ChangeScene();

		ImGuiManager::GetInstance()->Begin();


		// 現在シーン更新
		UpdateScene();

		ImGuiManager::GetInstance()->End();

		// 描画開始
		dxCommon->PreDraw();

		/// ここに描画処理を記述する
		// 現在シーンの描画
		DrawScene();

		ImGuiManager::GetInstance()->Draw();

		// 描画終了
		dxCommon->PostDraw();

	}

	// 解放処理
	KamataEngine::Finalize();

	// ゲームシーンの解放
	delete titleScene;
	delete gameScene;
	// nullptrの代入
	gameScene = nullptr;

	return 0;
}

void ChangeScene()
{ 
	switch (scene)
	{ 
	case Scene::kTitle:
			
		if (titleScene->IsFinished())
		{
			// シーン変更
			scene = Scene::kGame;
			// 旧シーンの解放
			delete titleScene;
			titleScene = nullptr;
			// 新シーンの生成と初期化
			gameScene = new GameScene;
		    gameScene->Initialize();
		}

		break;

	case Scene::kGame:

		if (gameScene->IsFinished()) 
		{
			// シーン変更
			scene = Scene::kTitle;
			// 旧シーンの解放
			delete gameScene;
			gameScene = nullptr;
			// 新シーンの生成と初期化
			titleScene = new TitleScene;
			titleScene->Initialize();
		}

		break;
	}
}

void UpdateScene()
{ 
	switch (scene)
	{
	case Scene::kTitle:

		titleScene->Update();

		break;

	case Scene::kGame:

		gameScene->Update();

		break;
	}
}

void DrawScene() 
{
	switch (scene) 
	{
	case Scene::kTitle:

		titleScene->Draw();

		break;

	case Scene::kGame:

		gameScene->Draw();

		break;
	}
}