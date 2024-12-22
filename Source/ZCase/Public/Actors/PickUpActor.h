//冷静

#pragma once

#include "CoreMinimal.h"
#include "Actors/InteractBase.h"
#include "PickupActor.generated.h"

UCLASS()
class ZCASE_API APickupActor : public AInteractBase
{
	GENERATED_BODY()

public:
	APickupActor();

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> HintClass;

	UUserWidget* HintUI;

	FTimerHandle DelayResetSphereHandle;

protected:
	virtual void PlayerEnteredCpp() override;
	virtual void PlayerLeftCpp() override;

	virtual void ToggleInteraction(AZCCharBase* playerRef) override;
	virtual void NextActionInInteractionActor() override;

public:
	void RemoveHintAndSetSphereInvalid();
	void ResetSphereAfterThrowing();

	virtual void NextAction() override;

private:
	void ResetSphere();
};
