#include "HitEffect.h"
#include "RandomEngine.h"

Model* HitEffect::model_ = nullptr;
Camera* HitEffect::camera_ = nullptr;

HitEffect* HitEffect::Create(const Vector3& position)
{
	// インスタンスを生成
	HitEffect* instance = new HitEffect();
	// newの失敗を検出
	assert(instance);
	// インスタンスの初期化
	instance->Initialize(position);
	// 初期化したインスタンスを返す
	return instance;
}

void HitEffect::Initialize(const Vector3& position)
{
	circleWorldTransform_.Initialize();
	circleWorldTransform_.translation_ = position;
	circleWorldTransform_.translation_.z -= 0.01f;

	for (WorldTransform& worldTransform : ellipseWorldTransform_) 
	{
		worldTransform.scale_ = { 0.1f, 2.0f, 1.0f };			  // スケールを設定
		worldTransform.rotation_ = 
		{ 0.0f, 0.0f, RandomEngine::GenerateFloat(-pi, pi) }; // 回転を設定
		worldTransform.translation_ = position;				  // 位置を設定

		worldTransform.Initialize();
	}

	objectColor_.Initialize();
	objectColor_.SetColor({1.0f, 1.0f, 1.0f, 1.0f}); // 色を設定

	// 振る舞いを初期化
	behavior_ = Behavior::kSpred; // 初期モードはスプレッド
	behaviorRequest_ = behavior_; // リクエストモードもスプレッド

	ChangeBehavior(behaviorRequest_);
}

void HitEffect::Update() 
{
	if (behaviorRequest_ != behavior_) 
	{
		// モード変更
		ChangeBehavior(behaviorRequest_);
	}

	currentState_->Update(this);

	// 円形エフェクト
	circleWorldTransform_.matWorld_ = MakeAffineMatrixB(
		circleWorldTransform_.scale_, 
		circleWorldTransform_.rotation_,
		circleWorldTransform_.translation_);

	circleWorldTransform_.TransferMatrix();

	// 楕円形エフェクト
	for (WorldTransform& worldTransform : ellipseWorldTransform_) 
	{
		worldTransform.matWorld_ = MakeAffineMatrixB(
			worldTransform.scale_, 
			worldTransform.rotation_, 
			worldTransform.translation_);
		worldTransform.TransferMatrix();
	}
}

void HitEffect::Draw()
{
	// モデルが設定されていない場合は描画しない
	if (!model_)
		return;
	// オブジェクトカラーを nullptr に設定して描画
	
	// 楕円形エフェクトの描画
	for (const WorldTransform& worldTransform : ellipseWorldTransform_) 
	{
		model_->Draw(worldTransform, *camera_, &objectColor_);
	}
	
	// 円形エフェクトの描画
	model_->Draw(circleWorldTransform_, *camera_, &objectColor_);
}

void HitEffect::BehaviorSpredUpdate()
{ 
	circleWorldTransform_.scale_ = circleWorldTransform_.scale_ * 1.02f;

	for (WorldTransform& worldTransform : ellipseWorldTransform_) 
	{
		worldTransform.scale_ = worldTransform.scale_ * 1.02f;
	}
}

void HitEffect::BehaviorFeadUpdate()
{ 
	objectColor_.SetColor(objectColor_.GetColor() - Vector4(0.0f, 0.0f, 0.0f, 0.05f)); 
}

void HitEffect::ChangeBehavior(Behavior behavior) 
{
	if (currentState_) 
	{
		currentState_->Shutdown(this);
		currentState_.reset();
	}

	// 現在の状態を変更する
	behavior_ = behavior;

	// 状態に応じた処理を行う
	switch (behavior_) 
	{
	case Behavior::kSpred:
		// Root状態の初期化処理
		currentState_ = std::make_unique<HitEffectStateSpred>();
		break;

	case Behavior::kFade:
		// Attack状態の初期化処理
		currentState_ = std::make_unique<HitEffectStateFade>();
		break;

	default:
		break;
	}

	if (!currentState_) 
	{
		return;
	}
	currentState_->Initialize(this);
}

void HitEffectStateSpred::Initialize(HitEffect* hitEffect) 
{
	hitEffect = hitEffect; 
}

void HitEffectStateSpred::Update(HitEffect* hitEffect) 
{ 
	animationTimer_ += 1.0f / 60.0f; // アニメーションタイマーを更新

	hitEffect->BehaviorSpredUpdate(); 

	if (animationTimer_ >= spredAnimation_)
	{
		hitEffect->SetBehaviorRequest(HitEffect::Behavior::kFade); // フェード状態に変更
	}
}

void HitEffectStateSpred::Draw(HitEffect* hitEffect) 
{ 
	hitEffect->Draw(); 
}

void HitEffectStateSpred::Shutdown(HitEffect* hitEffect) 
{ 
	hitEffect = hitEffect; 
}

void HitEffectStateFade::Initialize(HitEffect* hitEffect) 
{ 
	hitEffect = hitEffect; 
}

void HitEffectStateFade::Update(HitEffect* hitEffect) 
{ 
	animationTimer_ += 1.0f / 60.0f; // アニメーションタイマーを更新

	hitEffect->BehaviorFeadUpdate(); 

	if (animationTimer_ >= fadeAnimation_) 
	{
		hitEffect->SetIsDead(true); // エフェクトを終了状態にする
	}
}

void HitEffectStateFade::Draw(HitEffect* hitEffect) 
{ 
	hitEffect->Draw(); 
}

void HitEffectStateFade::Shutdown(HitEffect* hitEffect) 
{
	hitEffect = hitEffect;
}