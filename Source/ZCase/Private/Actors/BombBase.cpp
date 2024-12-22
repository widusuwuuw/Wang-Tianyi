// 冷静


#include "Actors/BombBase.h"
#include "Kismet/GameplayStatics.h"
#include "Debug/DebugHelper.h"
#include "Field/FieldSystemActor.h" // 需加上模块"FieldSystemEngine"，否则打包会无法通过
#include "PhysicsEngine\RadialForceComponent.h"
#include "Actors/WindTunnel.h"



ABombBase::ABombBase()
{
	PrimaryActorTick.bCanEverTick = false;

	SM = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Bomb Mesh"));
	SM->SetupAttachment(RootComponent);
	SM->SetLinearDamping(0.35f);
	SM->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	SM->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	RFComp = CreateDefaultSubobject<URadialForceComponent>("Radial Force Comp");
	RFComp->SetupAttachment(SM);
	RFComp->Radius = 300.0f;
	RFComp->ImpulseStrength = 3000.0f;
	RFComp->bImpulseVelChange = true;
	RFComp->ForceStrength = 100.0f;
}

void ABombBase::BeginPlay()
{
	Super::BeginPlay();

	// 生成炸弹时的特效
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BombSpawnVFX, GetActorLocation());

}

void ABombBase::Detonate()
{
	// 引爆炸弹时调用
	// 播放爆炸特效
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BombExplosionVFX, SM->GetComponentLocation());

	// 播放声音
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSFX, GetActorLocation());

	// 播放镜头震动效果
	GetWorld()->GetFirstPlayerController()->PlayerCameraManager->PlayWorldCameraShake(
		GetWorld(), ExplosionShake, SM->GetComponentLocation(), 0.0f, 2000.0f, 0.5f, false);

	// 炸碎静态网格体的冲击力
	if (FieldActorClass)
	{
		GetWorld()->SpawnActor<AActor>(FieldActorClass, SM->GetComponentLocation(), FRotator::ZeroRotator);
	}
	RFComp->FireImpulse();

	// 若爆炸时在草地上则生成风场，此处检测材质

	FVector Start = SM->GetComponentLocation();
	FVector End = Start;
	TArray<FHitResult> OutResults;
	FCollisionShape MySphere = FCollisionShape::MakeSphere(200.0f);
	FCollisionQueryParams Params;
	Params.bReturnPhysicalMaterial = true; // 重要
	GetWorld()->SweepMultiByChannel(OutResults, Start, End, FQuat::Identity, ECC_Visibility,
		MySphere, Params);

	bool bSpawnWind = false;
	for (auto ArrayElem : OutResults)
	{

		if (ArrayElem.bBlockingHit && ArrayElem.PhysMaterial != nullptr)
		{
			switch (UGameplayStatics::GetSurfaceType(ArrayElem))
			{
			case SurfaceType1:
				break;
			case SurfaceType2:
				// 发现草地！
				bSpawnWind = true;
				break;
			}
		}
	}

	if (bSpawnWind && WindTunnelClass)
	{
		// 生成的是临时风场，需将风场中的bTemporaryWT改为true
		// C++中用延时生成函数
		// FTransform MakeTransform(Rotation, Location, Scale)
		FVector WTLocation = FVector(SM->GetComponentLocation().X, SM->GetComponentLocation().Y, SM->GetComponentLocation().Z + 800.0f);
		FTransform CustomTransform(FRotator(0, 0, 0), WTLocation, FVector(4, 4, 4));
		AWindTunnel* WT = GetWorld()->SpawnActorDeferred<AWindTunnel>(
			WindTunnelClass, CustomTransform);
		// 初始化bTemporaryWT
		WT->bTemporaryWT = true;
		WT->InitialLifeSpan = 5.0f;
		// 再完全生成该Actor
		WT->FinishSpawning(CustomTransform);
	}

	// 销毁自身
	Destroy();
}



