// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/FallingDetector.h"
#include "Components/BoxComponent.h"
#include "Characters/ActionCharBase.h"
#include "Engine/DamageEvents.h"

AFallingDetector::AFallingDetector()
{
	PrimaryActorTick.bCanEverTick = false;

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	check(BoxComponent);
	SetRootComponent(BoxComponent);
	BoxComponent->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel3 /*LevelTrigger*/);
	BoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BoxComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	BoxComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	BoxComponent->SetBoxExtent(FVector(300.f, 50.f, 100.f));
}

void AFallingDetector::BeginPlay()
{
	Super::BeginPlay();
	
	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &AFallingDetector::OnCharacterBeginOverlap);
}

void AFallingDetector::OnCharacterBeginOverlap_Implementation(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor,UPrimitiveComponent* OtherComp,int32 OtherBodyIndex,bool bFromSweep,const FHitResult& SweepResult)
{
	if (AActionCharBase* ActionChar = Cast<AActionCharBase>(OtherActor))
	{
		FDamageEvent DamageEvent(DamageTypeClass);
		ActionChar->TakeDamage(FallingDamage, DamageEvent, nullptr, this);
	}
}
