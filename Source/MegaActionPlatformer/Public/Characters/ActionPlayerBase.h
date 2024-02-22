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
class AActionGameModeBase;
class ACameraRestrictor;
class ALadderDetector;

UCLASS()
class MEGAACTIONPLATFORMER_API AActionPlayerBase : public AActionCharBase
{
	GENERATED_BODY()

private:
	/** a cached pointer to a player controller */
	UPROPERTY(Transient)
	TObjectPtr<AActionPlayerController> PlayerController;

	UPROPERTY(Transient)
	TObjectPtr<AActionGameModeBase> ActionGameMode;

	UPROPERTY(Category=Camera,VisibleAnywhere)
	TObjectPtr<class USpringArmComponent> SpringArmComponent;

	UPROPERTY(Category=Camera,VisibleAnywhere)
	TObjectPtr<class UCameraComponent> CameraComponent;

public:
	AActionPlayerBase();

	void SetPlayerController(AActionPlayerController& InPlayerController);
	FORCEINLINE AActionPlayerController* GetPlayerController() const { return PlayerController; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE AActionGameModeBase* GetActionGameMode() const { return ActionGameMode; }

	//~ Begin AActionCharBase Interface.
	virtual void OnKnockbacked(float KnockbackTime) override;
	virtual void OnHurt() override;

	//~ Begin APawn Interface.
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	//~ Begin AActor Interface.
	virtual void  TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction);
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor Interface.
	//~ End APawn Interface.
	
	virtual void OnStartedDying(AActor* Causer) override;
	virtual void OnFinishedDying() override;
	//~ End AActionCharBase Interface.

private:
	void OnIA_Move(const FInputActionValue& Value);
	void OnIA_Jump(const FInputActionInstance& Instance);
	void OnIA_Shoot(const FInputActionInstance& Instance);
	void OnIA_Slide();

	void ResetMoveInputValue();

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
	float MoveInputX;

	/** Player's move input: Down is -1.0, Up is 1.0, and No input is 0. */
	UPROPERTY(Category=Input,VisibleInstanceOnly)
	float MoveInputY;

public:
	UFUNCTION(BlueprintPure, Category=MoveInput)
	FORCEINLINE float GetMoveInputX() const { return MoveInputX; }

	UFUNCTION(BlueprintPure, Category=MoveInput)
	FORCEINLINE float GetMoveInputY() const { return MoveInputY; }

	UFUNCTION(BlueprintPure, Category=Animation)
	bool IsSlidingWall() const;

	UFUNCTION(BlueprintPure, Category=Animation)
	bool IsSliding() const;

	UFUNCTION(BlueprintPure, Category=Animation)
	bool IsClimbing() const;

	void SetOverlappingLadder(ALadderDetector* InLadderDetector);

	FORCEINLINE ALadderDetector* GetOverlappingLadder() const { return OverlappingLadder; }

private:
	void JumpLaunchTo(float HorizontalDirection);
	void JumpFromWall();
	void JumpFromLadder();

	// Wall Sliding
	void TryWallSliding();
	void TransferWallSlidingState();
	void TransferNotWallSlidingState();

	// Floor Sliding
	void SlideFloor();
	void EndSlide();
	void EnableSlide();

	// Ladder Climbing
	void StartClimbLadder();
	void EndClimbLadder();
	void EnableClimb();

	UPROPERTY(Category="Animation|Sliding",VisibleInstanceOnly)
	bool bSlidingWall;

	UPROPERTY(Category="Animation|Sliding",VisibleInstanceOnly)
	bool bSliding;

	UPROPERTY(Category="Animation|Climbing",VisibleInstanceOnly)
	bool bClimbingLadder;

	UPROPERTY(Category="Movement|WallSlide",EditDefaultsOnly,meta=(ClampMin="0"))
	float WallTraceLength = 40.f;

	UPROPERTY(Category="Movement|WallSlide",EditDefaultsOnly,meta=(ClampMin="0"))
	float WallSlidingMaxVelocity = 50.f;

	UPROPERTY(Category="Movement|WallSlide",EditDefaultsOnly,meta=(ClampMin="0"))
	float WallSlidingMinVelocity = 20.f;

	UPROPERTY(Category="Movement|Falling",EditDefaultsOnly,meta=(ClampMin="0"))
	float RestoreFallingLateralFrictionTime = 0.25f;

	UPROPERTY(Category="Movement|Launch",EditDefaultsOnly)
	float JumpVerticalLaunch = 1500.f;

	UPROPERTY(Category="Movement|Launch",EditDefaultsOnly)
	float JumpHorizontalLaunch = 800.f;

	UPROPERTY(Category="Movement|FloorSlide",EditDefaultsOnly,meta=(ClampMin="0"))
	float SlidingForce = 2000.f;

	UPROPERTY(Category="Movement|FloorSlide",EditDefaultsOnly,meta=(ClampMin="0"))
	float SlideDuration = 0.3f;

	UPROPERTY(Category="Movement|FloorSlide",EditDefaultsOnly,meta=(ClampMin="0.1"))
	float DelayAfterSliding = 0.3f;

	UPROPERTY(Category="Movement|FloorSlide",EditDefaultsOnly)
	TObjectPtr<UCurveFloat> SlidingStrengthOverTime;

	UPROPERTY(Category="Movement|FloorSlide",EditDefaultsOnly)
	bool bEnableGravityForSliding;

	UPROPERTY(Category="Movement|Climbing",EditDefaultsOnly,meta=(ClampMin="0.1"))
	float DelayAfterClimbing = 0.3f;

	FTimerHandle RestoreFallingLateralFrictionTimer;
	FTimerHandle EndSlideTimer;
	FTimerHandle EnableSlideTimer;
	FTimerHandle EnableClimbTimer;

	bool bCanSliding = true;
	bool bWallIsOnRight;
	bool bDelayingToClimb = false;

	UPROPERTY(Transient,Category="Movement|Climbing",VisibleInstanceOnly)
	TObjectPtr<ALadderDetector> OverlappingLadder;

public:
	UFUNCTION(BlueprintPure, Category=Animation)
	bool IsShooting() const;

protected:
	void Shoot(const TSubclassOf<APlayerProjectileBase>& InProjectileClass);
	void EndShoot();
	void StartChargeShotEnergy();
	void EndChargeShotEnergy();
	void RestoreShotEnergy();

private:
	UPROPERTY(Category="Combat|Muzzle",VisibleAnywhere)
	TObjectPtr<USceneComponent> Muzzle;

	UPROPERTY(Category="Combat|Muzzle",EditDefaultsOnly)
	FVector MuzzleLocation;

	UPROPERTY(Category="Combat|Muzzle",EditDefaultsOnly)
	FVector MuzzleLocationForSlidingWall;

	UPROPERTY(Category="Combat|Muzzle",EditDefaultsOnly)
	FVector MuzzleLocationForClimbingLadder;

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

	FName ChargeFlashName;

public:
	void SpawnCamera(const FTransform& SpawnTransform, float InLifeSpan);
	void FadeOutCamera();
	void FadeInCamera();
	
	void SetOverlappingCameraRestrictor(ACameraRestrictor* InCameraRestrictor);

	void UpdateCameraPosition(float DeltaTime);

private:
	UPROPERTY(Category="Camera|FadeInOut",EditAnywhere)
	FLinearColor CameraFadeColor = FLinearColor::Black;

	UPROPERTY(Category="Camera|FadeInOut",EditAnywhere)
	float FadeOutDuration = 1.f;

	UPROPERTY(Category="Camera|FadeInOut",EditAnywhere)
	float FadeInDuration = 1.f;

	UPROPERTY(Category="Camera",EditAnywhere)
	FVector CameraOffset = FVector(0.f, 0.f, 200.f);

	/** If a speed <= 0.f, then immediately move to the target pos. */
	UPROPERTY(Category="Camera",EditAnywhere)
	float CameraInterpSpeed = 3.f;

	UPROPERTY(Transient)
	TObjectPtr<ACameraRestrictor> OverlappingCameraRestrictor;

private:
	UPROPERTY(Category=Sounds,EditDefaultsOnly)
	TObjectPtr<USoundBase> SlidingSound;

	UPROPERTY(Category=Sounds,EditDefaultsOnly)
	TObjectPtr<USoundBase> JumpingSound;

	UPROPERTY(Category=Sounds,EditDefaultsOnly)
	TObjectPtr<USoundBase> ChargingSound;
};
