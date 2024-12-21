// 冷静


#include "Actors/IceActor.h"
#include "Components\SceneComponent.h"
#include "Components\BoxComponent.h"

AIceActor::AIceActor()
{
 
	PrimaryActorTick.bCanEverTick = true;

	//创建SceneComponent作为调整基础
	BaseSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("BaseSceneRoot"));
	BaseSceneRoot->SetupAttachment(RootComponent);

	IceMesh= CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Ice Mesh"));
	IceMesh->SetupAttachment(BaseSceneRoot);
	IceMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	IceMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	IceMesh->SetCollisionObjectType(ECC_WorldStatic);
	IceMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//创建一个盒型碰撞体，避免2个冰柱重叠
	CheckOverlapComp = CreateDefaultSubobject<UBoxComponent>("Check Overlap");
	CheckOverlapComp->SetupAttachment(BaseSceneRoot);
	CheckOverlapComp->SetCollisionObjectType(ECC_WorldStatic);

	//在创建一个盒型碰撞体，用于与其他物理对象碰撞
	SolidBoxComp = CreateDefaultSubobject<UBoxComponent>("Solid Box");
	SolidBoxComp->SetupAttachment(BaseSceneRoot);
	SolidBoxComp->SetCollisionObjectType(ECC_WorldStatic);
	SolidBoxComp->SetCollisionResponseToAllChannels(ECR_Block);
	SolidBoxComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}


void AIceActor::BeginPlay()
{
	Super::BeginPlay();

	if (CollisionCurve)
	{
		FOnTimelineFloat TFCollisionHandle;
		TFCollisionHandle.BindUFunction(this, FName("CollisionUpdate"));

		FOnTimelineEvent TFCollisionFinishedHandle;
		TFCollisionFinishedHandle.BindUFunction(this, FName("CollisionFinished"));

		//将shu'zhoushuzhou2文件和mei'zheneizhen更新的han'sh函数的句柄绑定
		CollisionTimeline.AddInterpFloat(CollisionCurve, TFCollisionHandle);
		CollisionTimeline.SetTimelineFinishedFunc(TFCollisionFinishedHandle);

		//定义起点和终点
		ExtentStart = SolidBoxComp->GetScaledBoxExtent();
		ExtentEnd = FVector(SolidBoxComp->GetScaledBoxExtent().X, SolidBoxComp->GetScaledBoxExtent().Y, 100.0f);

		RelativeStart = SolidBoxComp->GetRelativeLocation();
		RelativeEnd = FVector(SolidBoxComp->GetRelativeLocation().X, SolidBoxComp->GetRelativeLocation().Y, 100.0f);
	}

	if (VisualCubeCurve)
	{
		FOnTimelineFloat TFVCHandle;
		TFVCHandle.BindUFunction(this, FName("VisualCubeUpdate"));

		//将数轴文件和每帧更新函数的句柄绑定
		VisualCubeTimeline.AddInterpFloat(VisualCubeCurve, TFVCHandle);

		//定义起点和终点
		WorldScaleStart = FVector(IceMesh->GetComponentScale().X, IceMesh->GetComponentScale().Y, 0.1f);
		WorldScaleEnd = FVector(IceMesh->GetComponentScale().X, IceMesh->GetComponentScale().Y, 2.0f);
	}
}

void AIceActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//时间轴注册
	CollisionTimeline.TickTimeline(DeltaTime);
	VisualCubeTimeline.TickTimeline(DeltaTime);
}

bool AIceActor::CheckOverlapping()
{
	//生成盒型射线检测（和BoxComponent相同）
	TArray<FHitResult>OutResults;
	const FVector Start = CheckOverlapComp->GetComponentLocation();
	const FVector End = Start;
	const FQuat Rot = (CheckOverlapComp->GetComponentRotation()).Quaternion();
	FCollisionShape CollisionShape = FCollisionShape::MakeBox(CheckOverlapComp->GetScaledBoxExtent());
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	GetWorld()->SweepMultiByChannel(OutResults, Start, End, Rot, ECC_Visibility, CollisionShape,Params);

	//遍历结果，若检测到非Water通道的物体对象，认为不可生成
	for (auto ArrayElem : OutResults)
	{
		ECollisionChannel tempChannel = ArrayElem.GetComponent()->GetCollisionObjectType();
		if (tempChannel == ECC_WorldStatic)
		{
			//有障碍，不能建造
			return false;
		}
	}

	return true;
}

void AIceActor::EnableCollision()
{
	SolidBoxComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	//开始执行时间轴
	CollisionTimeline.PlayFromStart();

	VisualCubeTimeline.SetPlayRate(1.0f);
	VisualCubeTimeline.PlayFromStart();
}

void AIceActor::SpawnIce()
{
	if (!bCanPlace)return;

	AActor*  SpawnedIce=GetWorld()->SpawnActor<AActor>(SpawnClass, GetActorLocation(), GetActorRotation());
	//执行新生成子类的EnableCollision函数
	Cast<AIceActor>(SpawnedIce)->EnableCollision();
}

void AIceActor::CollisionUpdate(float DeltaTime)
{
	//时间轴每帧更新
	FVector NewBoxExtent = FMath::Lerp(ExtentStart, ExtentEnd, DeltaTime);
	SolidBoxComp->SetBoxExtent(NewBoxExtent);

	FVector NewRelativeLocation = FMath::Lerp(RelativeStart, RelativeEnd, DeltaTime);
	SolidBoxComp->SetRelativeLocation(NewRelativeLocation);
}

void AIceActor::CollisionFinished()
{
	//Debug::Print("End of Timeline!");

}

void AIceActor::VisualCubeUpdate(float DeltaTime)
{
	FVector NewWorldScale = FMath::Lerp(WorldScaleStart, WorldScaleStart, DeltaTime);
	IceMesh->SetWorldScale3D(NewWorldScale);
}

void AIceActor::StartToPlayAnimationLoop()
{
	//仅用于首个展示是否重叠的模型
	VisualCubeTimeline.SetLooping(true);
	VisualCubeTimeline.SetPlayRate(0.4f);
	VisualCubeTimeline.PlayFromStart();
}

void AIceActor::StopPlayingAnimation()
{
	VisualCubeTimeline.Stop();
}

