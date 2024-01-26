// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectiles/EnemyProjectileBase.h"
#include "Factions/ActionFactionComponent.h"

AEnemyProjectileBase::AEnemyProjectileBase()
{
	UActionFactionComponent* FactionComp = GetFactionComponent();
	FactionComp->SetFaction(EActionFaction::EAF_Enemy);
}
