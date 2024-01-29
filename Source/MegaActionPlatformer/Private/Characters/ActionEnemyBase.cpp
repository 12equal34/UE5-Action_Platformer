// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/ActionEnemyBase.h"
#include "Characters/ActionPlayerBase.h"
#include "Factions/ActionFactionComponent.h"
#include "Combat/HPComponent.h"
#include "Combat/DamageComponent.h"

AActionEnemyBase::AActionEnemyBase()
{
	UActionFactionComponent* FactionComp = GetFactionComponent();
	FactionComp->SetFaction(EActionFaction::EAF_Enemy);

	UHPComponent* HPComp = GetHPComponent();
	HPComp->SetMaximumHP(10.f);

	TouchDamageComponent = CreateDefaultSubobject<UDamageComponent>(TEXT("TouchDamage"));
	check(TouchDamageComponent);
}

void AActionEnemyBase::BeginPlay()
{
	Super::BeginPlay();
}

void AActionEnemyBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AActionEnemyBase::OnActionCharBeginOverlap(AActionCharBase& OtherActionChar)
{
	Super::OnActionCharBeginOverlap(OtherActionChar);
}

void AActionEnemyBase::Knockback(AActionCharBase& OtherActionChar)
{
	const bool bOtherIsOnLeftSide = OtherActionChar.GetActorLocation().X < GetActorLocation().X;
	FVector LaunchVelocity;
	LaunchVelocity.X = HorizontalKnockbackPower * (bOtherIsOnLeftSide ? -1.f : 1.f);
	LaunchVelocity.Z = VerticalKnockbackPower;
	OtherActionChar.LaunchCharacter(LaunchVelocity, true, true);
	OtherActionChar.OnKnockbacked(KnockbackTime);
}

void AActionEnemyBase::OnEnemyBeginOverlapPlayer(AActionPlayerBase& PlayerActionChar)
{
	TouchDamageComponent->ApplyDamage(PlayerActionChar);
	if (bCanKnockback)
	{
		Knockback(PlayerActionChar);
	}
}
