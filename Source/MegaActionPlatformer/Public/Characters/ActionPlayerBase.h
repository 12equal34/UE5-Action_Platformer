// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/ActionCharBase.h"
#include "ActionPlayerBase.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPlayerInput, Display, All);

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
struct FInputActionInstance;

class AActionEnemyBase;
class APlayerProjectileBase;

UCLASS()
class MEGAACTIONPLATFORMER_API AActionPlayerBase : public AActionCharBase
{
	GENERATED_BODY()
public:
	AActionPlayerBase();

	//~ Begin APawn Interface.
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	//~ End APawn Interface.

	UFUNCTION(BlueprintPure, Category=Animation)
	bool IsShooting() const;


protected:
	//~ Begin AActor Interface.
	virtual void BeginPlay() override;
	//~ End AActor Interface.

	void Shoot(const TSubclassOf<APlayerProjectileBase>& InProjectileClass);
	void EndShoot();
	void ChargeShotEnergy();
	void StartChargeShotEnergy();
	void EndChargeShotEnergy();
	bool bCharging;

	void RestoreShotEnergy();

	void OnPlayerBeginOverlapEnemy(AActionEnemyBase& EnemyActionChar);

	virtual void OnActionCharBeginOverlap(AActionCharBase& OtherActionChar) override;

private:
	/** a cached pointer to a player controller */
	UPROPERTY(Transient)
	TObjectPtr<APlayerController> PlayerController;

	UPROPERTY(Category=Camera,VisibleAnywhere)
	TObjectPtr<class USpringArmComponent> SpringArmComponent;

	UPROPERTY(Category=Camera,VisibleAnywhere)
	TObjectPtr<class UCameraComponent> CameraComponent;

	FName ChargeFlashName;

private:
	void OnIA_Move(const FInputActionValue& Value);
	void OnIA_Jump(const FInputActionInstance& Instance);
	void OnIA_Shoot(const FInputActionInstance& Instance);

	void AddDefaultInputMappingContext();

	UPROPERTY(Category=Input,EditDefaultsOnly)
	TObjectPtr<UInputMappingContext> DefaultIMC;

	UPROPERTY(Category=Input,EditDefaultsOnly)
	TObjectPtr<UInputAction> IA_Move;

	UPROPERTY(Category=Input,EditDefaultsOnly)
	TObjectPtr<UInputAction> IA_Jump;

	UPROPERTY(Category=Input,EditDefaultsOnly)
	TObjectPtr<UInputAction> IA_Shoot;

	UPROPERTY(Category=Combat,EditDefaultsOnly)
	TSubclassOf<APlayerProjectileBase> NormalProjectileClass;

	UPROPERTY(Category=Combat,EditDefaultsOnly)
	TSubclassOf<APlayerProjectileBase> FullChargedProjectileClass;

	UPROPERTY(Category=Combat,EditDefaultsOnly)
	TSubclassOf<APlayerProjectileBase> HalfChargedProjectileClass;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> Muzzle;

	FTimerHandle ShootingTimer;

	TUniquePtr<TCircularBuffer<FTimerHandle>> RestoringShotEnergyTimers;

	uint32 RestoreShotEnergyIndex;

	UPROPERTY(Category=Combat,EditDefaultsOnly)
	float ShootingTime = 0.5f;
	
	UPROPERTY(Category=Combat,EditDefaultsOnly)
	float ShotEnergyRestoreTime = 0.5f;

	bool bShooting;

	UPROPERTY(Category=Combat,EditDefaultsOnly)
	int32 MaxShotEnergy = 3;

	UPROPERTY(Category=Combat,VisibleInstanceOnly,Transient)
	int32 ShotEnergy;

	UPROPERTY(Category=Combat,EditDefaultsOnly)
	float FullChargeTime = 2.f;

	/** ChargeFlash begins since this time. */
	UPROPERTY(Category=Combat,EditDefaultsOnly)
	float HalfChargeTime = 1.f;
};
