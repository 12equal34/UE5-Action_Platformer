// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ActionFactionComponent.generated.h"

UENUM()
enum class EActionFaction : uint8
{
	EAF_None,
	EAF_Player,
	EAF_Enemy,
	EAF_Neutrality,
	EAF_NPC
};

USTRUCT()
struct FActionFaction
{
	GENERATED_BODY()

	FActionFaction(EActionFaction InType = EActionFaction::EAF_None)
		: Type(InType) 
	{}

	UPROPERTY(EditDefaultsOnly)
	EActionFaction Type;

	bool operator==(const FActionFaction& InFaction) const { return Type == InFaction.Type; }
	bool operator!=(const FActionFaction& InFaction) const { return Type != InFaction.Type; }

	bool IsSameFaction(const FActionFaction& InFaction) const { return *this == InFaction; }
	bool IsDifferentFaction(const FActionFaction& InFaction) const { return !IsSameFaction(InFaction); }
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MEGAACTIONPLATFORMER_API UActionFactionComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	UPROPERTY(Category=Faction,EditDefaultsOnly)
	FActionFaction Faction;

public:	
	UActionFactionComponent();

	bool IsFriendship(const UActionFactionComponent& InFaction) const;
	bool IsHostile(const UActionFactionComponent& InFaction) const;

	void SetFaction(EActionFaction InType);
	void SetFaction(FActionFaction InFaction);
	FORCEINLINE FActionFaction GetFaction() const { return Faction; }
protected:
	virtual void BeginPlay() override;

};
