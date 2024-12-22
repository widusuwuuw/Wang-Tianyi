//冷静

#include "Actors/InteractBase.h"
#include "Components\SphereComponent.h"
#include "Debug/DebugHelper.h"

AInteractBase::AInteractBase()
{
	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>("Base Mesh");
	BaseMesh->SetupAttachment(RootComponent);

	InteractSphere = CreateDefaultSubobject<USphereComponent>("Interaction Sphere");
	InteractSphere->SetupAttachment(BaseMesh);
	InteractSphere->SetSphereRadius(150.0f);
	InteractSphere->OnComponentBeginOverlap.AddDynamic(this, &AInteractBase::OnBeginOverlap);
	InteractSphere->OnComponentEndOverlap.AddDynamic(this, &AInteractBase::OnEndOverlap);
}

void AInteractBase::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 检查是否是玩家
	if (OtherActor->ActorHasTag(FName("tag_player")))
	{
		PlayerEntered();
		PlayerEnteredCpp();
	}
}

void AInteractBase::PlayerEnteredCpp()
{
}

void AInteractBase::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// 检查是否是玩家
	if (OtherActor->ActorHasTag(FName("tag_player")))
	{
		PlayerLeft();
		PlayerLeftCpp();
	}
}

void AInteractBase::PlayerLeftCpp()
{
}

void AInteractBase::ToggleInteraction(AZCCharBase* playerRef)
{
}

void AInteractBase::NextActionInInteractionActor()
{

}

void AInteractBase::NextAction()
{
	NextActionInInteractionActor();
}
