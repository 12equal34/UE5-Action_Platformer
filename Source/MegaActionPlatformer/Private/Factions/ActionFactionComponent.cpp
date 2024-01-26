// Fill out your copyright notice in the Description page of Project Settings.


#include "Factions/ActionFactionComponent.h"

UActionFactionComponent::UActionFactionComponent()
{
	
}

void UActionFactionComponent::SetFaction(EActionFaction InFaction)
{
	Faction = InFaction;
}

void UActionFactionComponent::BeginPlay()
{
	Super::BeginPlay();

}

bool UActionFactionComponent::IsFriendship(const UActionFactionComponent& InFaction) const
{
	return Faction.IsSameFaction(InFaction.Faction);
}

bool UActionFactionComponent::IsHostile(const UActionFactionComponent& InFaction) const
{
	return Faction.IsDifferentFaction(InFaction.Faction);
}