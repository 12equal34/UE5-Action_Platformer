// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectiles/PlayerProjectileBase.h"
#include "Factions/ActionFactionComponent.h"

APlayerProjectileBase::APlayerProjectileBase()
{
	UActionFactionComponent* FactionComp = GetFactionComponent();
	FactionComp->SetFaction(EActionFaction::EAF_Player);
}
