
#include "Level/Checkpoint.h"
#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/PlayerStart.h"
#include "Characters/ActionPlayerBase.h"
#include "Controllers/ActionPlayerController.h"
#include "GameMode/ActionGameModeBase.h"

ACheckpoint::ACheckpoint()
{
	RootTransform = CreateDefaultSubobject<USceneComponent>(TEXT("RootTransform"));
	check(RootTransform);
	SetRootComponent(RootTransform);

 	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	check(BoxComponent);
	BoxComponent->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel3 /*LevelTrigger*/);
	BoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BoxComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	BoxComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	BoxComponent->SetBoxExtent(FVector(50.f,50.f,500.f));
	BoxComponent->SetupAttachment(RootTransform);
	BoxComponent->SetRelativeLocation(FVector(0.f,0.f,BoxComponent->GetUnscaledBoxExtent().Z));

	PlayerStartSpawnTransform = CreateDefaultSubobject<USceneComponent>(TEXT("PlayerStartSpawnTransform"));
	check(PlayerStartSpawnTransform);
	PlayerStartSpawnTransform->SetupAttachment(RootTransform);
	PlayerStartSpawnTransform->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
}

void ACheckpoint::BeginPlay()
{
	Super::BeginPlay();

	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &ACheckpoint::OnBoxBeginOverlap);
}

void ACheckpoint::OnBoxBeginOverlap_Implementation(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor,UPrimitiveComponent* OtherComp,int32 OtherBodyIndex,bool bFromSweep,const FHitResult& SweepResult)
{
	if (!bCheck && Cast<AActionPlayerBase>(OtherActor))
	{
		bCheck = true;

		UWorld* World = GetWorld();
		APlayerStart* NewPlayerStart = World->SpawnActor<APlayerStart>(APlayerStart::StaticClass(),PlayerStartSpawnTransform->GetComponentTransform());
		check(NewPlayerStart);

		AActionGameModeBase* GameMode = World->GetAuthGameMode<AActionGameModeBase>();
		if (GameMode)
		{
			GameMode->AddPlayerStart(NewPlayerStart);
		}
	}
}
