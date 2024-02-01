// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/ActionPlayerBase.h"
#include "Characters/LogPlayer.h"
#include "Characters/ActionEnemyBase.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "Projectiles/PlayerProjectileBase.h"
#include "Components/SceneComponent.h"
#include "PaperFlipbookComponent.h"
#include "Factions/ActionFactionComponent.h"
#include "Combat/HPComponent.h"
#include "VFX/FlashComponent.h"
#include "Curves/CurveLinearColor.h"
#include "Curves/CurveFloat.h"

DEFINE_LOG_CATEGORY(LogPlayerInput);

#pragma warning(disable: 26813)

#define UE_CONVERT_TO_TEXT(a) #a

AActionPlayerBase::AActionPlayerBase()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	JumpMaxHoldTime = 0.4f;

	UCharacterMovementComponent* MovementComp = GetCharacterMovement();
	MovementComp->AirControl = 1.f;
	MovementComp->FallingLateralFriction = 50.f;

	UPaperFlipbookComponent* MySprite = GetSprite();
	MySprite->SetRelativeLocation(FVector(-5.f, -0.1f, 13.9f));

	UActionFactionComponent* FactionComp = GetFactionComponent();
	FactionComp->SetFaction(EActionFaction::EAF_Player);

	UHPComponent* HPComp = GetHPComponent();
	HPComp->SetMaximumHP(100.f);

	/**
	* Create Default Subobjects.
	*/
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
	check(SpringArmComponent);
	SpringArmComponent->SetupAttachment(GetRootComponent());
	SpringArmComponent->SetUsingAbsoluteRotation(true);
	SpringArmComponent->SetWorldRotation(FRotator(-15.f, -90.f, 0.f));
	SpringArmComponent->TargetArmLength = 500.f;
	SpringArmComponent->bDoCollisionTest = false;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	check(CameraComponent);
	CameraComponent->SetupAttachment(SpringArmComponent);

	Muzzle = CreateDefaultSubobject<USceneComponent>(TEXT("Muzzle"));
	check(Muzzle);
	Muzzle->SetupAttachment(GetRootComponent());
	MuzzleLocation               = FVector(53.f, 0.f, 8.f);
	MuzzleLocationForSlidingWall = FVector(-77.f, 0.f, 4.f);
	Muzzle->SetRelativeLocation(MuzzleLocation);

	/**
	* Prepare default Input Assets.
	*/
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> DefaultIMC_Ref(TEXT("/Game/MegaActionPlatformer/Input/IMC_Action.IMC_Action"));
	static ConstructorHelpers::FObjectFinder<UInputAction>         IA_Move_Ref(TEXT("/Game/MegaActionPlatformer/Input/Actions/IA_Move.IA_Move"));
	static ConstructorHelpers::FObjectFinder<UInputAction>         IA_Jump_Ref(TEXT("/Game/MegaActionPlatformer/Input/Actions/IA_Jump.IA_Jump"));
	static ConstructorHelpers::FObjectFinder<UInputAction>         IA_Shoot_Ref(TEXT("/Game/MegaActionPlatformer/Input/Actions/IA_Shoot.IA_Shoot"));
	check(DefaultIMC_Ref.Succeeded());
	check(IA_Move_Ref.Succeeded());
	check(IA_Jump_Ref.Succeeded());
	check(IA_Shoot_Ref.Succeeded());

	/** Set default input assets. */
	DefaultIMC = DefaultIMC_Ref.Object;
	IA_Move    = IA_Move_Ref.Object;
	IA_Jump    = IA_Jump_Ref.Object;
	IA_Shoot   = IA_Shoot_Ref.Object;

	/** 
	* Prepare curves for Flash VFXs. 
	*/ 
	static ConstructorHelpers::FObjectFinder<UCurveLinearColor> ChargeFlashColorCurveRef(TEXT("/Game/MegaActionPlatformer/Curves/C_ChargeFlash_Color.C_ChargeFlash_Color"));
	static ConstructorHelpers::FObjectFinder<UCurveFloat>       ChargeFlashPowerFloatCurveRef(TEXT("/Game/MegaActionPlatformer/Curves/C_ChargeFlash_Float.C_ChargeFlash_Float"));
	check(ChargeFlashColorCurveRef.Succeeded());
	check(ChargeFlashPowerFloatCurveRef.Succeeded());

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

void AActionPlayerBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (ensure(EnhancedInput))
	{
		checkf(IA_Move, TEXT("%s is NOT set."), *IA_Move.GetName());
		checkf(IA_Jump, TEXT("%s is NOT set."), *IA_Jump.GetName());
		checkf(IA_Shoot,TEXT("%s is NOT set."), *IA_Shoot.GetName());

		EnhancedInput->BindAction(IA_Move, ETriggerEvent::None,      this, &AActionPlayerBase::OnIA_Move);
		EnhancedInput->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AActionPlayerBase::OnIA_Move);

		EnhancedInput->BindAction(IA_Jump, ETriggerEvent::Triggered, this, &AActionPlayerBase::OnIA_Jump);
		EnhancedInput->BindAction(IA_Jump, ETriggerEvent::Completed, this, &AActionPlayerBase::OnIA_Jump);
		EnhancedInput->BindAction(IA_Jump, ETriggerEvent::Canceled,  this, &AActionPlayerBase::OnIA_Jump);

		EnhancedInput->BindAction(IA_Shoot,ETriggerEvent::Started,   this, &AActionPlayerBase::OnIA_Shoot);
		EnhancedInput->BindAction(IA_Shoot,ETriggerEvent::Ongoing,   this, &AActionPlayerBase::OnIA_Shoot);
		EnhancedInput->BindAction(IA_Shoot,ETriggerEvent::Triggered, this, &AActionPlayerBase::OnIA_Shoot);
		EnhancedInput->BindAction(IA_Shoot,ETriggerEvent::Completed, this, &AActionPlayerBase::OnIA_Shoot);
		EnhancedInput->BindAction(IA_Shoot,ETriggerEvent::Canceled,  this, &AActionPlayerBase::OnIA_Shoot);
	}
}

bool AActionPlayerBase::IsShooting() const
{
	return bShooting;
}

bool AActionPlayerBase::IsSlidingWall() const
{
	return bSlidingWall;
}

void AActionPlayerBase::TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);

	TryWallSliding();
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
	const bool bWallAndInputValueAreSameDirection = 0.f < MoveInputValue * GetActorForwardVector().X;
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

	// Slide the wall.
	UCharacterMovementComponent* MovementComp = GetCharacterMovement();
	MovementComp->Velocity.Z = FMath::Max(FallingVelocity, -WallSlidingMaxVelocity);
	MovementComp->UpdateComponentVelocity();
	TransferWallSlidingState();
}

void AActionPlayerBase::TransferWallSlidingState()
{
	if (!bSlidingWall)
	{
		bSlidingWall = true;
		Muzzle->SetRelativeLocation(MuzzleLocationForSlidingWall);
		Muzzle->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));
		GetSprite()->SetRelativeLocation(FVector(6.5f, -0.1f, 13.9f));
	}
}

void AActionPlayerBase::TransferNotWallSlidingState()
{
	if (bSlidingWall)
	{
		bSlidingWall = false;
		Muzzle->SetRelativeLocation(MuzzleLocation);
		Muzzle->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
		GetSprite()->SetRelativeLocation(FVector(-5.f, -0.1f, 13.9f));
	}
}

void AActionPlayerBase::BeginPlay()
{
	Super::BeginPlay();

	PlayerController = CastChecked<APlayerController>(GetController());

	AddDefaultInputMappingContext();

	// makes the restoring shot energy timers buffer.
	ShotEnergy = MaxShotEnergy;
	check(MaxShotEnergy > 0);
	check(MaxShotEnergy <= 32/*size limit*/);
	RestoringShotEnergyTimers = MakeUnique<TCircularBuffer<FTimerHandle>>(MaxShotEnergy, FTimerHandle{});
	check(RestoringShotEnergyTimers.IsValid());
	RestoreShotEnergyIndex = RestoringShotEnergyTimers->Capacity() - 1;
}

void AActionPlayerBase::Shoot(const TSubclassOf<APlayerProjectileBase>& InProjectileClass)
{
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

	UWorld* World = GetWorld();
	check(World != nullptr);

	FTimerManager& TM = World->GetTimerManager();

	// spawns the projectile owned by this.
	FActorSpawnParameters Params;
	Params.bNoFail = true;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.Owner = this;
	Params.Instigator = this;
	APlayerProjectileBase* Projectile = World->SpawnActor<APlayerProjectileBase>(
		InProjectileClass.Get(),
		Muzzle->GetComponentTransform(),
		Params);
	UE_LOG(LogPlayer,Display, TEXT("Shoot! : %s is spawned by %s"), *Projectile->GetName(), *GetName());
	
	// decreases a shot energy and sets a timer for restoring the energy.
	check(RestoringShotEnergyTimers.IsValid());
	TCircularBuffer<FTimerHandle>& Timers = *RestoringShotEnergyTimers.Get();
	ShotEnergy--;
	UE_LOG(LogPlayer,Display, TEXT("Shot energy : %d"), ShotEnergy);
	RestoreShotEnergyIndex = Timers.GetNextIndex(RestoreShotEnergyIndex);
	TM.SetTimer(Timers[RestoreShotEnergyIndex],this,&AActionPlayerBase::RestoreShotEnergy,ShotEnergyRestoreTime,false);
}

void AActionPlayerBase::EndShoot()
{
	bShooting = false;
}

void AActionPlayerBase::StartChargeShotEnergy()
{
	GetFlashComponent()->PlayFlashFromStart(ChargeFlashName);
	bCharging = true;
}

void AActionPlayerBase::EndChargeShotEnergy()
{
	UFlashComponent* FlashComp = GetFlashComponent();
	if (FlashComp->CurrentFlashIs(ChargeFlashName))
	{
		FlashComp->FinishFlash();
	}
	bCharging = false;
}

void AActionPlayerBase::RestoreShotEnergy()
{
	if (ShotEnergy >= MaxShotEnergy) return;

	ShotEnergy++;
	UE_LOG(LogPlayer,Display, TEXT("The player's shot energy is restored. (now shot energy: %d)"), ShotEnergy);
}

void AActionPlayerBase::OnPlayerBeginOverlapEnemy(AActionEnemyBase& EnemyActionChar)
{
	checkNoRecursion();
	EnemyActionChar.OnEnemyBeginOverlapPlayer(*this);
}

void AActionPlayerBase::OnActionCharBeginOverlap(AActionCharBase& OtherActionChar)
{
	Super::OnActionCharBeginOverlap(OtherActionChar);

	if (AActionEnemyBase* EnemyActionChar = Cast<AActionEnemyBase>(&OtherActionChar))
	{
		OnPlayerBeginOverlapEnemy(*EnemyActionChar);
	}
}

void AActionPlayerBase::OnIA_Move(const FInputActionInstance& Instance)
{
	ETriggerEvent TriggerEvent = Instance.GetTriggerEvent();
	if (TriggerEvent == ETriggerEvent::None)
	{
		if (MoveInputValue != 0.f)
		{
			MoveInputValue = 0.f;
		}
		return;
	}

	if (bStop) return;

	MoveInputValue = Instance.GetValue().Get<float>();
	if (MoveInputValue == 0.f)
	{
		UE_LOG( LogPlayerInput,Display, TEXT("Move: Zero") );
		return;
	}

	const bool bRight = MoveInputValue > 0.f;
	AddMovementInput(FVector::XAxisVector, MoveInputValue);
	PlayerController->SetControlRotation(FRotator(0.f, (bRight ? 0.f : 180.f), 0.f));

	UE_LOG( LogPlayerInput,Display, TEXT("Move: %s"), (bRight?TEXT("Right"):TEXT("Left")) );
}

void AActionPlayerBase::OnIA_Jump(const FInputActionInstance& Instance)
{
	if (bStop) return;

	ETriggerEvent TriggerEvent = Instance.GetTriggerEvent();
	if (TriggerEvent == ETriggerEvent::Completed)
	{
		UE_LOG(LogPlayerInput, Display, TEXT("Jump: Completed"));

		StopJumping();
	}
	else if (TriggerEvent == ETriggerEvent::Canceled)
	{
		UE_LOG(LogPlayerInput, Display, TEXT("Jump: Canceled"));
	}
	else if (TriggerEvent == ETriggerEvent::Triggered)
	{
		UE_LOG(LogPlayerInput, Display, TEXT("Jump: Triggered"));

		if (bSlidingWall)
		{
			FVector LaunchVelocity = FVector(-WallJumpHorizontalLaunch * MoveInputValue, 0.f, WallJumpVerticalLaunch);
			GetCharacterMovement()->FallingLateralFriction = 0.f;
			GetWorldTimerManager().SetTimer(WallJumpRestoreFallingLateralFrictionTimer, this, &AActionPlayerBase::RestoreFallingLateralFriction, 
											WallJumpRestoreFallingLateralFrictionTime, false);
			LaunchCharacter(LaunchVelocity, true, true);
		}
		else
		{
			Jump();
		}
	}
}

void AActionPlayerBase::OnIA_Shoot(const FInputActionInstance& Instance)
{
	if (bStop) return;

	ETriggerEvent TriggerEvent = Instance.GetTriggerEvent();
	if (TriggerEvent == ETriggerEvent::Completed)
	{
		UE_LOG(LogPlayerInput, Display, TEXT("Shoot: Completed"));

		if (bCharging)
		{
			EndChargeShotEnergy();

			const float ElapsedTime = Instance.GetElapsedTime();
			if (ElapsedTime < FullChargeTime)
			{
				Shoot(HalfChargedProjectileClass);
			}
			else
			{
				Shoot(FullChargedProjectileClass);
			}
		}
		else
		{
			Shoot(NormalProjectileClass);
		}

		GetWorldTimerManager().SetTimer(ShootingTimer, this, &AActionPlayerBase::EndShoot, ShootingTime, false);
	}
	else if (TriggerEvent == ETriggerEvent::Canceled)
	{
		UE_LOG(LogPlayerInput, Display, TEXT("Shoot: Canceled"));
	}
	else if (TriggerEvent == ETriggerEvent::Triggered)
	{
		UE_LOG(LogPlayerInput, Display, TEXT("Shoot: Triggered"));
	}
	else if (TriggerEvent == ETriggerEvent::Ongoing)
	{
		UE_LOG(LogPlayerInput, Display, TEXT("Shoot: Ongoing"));

		const float ElapsedTime = Instance.GetElapsedTime();
		if (ElapsedTime >= HalfChargeTime && !bCharging)
		{
			StartChargeShotEnergy();
		}
	}
	else if (TriggerEvent == ETriggerEvent::Started)
	{
		UE_LOG(LogPlayerInput, Display, TEXT("Shoot: Started"));

		// Start player character shooting animations.
		bShooting = true;
		GetWorldTimerManager().ClearTimer(ShootingTimer);
	}
}

void AActionPlayerBase::AddDefaultInputMappingContext()
{
	check(PlayerController);
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	check(Subsystem);

	checkf(DefaultIMC, TEXT("%s does NOT setup a property of AActionPlayerBase: " UE_CONVERT_TO_TEXT(DefaultIMC)), *GetName());
	Subsystem->AddMappingContext(DefaultIMC, 0);
}

