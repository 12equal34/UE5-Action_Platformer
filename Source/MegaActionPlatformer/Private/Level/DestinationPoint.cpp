// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/DestinationPoint.h"
#include "Components/BoxComponent.h"
#include "Characters/ActionPlayerBase.h"
#include "GameMode/ActionGameModeBase.h"

ADestinationPoint::ADestinationPoint()
{
	PrimaryActorTick.bCanEverTick = false;

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	check(BoxComponent);
	SetRootComponent(BoxComponent);
	BoxComponent->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel3 /*LevelTrigger*/);
	BoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BoxComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	BoxComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	BoxComponent->SetBoxExtent(FVector(50.f, 50.f, 200.f));
}

void ADestinationPoint::BeginPlay()
{
	Super::BeginPlay();

	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &ADestinationPoint::OnPlayerBeginOverlap);
}

void ADestinationPoint::OnPlayerBeginOverlap_Implementation(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor,UPrimitiveComponent* OtherComp,int32 OtherBodyIndex,bool bFromSweep,const FHitResult& SweepResult)
{
	if (bArriveCheck) return;

	if (AActionPlayerBase* PlayerChar = Cast<AActionPlayerBase>(OtherActor))
	{
		AActionGameModeBase* GameMode = PlayerChar->GetActionGameMode();
		check(GameMode);

		bArriveCheck = true;
		GameMode->WinPlayer(PlayerChar->GetPlayerController());
	}
}
