//冷静

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/MyInterface.h"
#include "InteractBase.generated.h"

class USphereComponent;
class AZCCharBase;

UCLASS()
class ZCASE_API AInteractBase : public AActor, public IMyInterface
{
	GENERATED_BODY()

public:
	AInteractBase();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* BaseMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USphereComponent* InteractSphere;

	AZCCharBase* PlayerRef;

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION(BlueprintImplementableEvent)
	void PlayerEntered();

	virtual void PlayerEnteredCpp();

	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintImplementableEvent)
	void PlayerLeft();

	virtual void PlayerLeftCpp();

	virtual void ToggleInteraction(AZCCharBase* playerRef);

	UFUNCTION(BlueprintImplementableEvent)
	void ToggleInteractionBP(AZCCharBase* playerRef);

	virtual void NextActionInInteractionActor();

	virtual void NextAction() override;

};
