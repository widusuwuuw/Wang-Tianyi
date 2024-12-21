// 冷静


#include "Actors/WindTunnel.h"
#include "Components\BoxComponent.h"

AWindTunnel::AWindTunnel()
{
	PrimaryActorTick.bCanEverTick = true;

	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	Box->SetupAttachment(RootComponent);
	//绑定回调事件
	Box->OnComponentBeginOverlap.AddDynamic(this, &AWindTunnel::OnOverlapBegin);
	Box->OnComponentEndOverlap.AddDynamic(this, &AWindTunnel::OnOverlapEnd);
}

// Called when the game starts or when spawned
void AWindTunnel::BeginPlay()
{
	Super::BeginPlay();
	
	if (bTemporaryWT)
	{
		//如果生成的是临时风场，设定持续30秒后自动销毁
		InitialLifeSpan = 30.0f;
	}
}

// Called every frame
void AWindTunnel::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (PlayerRef == nullptr)return;

	if (PlayerRef->CurrentMT != EMovementTypes::MT_Gliding)return;

	FVector LocalUpVec = GetActorUpVector() * 10.0f;
	PlayerRef -> AddActorWorldOffset(LocalUpVec);

}

void AWindTunnel::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	PlayerRef = Cast<AZCCharBase>(OtherActor);
}

void AWindTunnel::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	PlayerRef = nullptr;
}

