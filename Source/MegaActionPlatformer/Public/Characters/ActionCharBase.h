// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperZDCharacter.h"
#include "ActionCharBase.generated.h"

class UActionFactionComponent;

UCLASS()
class MEGAACTIONPLATFORMER_API AActionCharBase : public APaperZDCharacter
{
	GENERATED_BODY()

public:
	AActionCharBase();

	//~ Begin AActor Interface.
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor Interface.

	FORCEINLINE UActionFactionComponent* GetFactionComponent() const { return FactionComponent; }
	
private:
	void PlayDestructionVFX();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UActionFactionComponent> FactionComponent;

	UPROPERTY(Category=VFX,EditDefaultsOnly)
	TSubclassOf<class APaperDestructionVFX> DestructionVfxClass;
};
