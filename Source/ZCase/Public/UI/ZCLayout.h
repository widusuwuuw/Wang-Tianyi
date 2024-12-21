// 冷静

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Characters/ZCCharBase.h"
#include "ZCLayout.generated.h"


UCLASS()
class ZCASE_API UZCLayout : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent)
	void ConstructDeferred(AZCCharBase* PlayerRef);

	UFUNCTION(BlueprintImplementableEvent)
	void ShowGaugeAnim(bool bShow);
};
