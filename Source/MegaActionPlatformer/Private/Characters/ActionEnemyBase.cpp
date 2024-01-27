// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/ActionEnemyBase.h"
#include "Factions/ActionFactionComponent.h"
#include "Combat/HPComponent.h"

AActionEnemyBase::AActionEnemyBase()
{
	UActionFactionComponent* FactionComp = GetFactionComponent();
	FactionComp->SetFaction(EActionFaction::EAF_Enemy);

	UHPComponent* HPComp = GetHPComponent();
	HPComp->SetMaximumHP(10.f);
}

void AActionEnemyBase::BeginPlay()
{
	Super::BeginPlay();

}

void AActionEnemyBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}
