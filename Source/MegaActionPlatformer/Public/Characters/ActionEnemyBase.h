// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/ActionCharBase.h"
#include "ActionEnemyBase.generated.h"

class AActionPlayerBase;

UCLASS()
class MEGAACTIONPLATFORMER_API AActionEnemyBase : public AActionCharBase
{
	GENERATED_BODY()
public:
	AActionEnemyBase();

	/** Called by the player overlapped */
	virtual void OnEnemyBeginOverlapPlayer(AActionPlayerBase& PlayerActionChar);

protected:
	//~ Begin AActor Interface.
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor Interface.

	virtual void OnActionCharBeginOverlap(AActionCharBase& OtherActionChar) override;

	void Knockback(AActionCharBase& OtherActionChar);

private:
	UPROPERTY(Category=Combat,EditDefaultsOnly)
	float HorizontalKnockbackPower = 500.f;

	UPROPERTY(Category=Combat,EditDefaultsOnly)
	float VerticalKnockbackPower = 500.f;

	UPROPERTY(Category=Combat,EditDefaultsOnly)
	float KnockbackTime = 0.3f;

	UPROPERTY(Category=Combat,EditDefaultsOnly)
	bool bCanKnockback = true;

	UPROPERTY(Category=Combat,VisibleAnywhere)
	TObjectPtr<class UDamageComponent> TouchDamageComponent;
};
