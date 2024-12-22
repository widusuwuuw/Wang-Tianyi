//冷静

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StasisActor.generated.h"

class UArrowComponent;

UCLASS()
class ZCASE_API AStasisActor : public AActor
{
	GENERATED_BODY()

public:
	AStasisActor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UArrowComponent* IndicatorArrow;

	int32 Impulse = 0;
	int32 Hits = 0; // 施加力的次数

	FVector GetImpulse();

	void UpdateForceInfo();

	FORCEINLINE class UArrowComponent* GetArrowComponent() const { return IndicatorArrow; }
};
