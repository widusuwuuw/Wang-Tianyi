// 冷静

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TimelineComponent.h"
#include "IceActor.generated.h"

class USceneComponent;
class UBoxComponent;
class UCurveFloat;
struct FTimeline;

UCLASS()
class ZCASE_API AIceActor : public AActor
{
	GENERATED_BODY()
	
public:	

	AIceActor();

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	USceneComponent* BaseSceneRoot;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	UStaticMeshComponent* IceMesh;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	UBoxComponent* CheckOverlapComp;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	UBoxComponent* SolidBoxComp;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	TSubclassOf<AIceActor> SpawnClass;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	UCurveFloat* CollisionCurve=nullptr;

	FTimeline CollisionTimeline;

	FVector ExtentStart;
	FVector ExtentEnd;
	FVector RelativeStart;
	FVector RelativeEnd;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	UCurveFloat* VisualCubeCurve=nullptr;

	FTimeline VisualCubeTimeline;

	FVector WorldScaleStart;
	FVector WorldScaleEnd;

	bool bCanPlace = false;

public:	
	virtual void BeginPlay()override;

	virtual void Tick(float DeltaTime) override;

	bool CheckOverlapping();

	void EnableCollision();
	
	void SpawnIce();

	UFUNCTION()
	void CollisionUpdate(float DeltaTime);

	//仅测试
	UFUNCTION()
	void  CollisionFinished();

	UFUNCTION()
	void VisualCubeUpdate(float DeltaTime);

	void StartToPlayAnimationLoop();

	void StopPlayingAnimation();
};
