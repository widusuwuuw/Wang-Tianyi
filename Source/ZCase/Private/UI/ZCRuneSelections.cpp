// 冷静


#include "UI/ZCRuneSelections.h"

void UZCRuneSelections::SelectedRuneType(TEnumAsByte<ERunes> RuneType)
{
	if (PlayerRef)
	{
		PlayerRef->ActiveRune = RuneType;
	}
}
