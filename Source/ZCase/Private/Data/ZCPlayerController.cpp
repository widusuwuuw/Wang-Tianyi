// 冷静


#include "Data/ZCPlayerController.h"

void AZCPlayerController::BeginPlay()
{
	Super::BeginPlay();

	SetInputMode(FInputModeGameOnly());
}
