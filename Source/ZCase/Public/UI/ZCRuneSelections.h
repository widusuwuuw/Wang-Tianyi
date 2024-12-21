// 冷静

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Characters/ZCCharBase.h"
#include "ZCRuneSelections.generated.h"

class AZCCharBase;

UCLASS()
class ZCASE_API UZCRuneSelections : public UUserWidget
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditAnywhere,BlueprintReadWrite)
	AZCCharBase* PlayerRef;

	UFUNCTION(BlueprintCallable)
	void SelectedRuneType(TEnumAsByte<ERunes> RuneType);
};
