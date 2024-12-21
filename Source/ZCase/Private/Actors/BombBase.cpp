// 冷静


#include "Actors/BombBase.h"
#include "Kismet/GameplayStatics.h"
#include "Debug/DebugHelper.h"


ABombBase::ABombBase()
{
 	
	PrimaryActorTick.bCanEverTick = false;

	SM = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Bomb Mesh"));
	SM->SetupAttachment(RootComponent);
	SM->SetLinearDamping(0.35f);
}


void ABombBase::BeginPlay()
{
	Super::BeginPlay();
	
	//生成炸弹时的特效
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BombSpawnVFX, GetActorLocation());
}

void ABombBase::Detonate()
{
	//引爆炸弹时调用
	//播放爆炸时的特效
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BombExplosionVFX, SM->GetComponentLocation());

	//播放声音
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSFX, GetActorLocation());

	//播放镜头震动效果
	GetWorld()->GetFirstPlayerController()->PlayerCameraManager->PlayWorldCameraShake(
		GetWorld(), ExplosionShake, SM->GetComponentLocation(), 0.0f, 2000.0f, 0.5f, false);

	//TBD-炸碎静态网格体的冲击力

	//TBD-若爆炸时在草地上则生成风场，此处检测材质



	//销毁自身
	Destroy();
}



