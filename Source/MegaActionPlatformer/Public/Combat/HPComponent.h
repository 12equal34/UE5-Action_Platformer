// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Delegates/Delegate.h"
#include "HPComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHPChangeSignature, AActor*, Instigator);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHPBecomeZeroSignature, AActor*, Instigator);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MEGAACTIONPLATFORMER_API UHPComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHPComponent();

	virtual void InitializeComponent() override;

	UFUNCTION(BlueprintCallable)
	void SetMaxHP(float InMaxHP, AActor* Instigator = nullptr);

	UFUNCTION(BlueprintCallable)
	void SetHP(float InHP, AActor* Instigator = nullptr, bool bCanExcessMax = false);

	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetMaxHP() const { return MaxHP; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetHP() const { return HP; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetRatioHP() const { return HP / MaxHP; }

	/** returns the HP after damaged. */
	float Injure(float InDamage, AActor* Instigator = nullptr);

	/** returns the HP after healed. */
	float Heal(float InHealth, AActor* Instigator = nullptr, bool bCanExcessMax = false);

	/** returns the HP after healed. */
	float HealRatio(float InRatio, AActor* Instigator = nullptr, bool bCanExcessMax = false);

	void HealFully(AActor* Instigator = nullptr);

	UFUNCTION(BlueprintPure)
	bool IsZero() const;

	UFUNCTION(BlueprintPure)
	bool IsLeft() const;

	UFUNCTION(BlueprintPure)
	bool IsExcess() const;

	UFUNCTION(BlueprintPure)
	bool IsFull() const;

	UPROPERTY(BlueprintAssignable, Category=HP)
	FHPBecomeZeroSignature OnHPBecomeZero;

	UPROPERTY(BlueprintAssignable, Category=HP)
	FHPChangeSignature OnHPChange;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly)
	float MaxHP = 10.f;

	UPROPERTY(Transient,VisibleInstanceOnly)
	float HP = 0.f;
};
