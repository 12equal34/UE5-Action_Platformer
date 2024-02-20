// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DamageComponent.generated.h"

class AActionCharBase;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MEGAACTIONPLATFORMER_API UDamageComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UDamageComponent();

	void SetDamage(float InDamage);

	FORCEINLINE float GetDamage() const { return Damage; }

	virtual float ApplyDamage(AActionCharBase& DamagedActionChar);

	void Knockback(AActionCharBase& OtherActionChar);

	void SetCanKnockback(bool bInCanKnockback);

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly)
	float Damage = 1.f;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UDamageType> DamageTypeClass;

	UPROPERTY(Transient)
	TObjectPtr<AActor> DamageCauser;

	UPROPERTY(Transient)
	TObjectPtr<AController> EventInstigator;

	UPROPERTY(Category=Combat,EditDefaultsOnly)
	float HorizontalKnockbackPower = 500.f;

	UPROPERTY(Category=Combat,EditDefaultsOnly)
	float VerticalKnockbackPower = 500.f;

	UPROPERTY(Category=Combat,EditDefaultsOnly)
	float KnockbackTime = 0.3f;

	UPROPERTY(Category=Combat,EditDefaultsOnly)
	bool bCanKnockback = false;
};
