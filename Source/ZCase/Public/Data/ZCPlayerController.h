// 冷静

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ZCPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class ZCASE_API AZCPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay()override;
};
