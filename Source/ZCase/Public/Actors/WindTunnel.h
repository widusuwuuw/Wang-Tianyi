// 冷静

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Characters/ZCCharBase.h"
#include "WindTunnel.generated.h"

class AZCCharBase;
class UBoxComponent;

UCLASS()
class ZCASE_API AWindTunnel : public AActor
{
	GENERATED_BODY()
	
public:	
	AWindTunnel();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true"))
	bool bTemporaryWT=false;

	UPROPERTY(EditAnywhere)
    AZCCharBase* PlayerRef;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Box;



protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnOverlapBegin( UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, 
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
