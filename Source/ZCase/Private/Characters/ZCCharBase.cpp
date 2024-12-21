// 冷静


#include "Characters/ZCCharBase.h"
#include"GameFramework\SpringArmComponent.h"
#include"Camera\CameraComponent.h"
#include "Data/ZCPlayerController.h"
#include"EnhancedInputSubsystems.h"
#include"EnhancedInputComponent.h"
#include"GameFramework\CharacterMovementComponent.h"
#include"Debug/DebugHelper.h"
#include "UI/ZCLayout.h"
#include "DrawDebugHelpers.h"
#include "Components\SkeletalMeshComponent.h"
#include "Actors/BombBase.h"
#include "Components\SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine\StaticMeshActor.h"
#include "PhysicsEngine\PhysicsHandleComponent.h"
#include "Particles\ParticleSystemComponent.h"
#include "Actors/IceActor.h"
#include "Kismet\KismetMathLibrary.h"
#include "Actors/StasisActor.h"
#include "Components/ArrowComponent.h"

AZCCharBase::AZCCharBase()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationRoll = false;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera=CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	Parachutte = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Parachutte"));
	Parachutte->SetupAttachment(GetMesh());
	Parachutte->SetVisibility(false);

	HeadPos = CreateDefaultSubobject<USceneComponent>(TEXT("ReadyThrowPosition"));
	HeadPos->SetupAttachment(GetMesh(),TEXT("socket_head"));

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->MaxWalkSpeed = 500.0f;

	MagHovered = CreateDefaultSubobject<UMaterialInterface>("MagHovered");
	MagNormal = CreateDefaultSubobject<UMaterialInterface>("MagNormal");
	MagDeactivated = CreateDefaultSubobject<UMaterialInterface>("MagDeactivated");

	PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>("PhysicsHandle");
	PhysicsHandle->LinearDamping = 100.0f;
	PhysicsHandle->LinearStiffness = 325.0f;
	PhysicsHandle->AngularDamping = 250.0f;
	PhysicsHandle->AngularStiffness = 750.0f;
	PhysicsHandle->InterpolationSpeed = 5.0f;

	PhysicObjectHolder = CreateDefaultSubobject<USceneComponent>("Physics Object Holder");
	PhysicObjectHolder->SetupAttachment(FollowCamera);

	IceDisabled = CreateDefaultSubobject<UMaterialInterface>("Ice Disabled");
	IceEnabled = CreateDefaultSubobject<UMaterialInterface>("Ice Enabled");
}

void AZCCharBase::BeginPlay()
{
	Super::BeginPlay();
	
	AZCPlayerController* PC = Cast<AZCPlayerController>(Controller);
	if (PC == nullptr)  return;//相当于CastFailed功能

	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
	if (Subsystem == nullptr)return;

	Subsystem->AddMappingContext(IMC_ZC, 0);

	//初始化精力值
	CurStamina = MaxStamina;

	//创建UI
	if (LayoutClassRef)
	{
		LayoutRef = CreateWidget<UZCLayout>(GetWorld(), LayoutClassRef);
		if (LayoutRef)
		{
			LayoutRef->ConstructDeferred(this);
			LayoutRef->AddToViewport();

			
		}
	}

	//为磁铁吸附技能事先筛选场景中的Actor,存放在AlMagSMs数组中
	FilterOutAllMagSMs();
}

void AZCCharBase::Landed(const FHitResult& Hit)
{
	if (CurrentMT == EMovementTypes::MT_Exhausted)
	{
		//立刻回复精力
		StartRecoveringStamina();
		return;
	}
	if (CurrentMT == EMovementTypes::MT_Gliding)
	{
		//如果在空中，会立刻切换为Falling
		LocomotionManager(EMovementTypes::MT_Walking);
		return;
	}
	if (CurrentMT == EMovementTypes::MT_Sprinting)
	{
		//继续持续冲刺状态
		LocomotionManager(EMovementTypes::MT_Sprinting);
	}
	else
	{
		LocomotionManager(EMovementTypes::MT_Walking);
	}
}

#pragma region Move&Camera Node

void AZCCharBase::Move_Triggered(const FInputActionValue& val)
{
	const FVector2D InputVector = val.Get<FVector2D>();
	Vel_X = InputVector.X;
	Vel_Y = InputVector.Y;

	if (Controller == nullptr) return;
	// 只关注水平方向Yaw
	const FRotator GroundRotation(0, Controller->GetControlRotation().Yaw, 0);

	// 左右 EAxis::Y
	const FVector RightDir = FRotationMatrix(GroundRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(RightDir, Vel_X);

	// 前后 EAxis::X
	const FVector FwdDir = FRotationMatrix(GroundRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(FwdDir, Vel_Y);
}

void AZCCharBase::Move_Completed(const FInputActionValue& val)
{
	Vel_X = 0;
	Vel_Y = 0;
}

void AZCCharBase::Look_Triggered(const FInputActionValue& val)
{
	FVector2D LookVal = val.Get<FVector2D>();
	if (Controller == nullptr) return;

	AddControllerYawInput(LookVal.X);
	AddControllerPitchInput(LookVal.Y);
}
#pragma endregion 

#pragma region Sprint Node
void AZCCharBase::Sprint_Triggered(const FInputActionValue& val)
{
	//用于监听，当无输入且在冲刺状态时，取消冲刺状态进入Walking状态
	if (Vel_Y == 0.0f && Vel_X == 0.0f && CurrentMT == EMovementTypes::MT_Sprinting)
	{
		LocomotionManager(EMovementTypes::MT_Walking);
	}
}

void AZCCharBase::Sprint_Started(const FInputActionValue& val)
{

	if (CurrentMT == EMovementTypes::MT_Walking || CurrentMT == EMovementTypes::MT_EMAX)
	{
		LocomotionManager(EMovementTypes::MT_Sprinting);
	}
}

void AZCCharBase::Sprint_Completed(const FInputActionValue& val)
{
	if (CurrentMT == EMovementTypes::MT_Sprinting)
	{
		LocomotionManager(EMovementTypes::MT_Walking);
	}
}

#pragma endregion

#pragma region Jump & Glide Node

void AZCCharBase::JumpGlide_Started(const FInputActionValue& val)
{
	if (CurrentMT == EMovementTypes::MT_Exhausted)
	{
		return;
	}

	if (GetCharacterMovement()->MovementMode != EMovementMode::MOVE_Falling)
	{
		//可以跳跃
		Jump();
		LocomotionManager(EMovementTypes::MT_Falling);
		return;
	}

	if (CurrentMT == EMovementTypes::MT_Gliding)
	{
		//已经在滑翔状态，则取消滑翔进入下落状态
		LocomotionManager(EMovementTypes::MT_Falling);
		//跳出该函数
		return;
	}

	//检查是否距离地面过近，如果过近，则不能进入滑翔状态

	FHitResult HitResult;
	const FVector Start = GetActorLocation();
	const FVector End = Start - EnableGlideDistance;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	bool HitAnything=GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility,
		Params);
	//显示射线
	//DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 5.0f, 0.0f, 3.0f);

	if (HitAnything)
	{
		//不可滑翔
	}
	else
	{
		//TBD(To be developed)-取消激活释放技能状态
		AutoDeactivateAllRunes();
		//切换至Gliding滑翔状态
		LocomotionManager(EMovementTypes::MT_Gliding);
	}
}

void AZCCharBase::JumpGlide_Completed(const FInputActionValue& val)
{
	StopJumping();
}

void AZCCharBase::ToggleUI_Started(const FInputActionValue& val)
{
	AZCPlayerController* PC = Cast<AZCPlayerController>(Controller);

	//TBD-打开UI前，取消激活释放技能的状态
	AutoDeactivateAllRunes();

	//获取蓝图中的WidgetSwitcher的内容
	if (GetWSIndexInfo() == 1)
	{
		//技能UI切换到游戏UI
		PC->bShowMouseCursor=false;
		PC->SetInputMode(FInputModeGameOnly());
		PC->SetPause(false);
		SetWSIndex(0);
	}
	else
	{
		//游戏UI切换回技能UI
		PC->bShowMouseCursor = true;
		FInputModeGameAndUI InputHandle;
		InputHandle.SetWidgetToFocus(LayoutRef->TakeWidget());
		PC->SetInputMode(InputHandle);
		PC->SetPause(true);
		SetWSIndex(1);
	}
}

void AZCCharBase::ActiveRune_Started(const FInputActionValue& val)
{
	ToggleRuneActivity();
}

void AZCCharBase::ReleaseRune_Started(const FInputActionValue& val)
{
	//TBD-检查是否有可投掷的物品
	switch (ActiveRune)
	{
	case ERunes::R_EMAX:
		break;
	case ERunes::R_RBS:
		ReleaseRemoteBombSphere();
		break;
	case ERunes::R_RBB:
		ReleaseRemoteBombCube();
		break;
	case ERunes::R_Mag:
		SelectOrReleaseMagObject();
		break;
	case ERunes::R_Stasis:
		AddStasisForce();
		break;
	case ERunes::R_Ice:
		CreateIce();
		break;
	}
}

void AZCCharBase::Interact_Started(const FInputActionValue& val)
{
	//取消已经激活的技能
	AutoDeactivateAllRunes();

    //TBD-尝试交互
}

#pragma endregion


// Called every frame
void AZCCharBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	//检测当前聚焦目标是否是潜在的可吸附cci贴目标，或更新拖拽位置
	MagDragObjTick();

	//更新冰柱所在的位置
	UpdateIcePositionTick(DeltaTime);
	
	//FString FloatStr = FString::SanitizeFloat(CurStamina);
	//Debug::Print(*FloatStr);
}

void AZCCharBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EIComp = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EIComp == nullptr)return;

	EIComp->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AZCCharBase::Move_Triggered);
	EIComp->BindAction(MoveAction, ETriggerEvent::Completed, this, &AZCCharBase::Move_Completed);

	EIComp->BindAction(LookAction, ETriggerEvent::Triggered, this, &AZCCharBase::Look_Triggered);

	EIComp->BindAction(SprintAction, ETriggerEvent::Triggered, this, &AZCCharBase::Sprint_Triggered);
	EIComp->BindAction(SprintAction, ETriggerEvent::Completed, this, &AZCCharBase::Sprint_Completed);
	EIComp->BindAction(SprintAction, ETriggerEvent::Started, this, &AZCCharBase::Sprint_Started);

	EIComp->BindAction(JumpGlideAction, ETriggerEvent::Completed, this, &AZCCharBase::JumpGlide_Completed);
	EIComp->BindAction(JumpGlideAction, ETriggerEvent::Started, this, &AZCCharBase::JumpGlide_Started);

	EIComp->BindAction(ToggleUIAction, ETriggerEvent::Started, this, &AZCCharBase::ToggleUI_Started);

	EIComp->BindAction(ActiveRuneAction, ETriggerEvent::Started, this, &AZCCharBase::ActiveRune_Started);
	EIComp->BindAction(ReleaseRuneAction, ETriggerEvent::Started, this, &AZCCharBase::ReleaseRune_Started);

	EIComp->BindAction(InteractAction, ETriggerEvent::Started, this, &AZCCharBase::Interact_Started);
}

#pragma region Locomotions

void AZCCharBase::LocomotionManager(EMovementTypes NewMovement)
{
	//控制各个运动
	if (NewMovement == CurrentMT) return;

	CurrentMT = NewMovement;

	//显示/隐藏滑翔翼模型
	if (Parachutte)
	{
		Parachutte->SetVisibility(CurrentMT == EMovementTypes::MT_Gliding);
	}

	//根据枚举值执行不同的运动逻辑
	switch (CurrentMT)
	{
	case EMovementTypes::MT_EMAX:
		break;
	case EMovementTypes::MT_Walking:
		SetWalking();
		break;
	case EMovementTypes::MT_Exhausted:
		SetExhausted();
		break;
	case EMovementTypes::MT_Sprinting:
		SetSprinting();
		break;
	case EMovementTypes::MT_Gliding:
		SetGliding();
		break;
	case EMovementTypes::MT_Falling:
		SetFalling();
		break;
	}
}

void AZCCharBase::ResetToWalk()
{
	//如正在添加重力，此处取消
	GetWorldTimerManager().ClearTimer(AddGravityForFlyingTimerHandle);

	//重置回WalWalk状态（从滑翔、下落状态）
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
}

void AZCCharBase::SetSprinting()
{
	Debug::Print(TEXT("Sprinting"));
	GetCharacterMovement()->MaxWalkSpeed = 1000.0f;
	GetCharacterMovement()->AirControl = 0.35f;

	ResetToWalk();
	//消耗精力槽
	StartDrainingStamina();
}

void AZCCharBase::SetWalking()
{
	Debug::Print(TEXT("Walking"));
	GetCharacterMovement()->MaxWalkSpeed = 500.0f;
	GetCharacterMovement()->AirControl = 0.35f;

	ResetToWalk();
	//回复精力槽
	StartRecoveringStamina();
}

void AZCCharBase::SetExhausted()
{
	GetCharacterMovement()->MaxWalkSpeed = 300.0f;//慢速行走
	GetCharacterMovement()->AirControl = 0.35f;

	ClearDrainRecoverTimers();//清楚Timers,停止消耗

    //如果在下落状态，不立刻回复jing'ljingli；待接触地面后再回复精力
	if (GetCharacterMovement()->MovementMode == EMovementMode::MOVE_Walking)
	{
		StartRecoveringStamina();
	}
	else if (GetCharacterMovement()->MovementMode == EMovementMode::MOVE_Falling)
	{
		ResetToWalk();
	}
	else
	{
		return ;
	}
}

void AZCCharBase::SetGliding()
{
	GetCharacterMovement()->AirControl = 0.6f;
	//设为飞行模式
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);

	StartDrainingStamina();
	//设置模拟重力，每帧执行
	GetWorldTimerManager().SetTimer(AddGravityForFlyingTimerHandle, this,
		&AZCCharBase::AddGravityForFlying, GetWorld()->GetDeltaSeconds(), true);
}

void AZCCharBase::SetFalling()
{
	//下落状态不回复精力zhizhi2，避免无限滑翔
	GetCharacterMovement()->AirControl = 0.35f;

	ResetToWalk();
	ClearDrainRecoverTimers();
}

bool const AZCCharBase::IsCharacterExhausted()
{
	bool bEqual = CurrentMT == EMovementTypes::MT_Exhausted;
		return bEqual;
}

#pragma endregion

#pragma region Stamina

void AZCCharBase::DrainStaminaTimer()
{
	if (CurStamina <= 0.0f)
	{
		LocomotionManager(EMovementTypes::MT_Exhausted);
	}
	else
	{
		CurStamina = FMath::Clamp((CurStamina - StaminaDepletionAmount), 0.0f, MaxStamina);
	}
}

void AZCCharBase::StartDrainingStamina()
{
	//清除已有Timer

	ClearDrainRecoverTimers();

	GetWorldTimerManager().SetTimer(DrainStaminaTimerHandle, this, 
		&AZCCharBase::DrainStaminaTimer,StaminaDepletionRate,true);

	//显示UI
	if (LayoutRef)
	{
		LayoutRef->ShowGaugeAnim(true);
	}
}

void AZCCharBase::RecoverStaminaTimer()
{
	if (CurStamina < MaxStamina)
	{
		CurStamina = FMath::Clamp((CurStamina + StaminaDepletionAmount), 0.0f, MaxStamina);
	}
	else
	{
		GetWorldTimerManager().ClearTimer(RecoverStaminaTimerHandle);
		LocomotionManager(EMovementTypes::MT_Walking);
		//隐藏UI
		if (LayoutRef)
		{
			LayoutRef->ShowGaugeAnim(false);
		}
	}
}

void AZCCharBase::StartRecoveringStamina()
{
	//清除已有Timer
	ClearDrainRecoverTimers();

	GetWorldTimerManager().SetTimer(RecoverStaminaTimerHandle, this, 
		&AZCCharBase::RecoverStaminaTimer,StaminaDepletionRate , true);
}

void AZCCharBase::ClearDrainRecoverTimers()
{
	GetWorldTimerManager().ClearTimer(DrainStaminaTimerHandle);
	GetWorldTimerManager().ClearTimer(RecoverStaminaTimerHandle);
}

void AZCCharBase::AddGravityForFlying()
{
	LaunchCharacter(FVector(0.0f, 0.0f, -100.0f), false, true);
}

#pragma endregion

#pragma region UI
int32 AZCCharBase::GetWSIndexInfo_Implementation()
{
	Debug::Print(TEXT("可执行cpp代码"));
	//若未添加则返回-1
	return -1;
}
#pragma endregion

#pragma region Runes

void AZCCharBase::AutoDeactivateAllRunes()
{
	bool bAnyRuneActive = !bRBActivated && !bMagActivated && !bIceActivated && !bStasisActivated;
	if (!bAnyRuneActive)
	{
		//如果任意一个为true，重置为非激活状态
		bReadyToThrow = false;
		ToggleRuneActivity();
	}
}

void AZCCharBase::ToggleRuneActivity()
{
	if (ActiveRune == ERunes::R_EMAX)return;

	//显示隐藏准星、平滑移动摄像机的相对位置
	bFlipflopCrosshair = !bFlipflopCrosshair;
	CrossHairAndCameraMode(bFlipflopCrosshair);

	LocomotionManager(EMovementTypes::MT_Walking);

	switch (ActiveRune)
	{
	case ERunes::R_EMAX:
		break;
	case ERunes::R_RBS:
		ToggleRemoteBomb();
		break;
	case ERunes::R_RBB:
		ToggleRemoteBomb();
		break;
	case ERunes::R_Mag:
		ToggleMagnesis();
		break;
	case ERunes::R_Stasis:
		ToggleStasisMode();
		break;
	case ERunes::R_Ice:
		ToggleIceMode();
		break;
	}
}

void AZCCharBase::ToggleRemoteBomb()
{
	bRBActivated = !bRBActivated;

	if (!bRBActivated)
	{
		//取消激活
		bHoldingBomb = false;
		bReadyToThrow = false;

		if (BombRef)
		{
			BombRef->Destroy();
			BombRef = nullptr;
		}
	}
}

void AZCCharBase::ReleaseRemoteBombSphere()
{
	bSphereBomb = true;
	ThrowAndIgniteBomb(bSphereBomb);
}

void AZCCharBase::ReleaseRemoteBombCube()
{
	bSphereBomb = false;
	ThrowAndIgniteBomb(bSphereBomb);
}

void AZCCharBase::ThrowAndIgniteBomb(bool bSphere)
{
	//处理扔出炸弹、引爆炸弹
	if (!bRBActivated)return;

	if (BombRef)
	{
		//若炸弹已生成，判断是投掷还是引爆
		if (bHoldingBomb)
		{
			//开启物理模拟
			BombRef->SM->SetSimulatePhysics(true);
			//设置透支的方向
			FVector ThrowDirection=GetThrowDirection();
			BombRef->SM->SetPhysicsLinearVelocity(ThrowDirection);
			//停止举起物品的状态
			bHoldingBomb = false;
			bReadyToThrow = false;
		}
		else
		{
			//点燃炸弹
			BombRef->Detonate();
			BombRef = nullptr;
		}
	}
	else
	{
		//若参考炸弹无效，则生成一个炸弹
		TSubclassOf<ABombBase> BombClass;
		if (bSphere)
		{
			BombClass = SphereBomb;
		}
		else
		{
			BombClass = CubeBomb;
		}

		if (BombClass == nullptr)return;
		FVector tempLoc = HeadPos->GetComponentLocation();
		BombRef=GetWorld()->SpawnActor<ABombBase>(BombClass, tempLoc, FRotator::ZeroRotator);

		//贴附
		BombRef->AttachToComponent(HeadPos, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

		bHoldingBomb = true;
		//播放举起的动画
		bReadyToThrow = true;
	}
}

const FVector AZCCharBase::GetThrowDirection()
{
	FVector FinalDirection(
		FollowCamera->GetForwardVector().X, FollowCamera->GetForwardVector().Y,0.0f);
	//获取其单位向量
	FinalDirection.GetSafeNormal(0.0001f);
	//加一些高度,乘以力度1000
	FinalDirection = FinalDirection + FVector(0.0f, 0.0f, 0.5f);
	FinalDirection = FinalDirection * 1000.0f;
	
	return FinalDirection;
}

void AZCCharBase::FilterOutAllMagSMs()
{
	TArray<AActor*> TempAllMagObjects;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), StaticMeshClass, TempAllMagObjects);

	for (auto ArrayElem : TempAllMagObjects)
	{
		AStaticMeshActor* LocalMag = Cast<AStaticMeshActor>(ArrayElem);
		//根据静态网格体材质中是否有Metal物理材质文件筛选
		EPhysicalSurface LocalSurfaceType = 
			LocalMag->GetStaticMeshComponent()->GetMaterial(0)->GetPhysicalMaterial()->SurfaceType;

		if (LocalSurfaceType == SurfaceType1)//相当于自定义的"Metal"
		{
			AllMagSMs.AddUnique(LocalMag);
		}
	}
}

void AZCCharBase::ToggleMagnesis()
{
	bMagActivated = !bMagActivated;

	//重置所有可吸附物体的材质为"MagDeactive"
	UpdateMagHintMats(AllMagSMs, nullptr);

	if (!bMagActivated)
	{
		//取消激活
		ReleaseMag();
		
	}
}

void AZCCharBase::ReleaseMag()
{
	//释放Magnesis物品
	PhysicsHandle->ReleaseComponent();
	//重置Physic Object Holder的位置
	PhysicObjectHolder->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, 0.0f),FQuat::Identity);
	MagnesisObj = nullptr;
	TempMagHitComp = nullptr;
	//销毁特效
	if (BeamParticleComp != nullptr)
	{
		BeamParticleComp->DestroyComponent();
		BeamParticleComp = nullptr;
	}
}

void AZCCharBase::UpdateMagHintMats(TArray<AStaticMeshActor*>Array, UPrimitiveComponent* HoveredObj)
{
	for (auto ArrayElem : AllMagSMs)
	{
		if (ArrayElem == nullptr)return;
		UStaticMeshComponent* LocalSMC = ArrayElem->GetStaticMeshComponent();
		if (HoveredObj == LocalSMC)
		{
			//设置为高亮状态，即MagHovered
			LocalSMC->SetMaterial(0, MagHovered);
		}
		else
		{
			//是否技能还在激活状态
			if (bMagActivated)
			{
				//如果还在激活，但但鼠标未指向，恢复为普通高亮
				LocalSMC->SetMaterial(0, MagNormal);
			}
			else
			{
				LocalSMC->SetMaterial(0, MagDeactivated);
			}
		}
	}
}

void AZCCharBase::SelectOrReleaseMagObject()
{
	if (!bMagActivated)return;
	GrabMagObject();
}

void AZCCharBase::GrabMagObject()
{
	if (MagnesisObj)
	{
		ReleaseMag();
		return;
	}

	//抓取，射线检测
	FHitResult HitResult;
	FVector Start;
	FVector End;
	CameraLineTraceDir(Start, End, 3000.0f);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	Params.bReturnPhysicalMaterial = true;//此处重要
	GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);

	if (!HitResult.bBlockingHit)return;
	MagnesisObj = HitResult.GetComponent();
	if (MagnesisObj == nullptr)return;
	EPhysicalSurface LocalPhys = UGameplayStatics::GetSurfaceType(HitResult);
	if (LocalPhys != SurfaceType1)return;
	if (!MagnesisObj->IsSimulatingPhysics())return;
	//使用物理组件PhysicsHandle抓取
	FName CustomNone;
	FVector GrabLocation = MagnesisObj->GetComponentLocation();
	FRotator GrabRotation(0.0f, MagnesisObj->GetComponentRotation().Yaw, 0.0f);
	PhysicsHandle->GrabComponentAtLocationWithRotation(MagnesisObj, CustomNone, GrabLocation, GrabRotation);
	//为了跟随摄像机移动，将挂载在FollowCamera下的Physics Object Holder设置为和目标物体相同位置
	PhysicObjectHolder->SetWorldLocation(MagnesisObj->GetComponentLocation());

	BeamParticleComp=UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MagDraggingVFX, GetActorLocation());
}

void AZCCharBase::CameraLineTraceDir(FVector& Start, FVector& End, const float Length)
{
	Start = FollowCamera->GetComponentLocation();
	End = Start + FollowCamera->GetForwardVector() * Length;

}

void AZCCharBase::MagDragObjTick()
{
	if (!bMagActivated)return;

	if (MagnesisObj)
	{
		//更新拖拽位置
		FVector NewLocation = PhysicObjectHolder->GetComponentLocation();
		FRotator NewRotation(0.0f, PhysicObjectHolder->GetComponentRotation().Yaw, 0.0f);
		PhysicsHandle->SetTargetLocationAndRotation(NewLocation, NewRotation);

		//更新连线特效
		if (BeamParticleComp != nullptr)
		{
			//连线由3组起始点组成,共6个点
			//设置第1个点起点
			BeamParticleComp->SetBeamSourcePoint(0, GetActorLocation(), 0);
			//设置第1个点终点
			BeamParticleComp->SetBeamTargetPoint(0, MagnesisObj->GetComponentLocation(), 0);

			//设置第2个点起点
			BeamParticleComp->SetBeamSourcePoint(1, GetActorLocation(), 0);
			//设置第2个点终点
			BeamParticleComp->SetBeamTargetPoint(1, MagnesisObj->GetComponentLocation(), 0);

			//设置第3个点起点
			BeamParticleComp->SetBeamSourcePoint(2, GetActorLocation(), 0);
			//设置第3个点终点
			BeamParticleComp->SetBeamTargetPoint(2, MagnesisObj->GetComponentLocation(), 0);
		}
	}
	else
	{
		//射线检测
		FHitResult HitResult;
		FVector Start;
		FVector End;
		CameraLineTraceDir(Start, End, 3000.0f);
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);

		Params.bReturnPhysicalMaterial = true;//此处重要
		GetWorld()->LineTraceSingleByChannel(HitResult, Start, End,ECC_Visibility,Params);
		if (!HitResult.bBlockingHit)
		{
			//射线未击中有效对象，重置TempMagHitComp
			TempMagHitComp = nullptr;
			UpdateMagHintMats(AllMagSMs, nullptr);
		}
		else
		{
			if (HitResult.GetComponent() == nullptr)return;
			//有聚焦对象发生变化时再更新,降低消耗
			if (HitResult.GetComponent() == TempMagHitComp)return;
			TempMagHitComp= HitResult.GetComponent();

			EPhysicalSurface LocalPhys = UGameplayStatics::GetSurfaceType(HitResult);
			if (LocalPhys != SurfaceType1 && TempMagHitComp->IsSimulatingPhysics())
			{
				UpdateMagHintMats(AllMagSMs, TempMagHitComp);
			}
			else
			{
				UpdateMagHintMats(AllMagSMs, nullptr);
			}
		}
	}
}

void AZCCharBase::ToggleIceMode()
{

	if (bIceActivated)
	{
		DeactivateIce();
	}
	else
	{
		ActivateIce();
	}
}

void AZCCharBase::ActivateIce()
{
	FHitResult HitResult;
	FVector Start;
	FVector End;
	CameraLineTraceDir(Start, End, 3000.0f);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	Params.bReturnPhysicalMaterial = true;//此处重要
	GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);

	if (!HitResult.bBlockingHit)return;
	//生成视觉上的冰柱对象，表示当前预生成的位置，无碰撞
	AActor* tempActor = GetWorld()->SpawnActor<AActor>(IceActorClass,HitResult.Location,FRotator(0,0,0));
	if (tempActor == nullptr)return;
	IceRef = Cast<AIceActor>(tempActor);
	if (IceRef == nullptr)return;

	//激活该模式
	bIceActivated = true;

	//播放展示用冰柱模型的时间轴动画时间轴动画
	IceRef->StartToPlayAnimationLoop();
}

void AZCCharBase::DeactivateIce()
{
	//停止时间轴动画
	IceRef->StopPlayingAnimation();

	//销毁IceRef并重置
	IceRef->Destroy();
	IceRef = nullptr;
	//取消激活
	bIceActivated = false;
}

void AZCCharBase::UpdateIcePositionTick(float DeltaTime)
{
	if (!IceRef || !bIceActivated)return;

	//检查射线是否检测到通道为Water的物体对象
	FHitResult HitResult;
	FVector Start;
	FVector End;
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECollisionChannel::ECC_GameTraceChannel1);
	CameraLineTraceDir(Start, End, 5000.0f);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	bool bHitAny = GetWorld()->LineTraceSingleByObjectType(HitResult, Start, End, ObjectParams, Params);

	if (!bHitAny)
	{
		if (IceRef)
		{
			// 隐藏视觉显示的冰柱
			IceRef->BaseSceneRoot->SetVisibility(false, true);
		}
		// 停止执行
		return;
	}

	IceRef->BaseSceneRoot->SetVisibility(true, true);
	//平滑移动视觉显示的冰柱，使用插值
	FVector tempTargetLoc = HitResult.Location;
	FVector finalTargetLoc = UKismetMathLibrary::VInterpTo(IceRef->GetActorLocation(), tempTargetLoc, DeltaTime, 10.0f);
	IceRef->SetActorLocation(finalTargetLoc);
	//将向量转化为角度
	FRotator LocalRot = UKismetMathLibrary::MakeRotFromZ(HitResult.ImpactNormal);
	IceRef->SetActorRotation(LocalRot);
	//检查是否重叠
	IceRef->bCanPlace = IceRef->CheckOverlapping();
	if (IceRef->bCanPlace)
	{
		IceRef->IceMesh->SetMaterial(0, IceEnabled);
	}
	else
	{
		IceRef->IceMesh->SetMaterial(0, IceDisabled);
	}
	
}

void AZCCharBase::CreateIce()
{
	if (bIceActivated)
	{
		IceRef->SpawnIce();
	}
}

void AZCCharBase::ToggleStasisMode()
{
	bStasisActivated = !bStasisActivated;
}

void AZCCharBase::AddStasisForce()
{
	if (!bStasisActivated) return;

	StartStasis();
}

void AZCCharBase::StartStasis()
{
	bool bSPhysics = false;
	UPrimitiveComponent* tempComp = nullptr;

	if (StasisComp)
	{
		//添加力
		StasisTrace(tempComp, bSPhysics);
		//一段时间内只能对同一个物体施加力
		if (tempComp != StasisComp)return;
		AddForceForStasisActor();
	}
	else
	{
		//激活当前StasisActor
		StasisTrace(tempComp, bSPhysics);
		if (!bSPhysics)return;
		StasisComp = tempComp;
		StasisComp->SetSimulatePhysics(false);
		//将原始材质存起来，方便取消激活状态时外观回复原状
		OriginMatStasis=StasisComp->GetMaterial(0);
		//激活时显示特殊材质
		StasisComp->SetMaterial(0, StasisMat);
		//设置Timer，延时5秒
		GetWorld()->GetTimerManager().SetTimer(StasisTimer, this, &AZCCharBase::BreakStasis, 5.0f, false);
	}
}

void AZCCharBase::StasisTrace(UPrimitiveComponent*& HitComp, bool bSimulatePhysc)
{
	FHitResult HitResult;
	FVector Start;
	FVector End;
	CameraLineTraceDir(Start, End, 3000.0f);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	bool HitAny=GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility,Params);

	if (HitAny)
	{
		HitComp = HitResult.GetComponent();
		bSimulatePhysc = HitResult.GetComponent()->IsSimulatingPhysics();
	}
	else
	{
		HitComp = nullptr;
		bSimulatePhysc = false;
	}
}

void AZCCharBase::AddForceForStasisActor()
{
	// 生成StasisForceActor
	if (StasisForceActor == nullptr)
	{
		StasisForceActor = Cast<AStasisActor>(GetWorld()->SpawnActor<AActor>
			(StasisClass, StasisComp->GetComponentLocation(), FRotator::ZeroRotator));
	}
	// 注意，无else
	// 更新StasisForceActor中的箭头反向，仅视觉
	FRotator Rot = UKismetMathLibrary::Conv_VectorToRotator(FollowCamera->GetForwardVector());
	UArrowComponent* ArrowCompStasis = StasisForceActor->GetArrowComponent();
	ArrowCompStasis->SetWorldRotation(Rot);
	StasisForceActor->UpdateForceInfo();
}

void AZCCharBase::BreakStasis()
{
	// 重新设置物理模式十分重要
	StasisComp->SetSimulatePhysics(true);

	if (StasisForceActor)
	{
		FName tempName;
		StasisComp->AddImpulse(StasisForceActor->GetImpulse(), tempName, true);
		// 回复原有材质
		StasisComp->SetMaterial(0, OriginMatStasis);
		// 摧毁StasisForceActor，并置空
		StasisForceActor->Destroy();
		StasisForceActor = nullptr;
		StasisComp = nullptr;
	}
	else
	{
		// 若没有施加力（仅点击一次），则5秒后回复原有材质，并置空
		StasisComp->SetMaterial(0, OriginMatStasis);
		StasisComp = nullptr;
	}
}

#pragma endregion
