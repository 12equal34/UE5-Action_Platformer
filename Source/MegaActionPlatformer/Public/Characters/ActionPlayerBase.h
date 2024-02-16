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

	//~ Begin APawn Interface.
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	//~ End APawn Interface.

	//~ Begin AActor Interface.
	virtual void  TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction);
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor Interface.

	//~ Begin AActionCharBase Interface.
	virtual void OnAppliedAnyDamage(AActor* DamagedActor,float Damage,const UDamageType* DamageType,AController* InstigatedBy,AActor* DamageCauser) override;
	virtual void OnStartedDying();
	virtual void OnFinishedDying();
	virtual void OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor,UPrimitiveComponent* OtherComp,int32 OtherBodyIndex,bool bFromSweep,const FHitResult& SweepResult) override;
	//~ End AActionCharBase Interface.

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

public:
	UFUNCTION(BlueprintPure, Category=Animation)
	bool IsSlidingWall() const;

	UFUNCTION(BlueprintPure, Category=Animation)
	bool IsSliding() const;

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
