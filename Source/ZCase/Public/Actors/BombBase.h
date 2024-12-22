// 冷静

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components\StaticMeshComponent.h"
#include "Particles\ParticleSystemComponent.h"
#include "BombBase.generated.h"

class UStaticMeshComponent;
class UParticleSystem;
class USoundBase;
class UCameraShakeBase;
class AFieldSystemActor;
class URadialForceComponent;

UCLASS()
class ZCASE_API ABombBase : public AActor
{
	GENERATED_BODY()

public:
	ABombBase();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent* SM;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	URadialForceComponent* RFComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	UParticleSystem* BombSpawnVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	UParticleSystem* BombExplosionVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	USoundBase* ExplosionSFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	TSubclassOf<UCameraShakeBase> ExplosionShake;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	TSubclassOf<AActor> FieldActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	TSubclassOf<AActor> WindTunnelClass;

protected:
	virtual void BeginPlay() override;

public:
	void Detonate();
};
