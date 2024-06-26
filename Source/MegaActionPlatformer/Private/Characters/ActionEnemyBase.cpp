// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/ActionEnemyBase.h"
#include "Characters/ActionPlayerBase.h"
#include "Components/CapsuleComponent.h"
#include "Factions/ActionFactionComponent.h"
#include "Combat/HPComponent.h"
#include "Combat/DamageComponent.h"

AActionEnemyBase::AActionEnemyBase()
{
	UActionFactionComponent* FactionComp = GetFactionComponent();
	FactionComp->SetFaction(EActionFaction::EAF_Enemy);

	UHPComponent* HPComp = GetHPComponent();
	HPComp->SetMaxHP(10.f);

	TouchDamageComponent = CreateDefaultSubobject<UDamageComponent>(TEXT("TouchDamage"));
	check(TouchDamageComponent);
	TouchDamageComponent->SetCanKnockback(true);
}

void AActionEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &AActionEnemyBase::OnPlayerTouched);
}

void AActionEnemyBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AActionEnemyBase::OnPlayerTouched_Implementation(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor,UPrimitiveComponent* OtherComp,int32 OtherBodyIndex,bool bFromSweep,const FHitResult& SweepResult)
{
	if (AActionPlayerBase* PlayerChar = Cast<AActionPlayerBase>(OtherActor))
	{
		TouchDamageComponent->ApplyDamage(*PlayerChar);
	}
}
