// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/ActionPlayerBase.h"
#include "Characters/LogPlayer.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "Projectiles/PlayerProjectileBase.h"
#include "Components/SceneComponent.h"

DEFINE_LOG_CATEGORY(LogActionPlayerInput);

#pragma warning(disable: 26813)

#define UE_CONVERT_TO_TEXT(a) #a

AActionPlayerBase::AActionPlayerBase()
{
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

	static ConstructorHelpers::FObjectFinder<UInputMappingContext> DefaultIMC_Ref(TEXT("/Game/MegaActionPlatformer/Input/IMC_Action.IMC_Action"));
	check(DefaultIMC_Ref.Succeeded());
	DefaultIMC = DefaultIMC_Ref.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> IA_Move_Ref(TEXT("/Game/MegaActionPlatformer/Input/Actions/IA_Move.IA_Move"));
	check(IA_Move_Ref.Succeeded());
	IA_Move = IA_Move_Ref.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> IA_Jump_Ref(TEXT("/Game/MegaActionPlatformer/Input/Actions/IA_Jump.IA_Jump"));
	check(IA_Jump_Ref.Succeeded());
	IA_Jump = IA_Jump_Ref.Object;

	JumpMaxHoldTime = 0.4f;

	static ConstructorHelpers::FObjectFinder<UInputAction> IA_Shoot_Ref(TEXT("/Game/MegaActionPlatformer/Input/Actions/IA_Shoot.IA_Shoot"));
	check(IA_Shoot_Ref.Succeeded());
	IA_Shoot = IA_Shoot_Ref.Object;

	Muzzle = CreateDefaultSubobject<USceneComponent>(TEXT("Muzzle"));
	check(Muzzle);
	Muzzle->SetupAttachment(GetRootComponent());
	Muzzle->SetRelativeLocation(FVector(53.f, 0.f, 8.f));

	UCharacterMovementComponent* CharMovement = GetCharacterMovement();
	CharMovement->AirControl = 1.f;
	CharMovement->FallingLateralFriction = 50.f;
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

		EnhancedInput->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AActionPlayerBase::OnIA_Move);
		EnhancedInput->BindAction(IA_Jump, ETriggerEvent::Triggered, this, &AActionPlayerBase::OnIA_Jump);
		EnhancedInput->BindAction(IA_Jump, ETriggerEvent::Completed, this, &AActionPlayerBase::OnIA_Jump);
		EnhancedInput->BindAction(IA_Jump, ETriggerEvent::Canceled,  this, &AActionPlayerBase::OnIA_Jump);
		EnhancedInput->BindAction(IA_Shoot,ETriggerEvent::Triggered, this, &AActionPlayerBase::OnIA_Shoot);
	}
}

bool AActionPlayerBase::IsShooting() const
{
	return bShooting;
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

void AActionPlayerBase::Shoot()
{
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
		PlayerProjectileClass.Get(),
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

	// for character shooting animations.
	bShooting = true;
	TM.SetTimer(ShootingTimer, this, &AActionPlayerBase::EndShoot, ShootingTime, false);
}

void AActionPlayerBase::EndShoot()
{
	bShooting = false;
}

void AActionPlayerBase::RestoreShotEnergy()
{
	if (ShotEnergy >= MaxShotEnergy) return;

	ShotEnergy++;
	UE_LOG(LogPlayer,Display, TEXT("The player's shot energy is restored. (now shot energy: %d)"), ShotEnergy);
}

void AActionPlayerBase::OnIA_Move(const FInputActionValue& Value)
{
	const float InputScale = Value.Get<float>();
	if (InputScale == 0.f)
	{
		UE_LOG( LogActionPlayerInput,Display, TEXT("Move: Zero") );
		return;
	}

	const bool bRight = InputScale > 0.f;
	AddMovementInput(FVector::XAxisVector, InputScale);
	PlayerController->SetControlRotation(FRotator(0.f, (bRight ? 0.f : 180.f), 0.f));

	UE_LOG( LogActionPlayerInput,Display, TEXT("Move: %s"), (bRight?TEXT("Right"):TEXT("Left")) );
}

void AActionPlayerBase::OnIA_Jump(const FInputActionInstance& Instance)
{
	ETriggerEvent TriggerEvent = Instance.GetTriggerEvent();
	if (TriggerEvent == ETriggerEvent::Completed)
	{
		UE_LOG(LogActionPlayerInput, Display, TEXT("Jump: Completed"));

		StopJumping();
	}
	else if (TriggerEvent == ETriggerEvent::Canceled)
	{
		UE_LOG(LogActionPlayerInput, Display, TEXT("Jump: Canceled"));
	}
	else if (TriggerEvent == ETriggerEvent::Triggered)
	{
		UE_LOG(LogActionPlayerInput, Display, TEXT("Jump: Triggered"));

		Jump();
	}
}

void AActionPlayerBase::OnIA_Shoot(const FInputActionValue& Value)
{
	const bool bPress = Value.Get<bool>();
	UE_LOG(LogActionPlayerInput, Display, TEXT("Shoot: %s"), bPress ? TEXT("Pressed") : TEXT("Released"));
	if (bPress)
	{
		Shoot();
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