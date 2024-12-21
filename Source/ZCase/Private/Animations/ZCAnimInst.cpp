// 冷静


#include "Animations/ZCAnimInst.h"
#include "Kismet\KismetMathLibrary.h"

void UZCAnimInst::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	PlayerRef = Cast<AZCCharBase>(TryGetPawnOwner());
	if (PlayerRef == nullptr)return;
	MoveComp = PlayerRef->GetCharacterMovement();
}

void UZCAnimInst::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (!PlayerRef || !MoveComp)return;

	GroundSpeed = UKismetMathLibrary::VSizeXY(PlayerRef->GetVelocity());
	AIrSpeed = PlayerRef->GetVelocity().Z;
	bIsFalling = MoveComp->IsFalling();
	bShouldMove = !bIsFalling && GroundSpeed > 5.0f && MoveComp->GetCurrentAcceleration().Size() > 0;
	bIsGliding = PlayerRef->CurrentMT == EMovementTypes::MT_Gliding;
	bReadyToThrow = PlayerRef->bReadyToThrow;
}
