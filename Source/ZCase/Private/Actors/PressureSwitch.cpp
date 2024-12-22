//冷静

#include "Actors/PressureSwitch.h"
#include "Components/BoxComponent.h"


APressureSwitch::APressureSwitch()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>("Scene Root");
	SceneRoot->SetupAttachment(RootComponent);

	BoxCollider = CreateDefaultSubobject<UBoxComponent>("Box Collider");
	BoxCollider->SetupAttachment(SceneRoot);
	BoxCollider->SetCollisionResponseToAllChannels(ECR_Overlap);

	Switcher = CreateDefaultSubobject<UStaticMeshComponent>("Switcher");
	Switcher->SetupAttachment(SceneRoot);

	Base = CreateDefaultSubobject<UStaticMeshComponent>("Base");
	Base->SetupAttachment(SceneRoot);

}

void APressureSwitch::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
