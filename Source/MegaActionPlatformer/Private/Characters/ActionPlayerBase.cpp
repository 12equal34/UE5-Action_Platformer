// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/ActionPlayerBase.h"
#include "Characters/LogPlayer.h"
#include "Characters/ActionEnemyBase.h"
#include "Controllers/ActionPlayerController.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraActor.h"
#include "PaperFlipbookComponent.h"
#include "PaperZDAnimInstance.h"
#include "Components/SceneComponent.h"
#include "Components/CapsuleComponent.h"

#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/RootMotionSource.h"
#include "GameMode/ActionGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"

#include "Level/CameraRestrictor.h"
#include "Level/LadderDetector.h"

#include "EnhancedInputComponent.h"

#include "Projectiles/PlayerProjectileBase.h"
#include "Factions/ActionFactionComponent.h"
#include "Combat/HPComponent.h"
#include "VFX/FlashComponent.h"

#include "Curves/CurveLinearColor.h"
#include "Curves/CurveFloat.h"

DEFINE_LOG_CATEGORY(LogPlayerInput);

#pragma warning(disable: 26813)

AActionPlayerBase::AActionPlayerBase()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	JumpMaxHoldTime = 0.4f;

	DyingTime = 2.f;
	DamagedInvisibleTime = 1.f;

	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	CapsuleComp->SetCapsuleHalfHeight(60.f);
	CapsuleComp->SetCapsuleRadius(24.f);

	UCharacterMovementComponent* MovementComp = GetCharacterMovement();
	MovementComp->AirControl = 1.f;
	MovementComp->FallingLateralFriction = 50.f;
	MovementComp->BrakingDecelerationFlying = 10000.f;

	UPaperFlipbookComponent* MySprite = GetSprite();
	MySprite->SetRelativeLocation(FVector(0.f, -0.1f, 13.9f));

	UActionFactionComponent* FactionComp = GetFactionComponent();
	FactionComp->SetFaction(EActionFaction::EAF_Player);

	UHPComponent* HPComp = GetHPComponent();
	HPComp->SetMaxHP(10.f);

	/**
	* Create Default Subobjects.
	*/
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
	check(SpringArmComponent);
	SpringArmComponent->SetupAttachment(GetRootComponent());
	SpringArmComponent->SetUsingAbsoluteLocation(true);
	SpringArmComponent->SetUsingAbsoluteRotation(true);
	SpringArmComponent->SetWorldRotation(FRotator(0.f, -90.f, 0.f));
	SpringArmComponent->TargetArmLength = 500.f;
	SpringArmComponent->bDoCollisionTest = false;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	check(CameraComponent);
	CameraComponent->SetupAttachment(SpringArmComponent);
	CameraComponent->SetProjectionMode(ECameraProjectionMode::Orthographic);
	CameraComponent->SetOrthoWidth(2048.f);

	/**
	* Setup muzzle locations to fit various animations.
	*/
	Muzzle = CreateDefaultSubobject<USceneComponent>(TEXT("Muzzle"));
	check(Muzzle);
	Muzzle->SetupAttachment(GetRootComponent());
	MuzzleLocation                  = FVector(53.f, 0.f, 8.f);
	MuzzleLocationForSlidingWall    = FVector(-77.f, 0.f, 4.f);
	MuzzleLocationForClimbingLadder = FVector(69.f, 0.f, 28.f);
	Muzzle->SetRelativeLocation(MuzzleLocation);

	/**
	* Prepare default Input Assets.
	*/
	static ConstructorHelpers::FObjectFinder<UInputAction>         IA_Move_Ref(TEXT("/Game/MegaActionPlatformer/Input/Actions/IA_Move.IA_Move"));
	static ConstructorHelpers::FObjectFinder<UInputAction>         IA_Jump_Ref(TEXT("/Game/MegaActionPlatformer/Input/Actions/IA_Jump.IA_Jump"));
	static ConstructorHelpers::FObjectFinder<UInputAction>         IA_Shoot_Ref(TEXT("/Game/MegaActionPlatformer/Input/Actions/IA_Shoot.IA_Shoot"));
	static ConstructorHelpers::FObjectFinder<UInputAction>         IA_Slide_Ref(TEXT("/Game/MegaActionPlatformer/Input/Actions/IA_Slide.IA_Slide"));
	check(IA_Move_Ref.Succeeded());
	check(IA_Jump_Ref.Succeeded());
	check(IA_Shoot_Ref.Succeeded());
	check(IA_Slide_Ref.Succeeded());

	/** Set default input assets. */
	IA_Move    = IA_Move_Ref.Object;
	IA_Jump    = IA_Jump_Ref.Object;
	IA_Shoot   = IA_Shoot_Ref.Object;
	IA_Slide   = IA_Slide_Ref.Object;

	/** 
	* Prepare curves for Flash VFXs. 
	*/ 
	static ConstructorHelpers::FObjectFinder<UCurveLinearColor> ChargeFlashColorCurveRef(TEXT("/Game/MegaActionPlatformer/Curves/C_ChargeFlash_Color.C_ChargeFlash_Color"));
	static ConstructorHelpers::FObjectFinder<UCurveFloat>       ChargeFlashPowerFloatCurveRef(TEXT("/Game/MegaActionPlatformer/Curves/C_ChargeFlash_Float.C_ChargeFlash_Float"));
	static ConstructorHelpers::FObjectFinder<UCurveFloat>       SlidingStrengthOverTimeRef(TEXT("/Game/MegaActionPlatformer/Curves/C_PlayerSlide_Float.C_PlayerSlide_Float"));
	check(ChargeFlashColorCurveRef.Succeeded());
	check(ChargeFlashPowerFloatCurveRef.Succeeded());
	check(SlidingStrengthOverTimeRef.Succeeded());

	SlidingStrengthOverTime = SlidingStrengthOverTimeRef.Object;

	/** Add a ChargeFlash Info. */
	UFlashComponent* FlashComp = GetFlashComponent();
	FFlashInfo ChargeFlashInfo;
	ChargeFlashInfo.PlayPurpose = EFlashInfoPlayPurpose::EFIPP_WantsFixPlayTime;
	ChargeFlashInfo.bLooping = true;
	ChargeFlashInfo.MinCurvePos = 0.f;
	ChargeFlashInfo.MaxCurvePos = 1.f;
	ChargeFlashInfo.WantedPlayTime = 0.3f;
	ChargeFlashInfo.MaterialColorParamName      = TEXT("FlashColor");
	ChargeFlashInfo.MaterialFlashPowerParamName = TEXT("FlashPower");
	ChargeFlashInfo.FlashColorCurve      = ChargeFlashColorCurveRef.Object;
	ChargeFlashInfo.FlashPowerFloatCurve = ChargeFlashPowerFloatCurveRef.Object;
	ChargeFlashName = TEXT("ChargeFlash");
	FlashComp->AddFlashInfo(ChargeFlashName, MoveTemp(ChargeFlashInfo));
}

void AActionPlayerBase::SetPlayerController(AActionPlayerController& InPlayerController)
{
	PlayerController = &InPlayerController;
}

bool AActionPlayerBase::IsShooting() const
{
	return bShooting;
}

bool AActionPlayerBase::IsSlidingWall() const
{
	return bSlidingWall;
}

bool AActionPlayerBase::IsSliding() const
{
	return bSliding;
}

bool AActionPlayerBase::IsClimbing() const
{
	return bClimbingLadder;
}

void AActionPlayerBase::SetOverlappingLadder(ALadderDetector* InLadderDetector)
{
	OverlappingLadder = InLadderDetector;
}

void AActionPlayerBase::TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);

	if (MoveInputX != 0.f)
	{
		TryWallSliding();
	}

	UpdateCameraPosition(DeltaTime);
}

void AActionPlayerBase::UpdateCameraPosition(float DeltaTime)
{
	// Interlerp the camera position from current to target.
	const FVector Current = SpringArmComponent->GetComponentLocation();
	FVector Target;
	if (OverlappingCameraRestrictor)
	{
		Target = OverlappingCameraRestrictor->GetCamreaPosition();
	}
	else
	{
		Target = GetActorLocation() + CameraOffset;
	}
	FVector CameraPosition = FMath::VInterpTo(Current, Target, DeltaTime, CameraInterpSpeed);
	SpringArmComponent->SetWorldLocation(CameraPosition);
}

float AActionPlayerBase::TakeDamage(float Damage,FDamageEvent const& DamageEvent,AController* EventInstigator,AActor* DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	return ActualDamage;
}

void AActionPlayerBase::OnKnockbacked(float KnockbackTime)
{
	Super::OnKnockbacked(KnockbackTime);

	if (bSliding)
	{
		EndSlide();
	}
	
	if (bSlidingWall)
	{
		TransferNotWallSlidingState();
	}

	if (bClimbingLadder)
	{
		EndClimbLadder();
	}
}

void AActionPlayerBase::OnHurt()
{
	Super::OnHurt();
}

void AActionPlayerBase::TryWallSliding()
{
	// Always try sliding when player is falling.
	const float FallingVelocity = GetVelocity().Z;
	if (FallingVelocity > -WallSlidingMinVelocity)
	{
		TransferNotWallSlidingState();
		return;
	}

	// And needs to input keeping.
	const float ToWallIfExists = GetActorForwardVector().X;
	const bool bWallAndInputValueAreSameDirection = 0.f < MoveInputX * ToWallIfExists;
	if (!bWallAndInputValueAreSameDirection) 
	{
		TransferNotWallSlidingState();
		return;
	}

	UWorld* const World = GetWorld();
	check(World);

	// Is there a wall?
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldStatic); // We assumes any ECC_WorldStatic is Wall Type.
	FVector Start = GetActorLocation();
	FVector End   = Start + GetActorForwardVector() * WallTraceLength;
	FHitResult HitResult;
	const bool bThereIsWall = World->LineTraceSingleByObjectType(HitResult,Start,End,ObjectQueryParams);
	if (!bThereIsWall)
	{
		TransferNotWallSlidingState();
		return;
	}
	bWallIsOnRight = ToWallIfExists > 0.f;

	// Slide the wall.
	UCharacterMovementComponent* MovementComp = GetCharacterMovement();
	MovementComp->Velocity.Z = FMath::Max(FallingVelocity, -WallSlidingMaxVelocity);
	MovementComp->UpdateComponentVelocity();
	TransferWallSlidingState();
}

void AActionPlayerBase::SlideFloor()
{
	// Sliding have a invicible ability for SlideDuration.
	OnInvinciblized(SlideDuration);

	// Play Sound
	if (SlidingSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SlidingSound, GetActorLocation());
	}

	// Create a root motion source
	TSharedPtr<FRootMotionSource_ConstantForce> SlidingRootMotionSource = MakeShared<FRootMotionSource_ConstantForce>();
	SlidingRootMotionSource->InstanceName = FName("SlidingConstantForce");
	SlidingRootMotionSource->AccumulateMode = ERootMotionAccumulateMode::Override;
	SlidingRootMotionSource->Priority = 5;
	SlidingRootMotionSource->Force = GetActorForwardVector() * SlidingForce;
	SlidingRootMotionSource->Duration = SlideDuration;
	SlidingRootMotionSource->StrengthOverTime = SlidingStrengthOverTime;
	SlidingRootMotionSource->FinishVelocityParams.Mode = ERootMotionFinishVelocityMode::SetVelocity;
	SlidingRootMotionSource->FinishVelocityParams.SetVelocity = FVector::Zero();

	// Apply a Sliding root motion.
	if (bEnableGravityForSliding)
	{
		SlidingRootMotionSource->Settings.SetFlag(ERootMotionSourceSettingsFlags::IgnoreZAccumulate);
	}
	GetCharacterMovement()->ApplyRootMotionSource(SlidingRootMotionSource);
}

void AActionPlayerBase::JumpLaunchTo(float HorizontalDirection)
{
	FVector LaunchVelocity = FVector(JumpHorizontalLaunch * HorizontalDirection, 0.f, JumpVerticalLaunch);
	GetCharacterMovement()->FallingLateralFriction = 0.f;
	GetWorldTimerManager().SetTimer(RestoreFallingLateralFrictionTimer,this,&AActionPlayerBase::RestoreFallingLateralFriction,
									RestoreFallingLateralFrictionTime,false);
	LaunchCharacter(LaunchVelocity,true,true);
}

void AActionPlayerBase::JumpFromWall()
{
	float OppositeToWall = bWallIsOnRight ? -1.f : 1.f;
	JumpLaunchTo(OppositeToWall);
}

void AActionPlayerBase::JumpFromLadder()
{
	EndClimbLadder();

	float InputDirection = MoveInputX;
	JumpLaunchTo(InputDirection);
}

void AActionPlayerBase::TransferWallSlidingState()
{
	if (!bSlidingWall)
	{
		bSlidingWall = true;
		Muzzle->SetRelativeLocation(MuzzleLocationForSlidingWall);
		Muzzle->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));
	}
}

void AActionPlayerBase::TransferNotWallSlidingState()
{
	if (bSlidingWall)
	{
		bSlidingWall = false;
		Muzzle->SetRelativeLocation(MuzzleLocation);
		Muzzle->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
	}
}

void AActionPlayerBase::EndSlide()
{
	bSliding = false;
}

void AActionPlayerBase::EnableSlide()
{
	bCanSliding = true;
}

void AActionPlayerBase::StartClimbLadder()
{
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);

	FVector NewLocation;
	NewLocation.X = OverlappingLadder->GetActorLocation().X;
	NewLocation.Y = GetActorLocation().Y;
	NewLocation.Z = GetActorLocation().Z;
	SetActorLocation(NewLocation);
	GetCharacterMovement()->Velocity = FVector::ZeroVector;

	// For animation
	bClimbingLadder = true;
	if (UPaperZDAnimInstance* PaperAnimInstance = GetAnimInstance())
	{
		PaperAnimInstance->JumpToNode(TEXT("JumpClimb"));
	}

	// For adjusting muzzle
	Muzzle->SetRelativeLocation(MuzzleLocationForClimbingLadder);
}

void AActionPlayerBase::EndClimbLadder()
{
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);

	// For animation
	bClimbingLadder = false;

	// For adjusting muzzle
	Muzzle->SetRelativeLocation(MuzzleLocation);

	// For delay
	bDelayingToClimb = true;
	GetWorldTimerManager().SetTimer(EnableClimbTimer, this, &AActionPlayerBase::EnableClimb, DelayAfterClimbing, false);
}

void AActionPlayerBase::EnableClimb()
{
	bDelayingToClimb = false;
}

void AActionPlayerBase::BeginPlay()
{
	Super::BeginPlay();

	SpringArmComponent->SetWorldLocation(GetActorLocation() + CameraOffset);

	// makes the restoring shot energy timers buffer.
	ShotEnergy = MaxShotEnergy;
	check(MaxShotEnergy > 0);
	check(MaxShotEnergy <= 32/*size limit*/);
	RestoringShotEnergyTimers = MakeUnique<TCircularBuffer<FTimerHandle>>(MaxShotEnergy, FTimerHandle{});
	check(RestoringShotEnergyTimers.IsValid());
	RestoreShotEnergyIndex = RestoringShotEnergyTimers->Capacity() - 1;

	// caches this game mode.
	ActionGameMode = GetWorld()->GetAuthGameMode<AActionGameModeBase>();
	checkf(ActionGameMode,TEXT("The game mode isn't inherited from AActionGameModeBase or isn't set."));
}

void AActionPlayerBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AActionPlayerBase::Shoot(const TSubclassOf<APlayerProjectileBase>& InProjectileClass)
{
	// For animation.
	bShooting = true;
	GetWorldTimerManager().SetTimer(EndShootTimer,this,&AActionPlayerBase::EndShoot,ShootingTime,false);

	if (InProjectileClass.Get() == nullptr)
	{
		UE_LOG(LogPlayer,Warning, TEXT("Failed to shoot. The player have not the projectile class."));
		return;
	}

	if (ShotEnergy <= 0)
	{
		UE_LOG(LogPlayer,Display, TEXT("Failed to shoot. The player have no shot energy."));
		return;
	}

	// Spawn the projectile owned by this.
	FActorSpawnParameters Params;
	Params.bNoFail = true;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.Owner = this;
	Params.Instigator = this;
	APlayerProjectileBase* Projectile = GetWorld()->SpawnActor<APlayerProjectileBase>(
		InProjectileClass.Get(),
		Muzzle->GetComponentTransform(),
		Params);
	UE_LOG(LogPlayer,Display, TEXT("Shoot! : %s is spawned by %s"), *Projectile->GetName(), *GetName());
	
	// Decrease a shot energy and set a timer for restoring the energy.
	check(RestoringShotEnergyTimers.IsValid());
	TCircularBuffer<FTimerHandle>& Timers = *RestoringShotEnergyTimers.Get();
	ShotEnergy--;
	UE_LOG(LogPlayer,Display, TEXT("Shot energy : %d"), ShotEnergy);
	RestoreShotEnergyIndex = Timers.GetNextIndex(RestoreShotEnergyIndex);
	GetWorldTimerManager().SetTimer(Timers[RestoreShotEnergyIndex],this,&AActionPlayerBase::RestoreShotEnergy,ShotEnergyRestoreTime,false);
}

void AActionPlayerBase::EndShoot()
{
	if (bShooting)
	{
		bShooting = false;
	}

	if (bCharging)
	{
		EndChargeShotEnergy();
	}
}

void AActionPlayerBase::StartChargeShotEnergy()
{
	GetFlashComponent()->PlayFlashFromStart(ChargeFlashName);

	// Play a charging sound loopingly.
	if (ChargingSound)
	{
		UAudioComponent* Audio = UGameplayStatics::SpawnSoundAttached(ChargingSound, GetSprite());
		if (!Audio)
		{
			UE_LOG(LogTemp, Display, TEXT("Not spawned sound attached to sprite"));
		}
	}
}

void AActionPlayerBase::EndChargeShotEnergy()
{
	UFlashComponent* FlashComp = GetFlashComponent();
	if (FlashComp->CurrentFlashIs(ChargeFlashName))
	{
		FlashComp->FinishFlash();
	}
	bCharging = false;

	// Stop playing the current charging sound.
	if (ChargingSound)
	{
		if (UAudioComponent* Audio = Cast<UAudioComponent>(GetSprite()->GetChildComponent(0)))
		{
			Audio->Stop();
			Audio->DestroyComponent();
		}
	}
}

void AActionPlayerBase::RestoreShotEnergy()
{
	if (ShotEnergy >= MaxShotEnergy) return;

	ShotEnergy++;
	UE_LOG(LogPlayer,Display, TEXT("The player's shot energy is restored. (now shot energy: %d)"), ShotEnergy);
}

void AActionPlayerBase::OnStartedDying(AActor* Causer)
{
	Super::OnStartedDying(Causer);

	const FTransform& CameraTransform = CameraComponent->GetComponentTransform();
	const float CameraLifeSpan = ActionGameMode->GetPlayerRespawnTime() + DyingTime + 0.1f;
	SpawnCamera(CameraTransform, CameraLifeSpan);
	FadeOutCamera();
}

void AActionPlayerBase::OnFinishedDying()
{
	check(PlayerController);

	ActionGameMode->OnPlayerLoses(*PlayerController);

	Super::OnFinishedDying();
}

void AActionPlayerBase::SpawnCamera(const FTransform& SpawnTransform,float InLifeSpan)
{
	ACameraActor* Camera = GetWorld()->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), SpawnTransform);
	Camera->GetCameraComponent()->SetAspectRatio(CameraComponent->AspectRatio);
	Camera->GetCameraComponent()->SetFieldOfView(CameraComponent->FieldOfView);
	Camera->GetCameraComponent()->bConstrainAspectRatio = false;
	Camera->SetLifeSpan(InLifeSpan);
	PlayerController->PlayerCameraManager->SetViewTarget(Camera);
}

void AActionPlayerBase::FadeOutCamera()
{
	PlayerController->PlayerCameraManager->StartCameraFade(0.f, 1.f, FadeOutDuration, CameraFadeColor, false, true);
}

void AActionPlayerBase::FadeInCamera()
{
	PlayerController->PlayerCameraManager->StartCameraFade(1.f, 0.f, FadeInDuration, CameraFadeColor, false, true);
}

void AActionPlayerBase::SetOverlappingCameraRestrictor(ACameraRestrictor* InCameraRestrictor)
{
	OverlappingCameraRestrictor = InCameraRestrictor;
}

//-----------------------------------------------------------------------------
// Inputs
//-----------------------------------------------------------------------------

void AActionPlayerBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	checkf(EnhancedInput, TEXT("The PlayerInputComponent can NOT cast a UEnhancedInputComponent."));

	checkf(IA_Move, TEXT("%s is NOT set."), *IA_Move.GetName());
	checkf(IA_Jump, TEXT("%s is NOT set."), *IA_Jump.GetName());
	checkf(IA_Shoot,TEXT("%s is NOT set."), *IA_Shoot.GetName());

	EnhancedInput->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AActionPlayerBase::OnIA_Move);
	EnhancedInput->BindAction(IA_Move, ETriggerEvent::Completed, this, &AActionPlayerBase::ResetMoveInputValue);

	EnhancedInput->BindAction(IA_Jump, ETriggerEvent::Started,  this, &AActionPlayerBase::OnIA_Jump);
	EnhancedInput->BindAction(IA_Jump, ETriggerEvent::Ongoing,  this, &AActionPlayerBase::OnIA_Jump);
	EnhancedInput->BindAction(IA_Jump, ETriggerEvent::Triggered, this, &AActionPlayerBase::OnIA_Jump);
	EnhancedInput->BindAction(IA_Jump, ETriggerEvent::Completed, this, &AActionPlayerBase::OnIA_Jump);
	EnhancedInput->BindAction(IA_Jump, ETriggerEvent::Canceled,  this, &AActionPlayerBase::OnIA_Jump);

	EnhancedInput->BindAction(IA_Shoot,ETriggerEvent::Started,   this, &AActionPlayerBase::OnIA_Shoot);
	EnhancedInput->BindAction(IA_Shoot,ETriggerEvent::Ongoing,   this, &AActionPlayerBase::OnIA_Shoot);
	EnhancedInput->BindAction(IA_Shoot,ETriggerEvent::Triggered, this, &AActionPlayerBase::OnIA_Shoot);
	EnhancedInput->BindAction(IA_Shoot,ETriggerEvent::Completed, this, &AActionPlayerBase::OnIA_Shoot);
	EnhancedInput->BindAction(IA_Shoot,ETriggerEvent::Canceled,  this, &AActionPlayerBase::OnIA_Shoot);

	EnhancedInput->BindAction(IA_Slide,ETriggerEvent::Triggered, this, &AActionPlayerBase::OnIA_Slide);
}

void AActionPlayerBase::ResetMoveInputValue()
{
	UE_LOG( LogPlayerInput,Display, TEXT("Move: Completed"));
	MoveInputX = 0.f;
	MoveInputY = 0.f;
}

void AActionPlayerBase::OnIA_Move(const FInputActionValue& Value)
{
	if (bHurt)
	{
		bHurt = false;
	}

	if (OverlappingLadder == nullptr && bClimbingLadder)
	{
		EndClimbLadder();
	}

	if (bDead || bStop || bSliding)
	{
		return;
	}

	const FVector2D Input = Value.Get<FVector2D>();
	MoveInputX = Input.X;
	MoveInputY = Input.Y;

	// Try climbing a ladder if a player isn't climbing when the vertical input value isn't zero.
	if (OverlappingLadder && !bClimbingLadder && MoveInputY != 0.f)
	{
		if (!bDelayingToClimb)
		{
			StartClimbLadder();
		}
	}

	// Set the horizontal input to the player's control rotation.
	if (MoveInputX == 0.f)
	{
		UE_LOG( LogPlayerInput,Display, TEXT("Move: %s"), (bClimbingLadder ? TEXT("Climbing"):TEXT("Walking")));
	}
	else
	{
		PlayerController->SetControlRotation(FRotator(0.f, (MoveInputX > 0.f ? 0.f : 180.f), 0.f));
		UE_LOG( LogPlayerInput,Display, TEXT("Move: %s"), (MoveInputX > 0.f ? TEXT("Look Right"):TEXT("Look Left")) );
	}
	
	// Add a proper movement input for either Climbing or Walking.
	if (bClimbingLadder)
	{
		AddMovementInput(FVector::ZAxisVector, MoveInputY);
		if (MoveInputY != 0.f)
		{
			UE_LOG( LogPlayerInput,Display, TEXT("Move: %s"), (MoveInputY > 0.f ? TEXT("Climbing Up"):TEXT("Climbing Down")) );
		}
	}
	else
	{
		AddMovementInput(FVector::XAxisVector, MoveInputX);
		UE_LOG( LogPlayerInput,Display, TEXT("Move: %s"), (MoveInputX > 0.f ? TEXT("Walking Right"):TEXT("Walking Left")) );
	}
}

void AActionPlayerBase::OnIA_Jump(const FInputActionInstance& Instance)
{
	if (bDead)
	{
		return;
	}
	if (bStop || bSliding) return;

	if (bHurt)
	{
		bHurt = false;
	}

	ETriggerEvent TriggerEvent = Instance.GetTriggerEvent();
	if (TriggerEvent == ETriggerEvent::Started)
	{
		UE_LOG(LogPlayerInput, Display, TEXT("Jump: Started"));

		if (bSlidingWall)
		{
			JumpFromWall();
		}
		else if (bClimbingLadder)
		{
			JumpFromLadder();
		}

		if (JumpingSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, JumpingSound, GetActorLocation());
		}
	}
	else if (TriggerEvent == ETriggerEvent::Ongoing)
	{
		UE_LOG(LogPlayerInput, Display, TEXT("Jump: Ongoing"));

		if (!bSlidingWall)
		{
			Jump();
		}
	}
	else if (TriggerEvent == ETriggerEvent::Completed)
	{
		UE_LOG(LogPlayerInput, Display, TEXT("Jump: Completed"));
	}
	else if (TriggerEvent == ETriggerEvent::Canceled)
	{
		UE_LOG(LogPlayerInput, Display, TEXT("Jump: Canceled"));
	}
	else if (TriggerEvent == ETriggerEvent::Triggered)
	{
		UE_LOG(LogPlayerInput, Display, TEXT("Jump: Triggered"));

		StopJumping();
	}
}

void AActionPlayerBase::OnIA_Shoot(const FInputActionInstance& Instance)
{
	if (bStop || bDead)
	{
		return;
	}

	const ETriggerEvent TriggerEvent = Instance.GetTriggerEvent();

	static bool bFailedForShooting;
	if (TriggerEvent == ETriggerEvent::Started)
	{
		UE_LOG(LogPlayerInput, Display, TEXT("Shoot: Started"));

		bFailedForShooting = false;
		GetWorldTimerManager().ClearTimer(EndShootTimer);
	}
	else if (TriggerEvent == ETriggerEvent::Ongoing)
	{
		if (bFailedForShooting)
		{
			UE_LOG(LogPlayerInput, Display, TEXT("Shoot: Ongoing (Failed for shooting)"));
			return;
		}
		
		if (bHurt)
		{
			UE_LOG(LogPlayerInput, Display, TEXT("Shoot: Ongoing (Hurt for shooting)"));
			bFailedForShooting = true;
			EndShoot();
			return;
		}
		
		if (bClimbingLadder && MoveInputY != 0.f)
		{
			bShooting = false;
		}
		else
		{
			bShooting = true;
		}

		const float ElapsedTime = Instance.GetElapsedTime();
		if (ElapsedTime < HalfChargeTime)
		{
			UE_LOG(LogPlayerInput, Display, TEXT("Shoot: Ongoing (Not charged)"));
		}
		else if (ElapsedTime < FullChargeTime)
		{
			UE_LOG(LogPlayerInput, Display, TEXT("Shoot: Ongoing (Half charged)"));
		}
		else
		{
			UE_LOG(LogPlayerInput, Display, TEXT("Shoot: Ongoing (Full charged)"));
		}

		if (!bCharging && ElapsedTime >= HalfChargeTime)
		{
			bCharging = true;
			StartChargeShotEnergy();
		}
	}
	else if (TriggerEvent == ETriggerEvent::Completed)
	{
		if (!bFailedForShooting && bSliding)
		{
			UE_LOG(LogPlayerInput, Display, TEXT("Shoot: Completed, The player can't shoot for sliding, so shooting is failed."));
			bFailedForShooting = true;
		}

		if (bFailedForShooting)
		{
			UE_LOG(LogPlayerInput, Display, TEXT("Shoot: Completed, The play shooting is failed."));
			EndShoot();
			return;
		}

		TSubclassOf<APlayerProjectileBase> ProjectileClass = NormalProjectileClass;
		const float ElapsedTime = Instance.GetElapsedTime();
		if (ElapsedTime > FullChargeTime)
		{
			UE_LOG(LogPlayerInput, Display, TEXT("Shoot: Completed (Half shot)"));
			ProjectileClass = FullChargedProjectileClass;
		}
		else if (ElapsedTime > HalfChargeTime)
		{
			UE_LOG(LogPlayerInput, Display, TEXT("Shoot: Completed (Full shot)"));
			ProjectileClass = HalfChargedProjectileClass;
		}
		else
		{
			UE_LOG(LogPlayerInput, Display, TEXT("Shoot: Completed (Normal shot)"));
		}
		if (bCharging)
		{
			EndChargeShotEnergy();
		}
		Shoot(ProjectileClass);
	}
}

void AActionPlayerBase::OnIA_Slide()
{
	if (bDead || !bCanSliding || bClimbingLadder || GetCharacterMovement()->IsFalling())
	{
		return;
	}

	// Set sliding timers for sliding cooltime and animation.
	bCanSliding = false;
	bSliding = true;
	const float SlideDelay = SlideDuration + DelayAfterSliding;
	check(SlideDelay >= SlideDuration);
	GetWorldTimerManager().SetTimer(EnableSlideTimer, this, &AActionPlayerBase::EnableSlide, SlideDelay, false);
	GetWorldTimerManager().SetTimer(EndSlideTimer,    this, &AActionPlayerBase::EndSlide, SlideDuration, false);

	SlideFloor();
}
