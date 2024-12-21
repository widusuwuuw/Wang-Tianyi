// 冷静

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ZCCharBase.generated.h"

//前置声明
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UZCLayout;
class USkeletalMeshComponent;
class ABombBase;
class USceneComponent;
class AStaticMeshActor;
class UPhysicsHandleComponent;
class UParticleSystem;
class AIceActor;
class AStasisActor;

UENUM(BlueprintType)
enum class EMovementTypes
{
	MT_EMAX UMETA(DisplayName =EMAX),            //缺省值，留白
	MT_Walking UMETA(DisplayName = Walking),     //地面移动状态
	MT_Exhausted UMETA(DisplayName = Exhausted), //精力耗尽，不可跑步，冲刺，滑行
	MT_Sprinting UMETA(DisplayName=Sprinting),   //冲刺状态
	MT_Gliding UMETA(DisplayName=Gliding),       //滑翔状态
	MT_Falling UMETA(DisplayName=Falling)        //下落状态，不可跑步、冲刺
};

UENUM(BlueprintType)
enum class ERunes
{
	R_EMAX UMETA(DisplayName =EMAX),            //缺省值，留白
	R_RBS UMETA(DisplayName = RBS),             //（遥控炸弹）球形
	R_RBB UMETA(DisplayName = RBB),             //（遥控炸弹）方形
	R_Mag UMETA(DisplayName=Magnesis),          //磁铁吸附
	R_Stasis UMETA(DisplayName=Stasis),         //时停
	R_Ice UMETA(DisplayName=Ice)                //水面生成冰柱
};


UCLASS()
class ZCASE_API AZCCharBase : public ACharacter
{
	GENERATED_BODY()

public:
	AZCCharBase();

#pragma region Variables


	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Comps")
	USpringArmComponent* CameraBoom;
	//TobjectPtr<USpringArmComponent CameraBoom 等价
	//可追踪、软引用

	UPROPERTY(EditAnywhere,Category="Comps")
	UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, Category = "Comps")
	UPhysicsHandleComponent* PhysicsHandle;

	UPROPERTY(EditAnywhere,Category="Comps")
	USceneComponent* PhysicObjectHolder;

	UPROPERTY(EditAnywhere,Category="Comps")
	USkeletalMeshComponent* Parachutte;

	UPROPERTY(EditAnywhere,Category="Comps")
	USceneComponent* HeadPos;

	UPROPERTY(EditAnywhere,Category="Inputs")
	UInputMappingContext* IMC_ZC;

	UPROPERTY(EditAnywhere,Category="Inputs")
	UInputAction*MoveAction;

	UPROPERTY(EditAnywhere,Category="Inputs")
	UInputAction*LookAction;

	UPROPERTY(EditAnywhere,Category="Inputs")
	UInputAction*SprintAction;

	UPROPERTY(EditAnywhere,Category="Inputs")
	UInputAction*JumpGlideAction;

	UPROPERTY(EditAnywhere,Category="Inputs")
	UInputAction*ToggleUIAction;

	UPROPERTY(EditAnywhere,Category="Inputs")
	UInputAction*ActiveRuneAction;

	UPROPERTY(EditAnywhere,Category="Inputs")
	UInputAction*ReleaseRuneAction;

	UPROPERTY(EditAnywhere,Category="Inputs")
	UInputAction*InteractAction;

	UPROPERTY(EditAnywhere,Category="Movements")
	EMovementTypes CurrentMT{EMovementTypes::MT_EMAX};

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> LayoutClassRef;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="UI")
	UZCLayout* LayoutRef;

	UPROPERTY(EditDefaultsOnly, Category = "Movements")
	FVector EnableGlideDistance=FVector(0.0f,0.0f,150.0f);

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Rune | Magnesis")
	TSubclassOf<AActor>StaticMeshClass;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Rune | Magnesis")
	UParticleSystem* MagDraggingVFX;

	UParticleSystemComponent* BeamParticleComp;

	TArray<AStaticMeshActor*> AllMagSMs;//存放当前场景中所有的可吸附物体

	UPrimitiveComponent* MagnesisObj = nullptr;
	UPrimitiveComponent* TempMagHitComp = nullptr;//更改磁铁吸附材质用

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Rune | Magnesis")
	UMaterialInterface* MagHovered=nullptr;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Rune | Magnesis")
	UMaterialInterface* MagNormal=nullptr;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Rune | Magnesis")
	UMaterialInterface* MagDeactivated=nullptr;
	
	AIceActor* IceRef=nullptr;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="Rune | Ice")
	TSubclassOf<AActor> IceActorClass;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="Rune | Ice")
	UMaterialInterface* IceDisabled=nullptr;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="Rune | Ice")
	UMaterialInterface* IceEnabled=nullptr;

	UMaterialInterface* OriginMatStasis = nullptr;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="Rune | Stasis")
	UMaterialInterface* StasisMat = nullptr;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="Rune | Stasis")
	TSubclassOf<AActor> StasisClass;//是一个类

	FTimerHandle StasisTimer;

	AStasisActor* StasisForceActor;

	bool bReadyToThrow=false;
	
	ERunes ActiveRune{ERunes::R_EMAX};

	bool bRBActivated = false;

	bool bMagActivated=false;

	bool bIceActivated = false;

	bool bStasisActivated = false;

	UPROPERTY(EditAnywhere, Category = "Runes")
	ABombBase* BombRef;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Runes")
	TSubclassOf<ABombBase>SphereBomb;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Runes")
	TSubclassOf<ABombBase>CubeBomb;

	float Vel_X;
	float Vel_Y;

	bool bFlipflopCrosshair = false;

    bool bHoldingBomb = false;
	bool bSphereBomb = false;

	UPrimitiveComponent* StasisComp;
#pragma endregion



protected:
	virtual void BeginPlay() override;

	virtual void Landed(const FHitResult& Hit)override;

#pragma region Inputs Node
	UFUNCTION()
	void Move_Triggered(const FInputActionValue& val);

	UFUNCTION()
	void Move_Completed(const FInputActionValue& val);

	UFUNCTION()
	void Look_Triggered(const  FInputActionValue& val);

	UFUNCTION()
	void Sprint_Triggered(const  FInputActionValue& val);

	UFUNCTION()
	void Sprint_Started(const  FInputActionValue& val);

	UFUNCTION()
	void Sprint_Completed(const  FInputActionValue& val);

	UFUNCTION()
	void JumpGlide_Started(const  FInputActionValue& val);

	UFUNCTION()
	void JumpGlide_Completed(const  FInputActionValue& val);

	UFUNCTION()
	void ToggleUI_Started(const  FInputActionValue& val);

	UFUNCTION()
	void ActiveRune_Started(const  FInputActionValue& val);

	UFUNCTION()
	void ReleaseRune_Started(const  FInputActionValue& val);

	UFUNCTION()
	void Interact_Started(const  FInputActionValue& val);

#pragma endregion

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

#pragma region Locomotion

	UFUNCTION()
	void LocomotionManager(EMovementTypes NewMovement);

	void ResetToWalk();

	void SetSprinting();

	void SetWalking();

	void SetExhausted();

	void SetGliding();

	void SetFalling();

	UFUNCTION(BlueprintCallable,BlueprintPure)
	bool const IsCharacterExhausted();

#pragma endregion

#pragma region Stamina

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Stamina")
	float CurStamina=0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stamina")
	float MaxStamina=1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stamina")
	float StaminaDepletionRate=0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stamina")
	float StaminaDepletionAmount=0.05f;

	void DrainStaminaTimer();

	FTimerHandle DrainStaminaTimerHandle;

	void StartDrainingStamina();

	void RecoverStaminaTimer();

	FTimerHandle RecoverStaminaTimerHandle;

	void StartRecoveringStamina();

	void ClearDrainRecoverTimers();

	FTimerHandle AddGravityForFlyingTimerHandle;

	void AddGravityForFlying();

#pragma endregion

#pragma region UI
	UFUNCTION(BlueprintNativeEvent)
	int32 GetWSIndexInfo();

	UFUNCTION(BlueprintImplementableEvent)
	void SetWSIndex(int32 index);
#pragma endregion



#pragma region Runes

	void AutoDeactivateAllRunes();

	UFUNCTION()
	void ToggleRuneActivity();

	UFUNCTION(BlueprintImplementableEvent)
	void CrossHairAndCameraMode(bool Display);

	UFUNCTION()
	void ToggleRemoteBomb ();

	UFUNCTION()
	void ReleaseRemoteBombSphere ();
	void ReleaseRemoteBombCube();
	void ThrowAndIgniteBomb(bool bSphere);
	const FVector GetThrowDirection();

	//Magnesis,磁铁吸附
	void FilterOutAllMagSMs();
	
	UFUNCTION()
	void ToggleMagnesis();

	void ReleaseMag();
	void UpdateMagHintMats(TArray<AStaticMeshActor*>Array,UPrimitiveComponent* HoveredObj);
	void SelectOrReleaseMagObject();
	void GrabMagObject();
	void CameraLineTraceDir(FVector &Start, FVector &End, const float Length);
	void MagDragObjTick();

	//Ice,生成冰柱
	UFUNCTION()
	void ToggleIceMode();

	void ActivateIce();
	void DeactivateIce();

	void UpdateIcePositionTick(float DeltaTime);
	void CreateIce();

	//Stasis,时间暂停
	UFUNCTION()
	void ToggleStasisMode();

	void AddStasisForce();
	void StartStasis();
	void StasisTrace(UPrimitiveComponent*& HitComp, bool bSimulatePhysc);
	void AddForceForStasisActor();
	void BreakStasis();

#pragma endregion
#pragma endregion
};
