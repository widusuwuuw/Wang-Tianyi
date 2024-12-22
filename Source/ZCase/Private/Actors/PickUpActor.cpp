//冷静

#include "Actors/PickupActor.h"
#include "Blueprint/UserWidget.h"
#include "Components/SphereComponent.h"
#include "Characters/ZCCharBase.h"

APickupActor::APickupActor()
{
}

void APickupActor::PlayerEnteredCpp()
{
	// 创建UI提示
	if (HintClass == nullptr) return;
	HintUI = CreateWidget<UUserWidget>(GetWorld(), HintClass);
	HintUI->AddToViewport();
}

void APickupActor::PlayerLeftCpp()
{
	// 消除UI提示
	if (HintUI == nullptr) return;
	HintUI->RemoveFromParent();
	HintUI = nullptr;
}

void APickupActor::ToggleInteraction(AZCCharBase* playerRef)
{
	if (playerRef == nullptr) return;
	PlayerRef = playerRef;

	RemoveHintAndSetSphereInvalid();

	SetActorEnableCollision(false);
	BaseMesh->SetSimulatePhysics(false);

	BaseMesh->AttachToComponent(
		playerRef->HeadPos,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	PlayerRef->CrossHairAndCameraMode(true);
	PlayerRef->bReadyToThrow = true;
}

void APickupActor::NextActionInInteractionActor()
{
	// 放下抬起的物品
	if (PlayerRef == nullptr) return;
	PlayerRef->ReadyToThrow(BaseMesh);
	SetActorEnableCollision(true);
	ResetSphereAfterThrowing();
	PlayerRef->bReadyToThrow = false;
	PlayerRef = nullptr;
}

void APickupActor::RemoveHintAndSetSphereInvalid()
{
	// 拿起的时候隐藏UI提示
	if (HintUI == nullptr) return;
	HintUI->RemoveFromParent();
	HintUI = nullptr;

	InteractSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void APickupActor::ResetSphereAfterThrowing()
{
	// 延迟2秒后重置Sphere
	GetWorld()->GetTimerManager().SetTimer(DelayResetSphereHandle, this,
		&APickupActor::ResetSphere, 2.0f, false);
}

void APickupActor::NextAction()
{
	//Super::NextAction()
	//两种都可以，但不要两个都写，会执行2次
	NextActionInInteractionActor();
}

void APickupActor::ResetSphere()
{
	InteractSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}
