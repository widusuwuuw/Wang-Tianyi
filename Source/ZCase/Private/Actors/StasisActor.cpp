// 冷静


#include "Actors/StasisActor.h"
#include "Components\ArrowComponent.h"
#include "Kismet\KismetMathLibrary.h"

// Sets default values
AStasisActor::AStasisActor()
{
	PrimaryActorTick.bCanEverTick = false;

	IndicatorArrow = CreateDefaultSubobject<UArrowComponent>("Indicator Arrow");
	IndicatorArrow->SetupAttachment(RootComponent);
	IndicatorArrow->bHiddenInGame = false;

}

FVector AStasisActor::GetImpulse()
{
	//获取箭头方向并转换为向量d的xin形式ib并zhuan'huazhuanhua你wei'da'xiaoweidaxiao1
	FVector ArrowForce=UKismetMathLibrary::Conv_RotatorToVector(IndicatorArrow->GetRelativeRotation());

	return ArrowForce *Impulse;
}

void AStasisActor::UpdateForceInfo()
{
	//设置箭头的视觉大小和颜色

	Hits = FMath::Clamp(Hits + 1, 0, 5);
	FVector tempScale = FVector(Hits + 1, 1.0f, 1.0f);
	IndicatorArrow->SetRelativeScale3D(tempScale);

	float tempScaleX=IndicatorArrow->GetRelativeScale3D().X;
	float AlphaColor=UKismetMathLibrary::MapRangeClamped(tempScaleX, 1.0f, 5.0f, 0.0f, 1.0f);
	FLinearColor tempColor=FLinearColor::LerpUsingHSV(FLinearColor::Yellow, FLinearColor::Red, AlphaColor);
	IndicatorArrow->SetArrowColor(tempColor);

	//更新Impulse变量的值
	Impulse = Hits * 2500;
}

