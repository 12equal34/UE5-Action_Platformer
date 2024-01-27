// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperZDCharacter.h"
#include "ActionCharBase.generated.h"

class UActionFactionComponent;
class UHPComponent;

UCLASS()
class MEGAACTIONPLATFORMER_API AActionCharBase : public APaperZDCharacter
{
	GENERATED_BODY()

public:
	AActionCharBase();

	//~ Begin AActor Interface.
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	//~ End AActor Interface.

	FORCEINLINE UActionFactionComponent* GetFactionComponent() const { return FactionComponent; }
	FORCEINLINE UHPComponent* GetHPComponent() const { return HPComponent; }

	UFUNCTION()
	void OnAppliedAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	void OnStartedDying();
	void OnFinishedDying();

private:
	void PlayDestructionVFX();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UActionFactionComponent> FactionComponent;

	UPROPERTY(Category=VFX,EditDefaultsOnly)
	TSubclassOf<class APaperDestructionVFX> DestructionVfxClass;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UHPComponent> HPComponent;

	bool bDead;
};
