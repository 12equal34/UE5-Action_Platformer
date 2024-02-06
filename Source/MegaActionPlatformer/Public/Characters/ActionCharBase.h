// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperZDCharacter.h"
#include "ActionCharBase.generated.h"

class UActionFactionComponent;
class UHPComponent;
class UFlashComponent;

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
	FORCEINLINE UMaterialInstanceDynamic& GetSpriteMaterialDynamic() const { check(SpriteMaterialDynamic); return *SpriteMaterialDynamic; }
	FORCEINLINE UFlashComponent* GetFlashComponent() const { return FlashComponent; }

	void OnKnockbacked(float KnockbackTime);
	void OnInvinciblized(float InInvisibleTime);

	FORCEINLINE bool IsInvincible() const { return bInvincible; }

protected:
	virtual void OnActionCharBeginOverlap(AActionCharBase& OtherActionChar);

	UFUNCTION()
	virtual void OnAppliedAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

private:
	void OnStartedDying();
	void OnFinishedDying();
	void FinishStop();
	void FinishInvincible();

	UFUNCTION()
	void OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
	void RestoreFallingLateralFriction();

	float InitialFallingLateralFriction;

protected:
	UPROPERTY(Category=Animation,VisibleAnywhere,BlueprintReadOnly)
	bool bHurt;

	UPROPERTY(Category=Animation,VisibleAnywhere,BlueprintReadOnly)
	bool bDead;

	bool bStop;
	bool bInvincible;

	FName HitFlashName;

	

private:
	void PlayDestructionVFX();

	UPROPERTY(Category=Combat,VisibleAnywhere)
	TObjectPtr<UHPComponent> HPComponent;

	UPROPERTY(Category=Combat,VisibleAnywhere)
	TObjectPtr<UActionFactionComponent> FactionComponent;

	UPROPERTY(Category=VFX,EditDefaultsOnly)
	TSubclassOf<class APaperDestructionVFX> DestructionVfxClass;

	UPROPERTY(Category=VFX,VisibleAnywhere)
	TObjectPtr<UFlashComponent> FlashComponent;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> SpriteMaterialDynamic;

	UPROPERTY(Category=Combat,EditDefaultsOnly)
	float DamagedInvisibleTime = 0.2f;

private:
	FTimerHandle FinishStopTimer;
	FTimerHandle FinishInvincibleTimer;
};
