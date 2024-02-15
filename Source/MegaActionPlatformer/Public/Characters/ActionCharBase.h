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

protected:
	UPROPERTY(Category=Animation,VisibleAnywhere,BlueprintReadOnly)
	bool bHurt;

	UPROPERTY(Category=Animation,VisibleAnywhere,BlueprintReadOnly)
	bool bDead;

	FTimerHandle DyingTimer;

	UPROPERTY(Category=Combat,EditDefaultsOnly)
	float DyingTime = 0.f;

	UPROPERTY(transient,Category=Combat,VisibleInstanceOnly)
	bool bStop;

	FTimerHandle FinishStopTimer;

	UPROPERTY(transient,Category=Combat,VisibleInstanceOnly)
	bool bInvincible;

	FTimerHandle FinishInvincibleTimer;

	UPROPERTY(Category=Combat,EditDefaultsOnly)
	float DamagedInvisibleTime = 0.2f;

	UPROPERTY(transient)
	float InitialFallingLateralFriction;

	FName HitFlashName;

public:
	AActionCharBase();

	FORCEINLINE UActionFactionComponent*  GetFactionComponent() const { return FactionComponent; }
	FORCEINLINE UHPComponent*             GetHPComponent() const { return HPComponent; }
	FORCEINLINE UMaterialInstanceDynamic& GetSpriteMaterialDynamic() const { check(SpriteMaterialDynamic); return *SpriteMaterialDynamic; }
	FORCEINLINE UFlashComponent*          GetFlashComponent() const { return FlashComponent; }

	FORCEINLINE bool IsInvincible() const { return bInvincible; }

protected:
	UFUNCTION()
	virtual void OnAppliedAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION()
	virtual void OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:
	void OnKnockbacked(float KnockbackTime);
	void OnInvinciblized(float InInvisibleTime);

	//~ Begin AActor Interface.
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	virtual void FellOutOfWorld(const UDamageType& dmgType) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor Interface.

	UFUNCTION()
	virtual void OnStartedDying();
	virtual void OnFinishedDying();

	void RestoreFallingLateralFriction();

private:
	void PlayDestructionVFX();
	void FinishStop();
	void FinishInvincible();
};
