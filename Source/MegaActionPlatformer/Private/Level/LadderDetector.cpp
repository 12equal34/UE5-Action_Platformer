#include "Level/LadderDetector.h"
#include "Components/BoxComponent.h"
#include "Characters/ActionPlayerBase.h"

ALadderDetector::ALadderDetector()
{
	PrimaryActorTick.bCanEverTick = false;

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	check(BoxComponent);
	SetRootComponent(BoxComponent);
	BoxComponent->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel3 /*LevelTrigger*/);
	BoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BoxComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	BoxComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	BoxComponent->SetBoxExtent(FVector(25.f, 25.f, 100.f));
}

void ALadderDetector::BeginPlay()
{
	Super::BeginPlay();
	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &ALadderDetector::OnBoxBeginOverlap);
	BoxComponent->OnComponentEndOverlap.AddDynamic(this, &ALadderDetector::OnBoxEndOverlap);
}

void ALadderDetector::OnBoxBeginOverlap_Implementation(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor,UPrimitiveComponent* OtherComp,int32 OtherBodyIndex,bool bFromSweep,const FHitResult& SweepResult)
{
	if (AActionPlayerBase* Player = Cast<AActionPlayerBase>(OtherActor))
	{
		Player->SetOverlappingLadder(this);
	}
}

void ALadderDetector::OnBoxEndOverlap_Implementation(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor,UPrimitiveComponent* OtherComp,int32 OtherBodyIndex)
{
	if (AActionPlayerBase* Player = Cast<AActionPlayerBase>(OtherActor))
	{
		if (Player->GetOverlappingLadder() == this)
		{
			Player->SetOverlappingLadder(nullptr);
		}
	}
}
