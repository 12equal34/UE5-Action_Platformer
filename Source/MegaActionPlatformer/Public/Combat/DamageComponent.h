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
};
