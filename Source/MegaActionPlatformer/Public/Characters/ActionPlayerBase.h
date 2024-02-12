// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/ActionCharBase.h"
#include "ActionPlayerBase.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPlayerInput, Display, All);

class UInputAction;
struct FInputActionValue;
struct FInputActionInstance;

class AActionEnemyBase;
class APlayerProjectileBase;
class AActionPlayerController;

UCLASS()
class MEGAACTIONPLATFORMER_API AActionPlayerBase : public AActionCharBase
{
	GENERATED_BODY()
public:
	AActionPlayerBase();

	//~ Begin APawn Interface.
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	//~ End APawn Interface.

	void SetPlayerController(AActionPlayerController& InPlayerController);

	UFUNCTION(BlueprintPure, Category=Animation)
	bool IsShooting() const;

	UFUNCTION(BlueprintPure, Category=Animation)
	bool IsSlidingWall() const;

	UFUNCTION(BlueprintPure, Category=Animation)
	bool IsSliding() const;

	//~ Begin AActor Interface.
	virtual void TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction);
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor Interface.

	void Shoot(const TSubclassOf<APlayerProjectileBase>& InProjectileClass);
	void EndShoot();
	void StartChargeShotEnergy();
	void EndChargeShotEnergy();
	void RestoreShotEnergy();

	void OnPlayerBeginOverlapEnemy(AActionEnemyBase& EnemyActionChar);

	//~ Begin AActionCharBase Interface.
	virtual void OnActionCharBeginOverlap(AActionCharBase& OtherActionChar) override;
	virtual void OnAppliedAnyDamage(AActor* DamagedActor,float Damage,const UDamageType* DamageType,AController* InstigatedBy,AActor* DamageCauser) override;
	virtual void OnStartedDying();
	virtual void OnFinishedDying();
	//~ End AActionCharBase Interface.

private:
	/** a cached pointer to a player controller */
	UPROPERTY(Transient)
	TObjectPtr<AActionPlayerController> PlayerController;

	UPROPERTY(Category=Camera,VisibleAnywhere)
	TObjectPtr<class USpringArmComponent> SpringArmComponent;

	UPROPERTY(Category=Camera,VisibleAnywhere)
	TObjectPtr<class UCameraComponent> CameraComponent;

	FName ChargeFlashName;

private:
	void OnIA_Move(const FInputActionInstance& Instance);
	void OnIA_Jump(const FInputActionInstance& Instance);
	void OnIA_Shoot(const FInputActionInstance& Instance);
	void OnIA_Slide();

	UPROPERTY(Category=Input,EditDefaultsOnly)
	TObjectPtr<UInputAction> IA_Move;

	UPROPERTY(Category=Input,EditDefaultsOnly)
	TObjectPtr<UInputAction> IA_Jump;

	UPROPERTY(Category=Input,EditDefaultsOnly)
	TObjectPtr<UInputAction> IA_Shoot;

	UPROPERTY(Category=Input,EditDefaultsOnly)
	TObjectPtr<UInputAction> IA_Slide;

	/** Player's move input: Left is -1.0, right is 1.0, and No input is 0. */
	UPROPERTY(Category=Input,VisibleInstanceOnly)
	float MoveInputValue;

private:
	void TryWallSliding();
	void JumpFromWall();
	void TransferWallSlidingState();
	void TransferNotWallSlidingState();

	void SlideFloor();
	void EndSlide();
	void EnableSlide();

	UPROPERTY(Category="Animation|Sliding",VisibleInstanceOnly)
	bool bSlidingWall;

	UPROPERTY(Category="Animation|Sliding",VisibleInstanceOnly)
	bool bSliding;

	UPROPERTY(Category="Movement|WallSlide",EditDefaultsOnly,meta=(ClampMin="0"))
	float WallTraceLength = 40.f;

	UPROPERTY(Category="Movement|WallSlide",EditDefaultsOnly,meta=(ClampMin="0"))
	float WallSlidingMaxVelocity = 50.f;

	UPROPERTY(Category="Movement|WallSlide",EditDefaultsOnly,meta=(ClampMin="0"))
	float WallSlidingMinVelocity = 20.f;

	UPROPERTY(Category="Movement|WallSlide",EditDefaultsOnly,meta=(ClampMin="0"))
	float WallJumpRestoreFallingLateralFrictionTime = 0.25f;

	UPROPERTY(Category="Movement|WallSlide",EditDefaultsOnly)
	float WallJumpVerticalLaunch = 1500.f;

	UPROPERTY(Category="Movement|WallSlide",EditDefaultsOnly)
	float WallJumpHorizontalLaunch = 800.f;

	UPROPERTY(Category="Movement|FloorSlide",EditDefaultsOnly,meta=(ClampMin="0"))
	float SlidingForce = 2000.f;

	UPROPERTY(Category="Movement|FloorSlide",EditDefaultsOnly,meta=(ClampMin="0"))
	float SlideDuration = 0.3f;

	UPROPERTY(Category="Movement|FloorSlide",EditDefaultsOnly,meta=(ClampMin="0"))
	float DelayAfterSliding = 0.3f;

	UPROPERTY(Category="Movement|FloorSlide",EditDefaultsOnly)
	TObjectPtr<UCurveFloat> SlidingStrengthOverTime;

	UPROPERTY(Category="Movement|FloorSlide",EditDefaultsOnly)
	bool bEnableGravityForSliding;

	FTimerHandle WallJumpRestoreFallingLateralFrictionTimer;

	FTimerHandle EndSlideTimer;

	FTimerHandle EnableSlideTimer;

	bool bCanSliding = true;

private:
	UPROPERTY(Category="Combat|Muzzle",VisibleAnywhere)
	TObjectPtr<USceneComponent> Muzzle;

	UPROPERTY(Category="Combat|Muzzle",EditDefaultsOnly)
	FVector MuzzleLocation;

	UPROPERTY(Category="Combat|Muzzle",EditDefaultsOnly)
	FVector MuzzleLocationForSlidingWall;

	UPROPERTY(Category="Combat|Shot",EditDefaultsOnly)
	TSubclassOf<APlayerProjectileBase> NormalProjectileClass;

	UPROPERTY(Category="Combat|Shot",EditDefaultsOnly)
	TSubclassOf<APlayerProjectileBase> FullChargedProjectileClass;

	UPROPERTY(Category="Combat|Shot",EditDefaultsOnly)
	TSubclassOf<APlayerProjectileBase> HalfChargedProjectileClass;

	UPROPERTY(Category="Combat|Shot",EditDefaultsOnly)
	float ShootingTime = 0.5f;
	
	UPROPERTY(Category="Combat|Shot",EditDefaultsOnly)
	float ShotEnergyRestoreTime = 1.f;

	UPROPERTY(Category="Combat|Shot",EditDefaultsOnly)
	bool bIsHurtForShooting;

	UPROPERTY(Category="Combat|Shot",EditDefaultsOnly,meta=(ClampMin="0"))
	int32 MaxShotEnergy = 3;

	UPROPERTY(Category="Combat|Shot",VisibleInstanceOnly,Transient)
	int32 ShotEnergy;

	UPROPERTY(Category="Combat|Shot",EditDefaultsOnly,meta=(ClampMin="0"))
	float FullChargeTime = 2.f;

	/** ChargeFlash begins since this time. */
	UPROPERTY(Category="Combat|Shot",EditDefaultsOnly,meta=(ClampMin="0"))
	float HalfChargeTime = 1.f;

	UPROPERTY(Category="Animation|Shooting",VisibleInstanceOnly)
	bool bShooting;

	UPROPERTY(Category="Animation|Shooting",VisibleInstanceOnly)
	bool bCharging;

	FTimerHandle EndShootTimer;

	TUniquePtr<TCircularBuffer<FTimerHandle>> RestoringShotEnergyTimers;

	uint32 RestoreShotEnergyIndex;
};
