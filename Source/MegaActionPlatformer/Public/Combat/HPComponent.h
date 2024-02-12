// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HPComponent.generated.h"

DECLARE_DELEGATE(FHPBecameZeroSignature);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MEGAACTIONPLATFORMER_API UHPComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHPComponent();

	virtual void InitializeComponent() override;

	void SetMaximumHP(float InMaxHP);
	void SetCurrentHP(float InHP, bool bCanExcessMax = false);

	FORCEINLINE float GetMaxHP() const { return MaxHP; }
	FORCEINLINE float GetHP() const { return HP; }
	FORCEINLINE float GetRatioHP() const { return HP / MaxHP; }

	/** returns the HP after damaged. */
	float Injure(float InDamage);

	/** returns the HP after healed. */
	float Heal(float InHealth, bool bCanExcessMax = false);

	/** returns the HP after healed. */
	float HealRatio(float InRatio, bool bCanExcessMax = false);

	void HealFully();

	bool IsZero() const;
	bool IsLeft() const;
	bool IsExcess() const;
	bool IsFull() const;

	FHPBecameZeroSignature OnHPBecameZero;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly)
	float MaxHP = 10.f;

	UPROPERTY(Transient,VisibleInstanceOnly)
	float HP;
};
