// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperZDCharacter.h"
#include "ActionCharBase.generated.h"

class AController;
class UActionFactionComponent;
class UHPComponent;
class UFlashComponent;

UCLASS()
class MEGAACTIONPLATFORMER_API AActionCharBase : public APaperZDCharacter
{
	GENERATED_BODY()
public:
	AActionCharBase();

	FORCEINLINE UActionFactionComponent*  GetFactionComponent() const { return FactionComponent; }
	FORCEINLINE UHPComponent*             GetHPComponent() const { return HPComponent; }
	FORCEINLINE UMaterialInstanceDynamic& GetSpriteMaterialDynamic() const { check(SpriteMaterialDynamic); return *SpriteMaterialDynamic; }
	FORCEINLINE UFlashComponent*          GetFlashComponent() const { return FlashComponent; }

	FORCEINLINE bool IsInvincible() const { return bInvincible; }

private:
	UPROPERTY(Category=Combat,VisibleAnywhere,BlueprintReadWrite,meta=(AllowPrivateAccess="true"))
	TObjectPtr<UHPComponent> HPComponent;

	UPROPERTY(Category=Combat,VisibleAnywhere)
	TObjectPtr<UActionFactionComponent> FactionComponent;

	UPROPERTY(Category=VFX,EditDefaultsOnly)
	TSubclassOf<class APaperDestructionVFX> DestructionVfxClass;

	UPROPERTY(Category=VFX,VisibleAnywhere)
	TObjectPtr<UFlashComponent> FlashComponent;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> SpriteMaterialDynamic;

public:
	virtual void OnKnockbacked(float KnockbackTime);
	virtual void OnHurt();

	void OnInvinciblized(float InInvisibleTime);

protected:
	UFUNCTION()
	virtual void OnStartedDying(AActor* Causer);

	virtual void OnFinishedDying();

	void RestoreFallingLateralFriction();

	UPROPERTY(Category=Sounds,EditDefaultsOnly)
	TObjectPtr<USoundBase> HitSound;

	UPROPERTY(Category=Animation,VisibleAnywhere,BlueprintReadOnly)
	bool bHurt;

	UPROPERTY(Category=Animation,EditDefaultsOnly)
	float HurtTime = 1.f;

	UPROPERTY(Category=Animation,VisibleAnywhere,BlueprintReadOnly)
	bool bDead;

	UPROPERTY(Category=Combat,VisibleInstanceOnly,BlueprintReadOnly)
	bool bStop;

	UPROPERTY(Category=Combat,EditDefaultsOnly)
	float DyingTime = 0.f;

	UPROPERTY(transient,Category=Combat,VisibleInstanceOnly)
	bool bInvincible;

	UPROPERTY(Category=Combat,EditDefaultsOnly)
	float DamagedInvisibleTime = 0.f;

private:
	void PlayDestructionVFX();
	void FinishStop();
	void FinishHurt();
	void FinishInvincible();

	FTimerHandle FinishStopTimer;
	FTimerHandle FinishInvincibleTimer;
	FTimerHandle FinishHurtTimer;
	FTimerHandle DyingTimer;

	FName HitFlashName;

	float InitialFallingLateralFriction;

public:
	//~ Begin AActor Interface.
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	virtual void FellOutOfWorld(const UDamageType& dmgType) override;
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor Interface.
};
