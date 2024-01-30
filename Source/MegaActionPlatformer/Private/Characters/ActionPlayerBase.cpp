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
	UPaperFlipbookComponent* MySprite = GetSprite();
	MySprite->SetRelativeLocation(FVector(-5.f, 0.f, 13.9f));

	UActionFactionComponent* FactionComp = GetFactionComponent();
	FactionComp->SetFaction(EActionFaction::EAF_Player);

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

	UHPComponent* HPComp = GetHPComponent();
	HPComp->SetMaximumHP(100.f);

	/** 
	* Prepare curves for Flash VFXs. 
	*/ 
	static ConstructorHelpers::FObjectFinder<UCurveLinearColor> ChargeFlashColorCurveRef(TEXT("/Game/MegaActionPlatformer/Curves/C_ChargeFlash_Color.C_ChargeFlash_Color"));
	check(ChargeFlashColorCurveRef.Succeeded());

	static ConstructorHelpers::FObjectFinder<UCurveFloat> ChargeFlashPowerFloatCurveRef(TEXT("/Game/MegaActionPlatformer/Curves/C_ChargeFlash_Float.C_ChargeFlash_Float"));
	check(ChargeFlashPowerFloatCurveRef.Succeeded());

	UFlashComponent* FlashComp = GetFlashComponent();

	// Add a ChargeFlash Info.
	FFlashInfo ChargeFlashInfo;
	ChargeFlashInfo.CurveTimeRatio = EFlashCurveTimeRatio::EFCTR_Proportional;
	ChargeFlashInfo.SetTimeLength(0.3f);
	ChargeFlashInfo.MaterialColorParamName = TEXT("FlashColor");
	ChargeFlashInfo.FlashColorCurve = ChargeFlashColorCurveRef.Object;
	ChargeFlashInfo.MaterialFlashPowerParamName = TEXT("FlashPower");
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

		EnhancedInput->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AActionPlayerBase::OnIA_Move);
		EnhancedInput->BindAction(IA_Jump, ETriggerEvent::Triggered, this, &AActionPlayerBase::OnIA_Jump);
		EnhancedInput->BindAction(IA_Jump, ETriggerEvent::Completed, this, &AActionPlayerBase::OnIA_Jump);
		EnhancedInput->BindAction(IA_Jump, ETriggerEvent::Canceled,  this, &AActionPlayerBase::OnIA_Jump);
		EnhancedInput->BindAction(IA_Shoot,ETriggerEvent::Triggered, this, &AActionPlayerBase::OnIA_Shoot);
		EnhancedInput->BindAction(IA_Shoot,ETriggerEvent::Completed, this, &AActionPlayerBase::OnIA_ChargeShoot);
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

void AActionPlayerBase::OnIA_Move(const FInputActionValue& Value)
{
	if (bStop) return;

	const float InputScale = Value.Get<float>();
	if (InputScale == 0.f)
	{
		UE_LOG( LogPlayerInput,Display, TEXT("Move: Zero") );
		return;
	}

	const bool bRight = InputScale > 0.f;
	AddMovementInput(FVector::XAxisVector, InputScale);
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

		Jump();
	}
}

void AActionPlayerBase::OnIA_Shoot(const FInputActionValue& Value)
{
	if (bStop) return;

	const bool bPress = Value.Get<bool>();
	UE_LOG(LogPlayerInput, Display, TEXT("Shoot: %s"), bPress ? TEXT("Pressed") : TEXT("Released"));
	if (bPress)
	{
		Shoot();
	}
}

void AActionPlayerBase::OnIA_ChargeShoot(const FInputActionInstance& Instance)
{
	const float ElapsedTime = Instance.GetElapsedTime();

	UFlashComponent* FlashComp = GetFlashComponent();
	check(FlashComp);

	if (ElapsedTime < HalfChargeTime)
	{
		UE_LOG(LogPlayerInput, Display, TEXT("Charge Shoot: No enough charged."));
	}
	else if (ElapsedTime < FullChargeTime)
	{
		UE_LOG(LogPlayerInput, Display, TEXT("Charge Shoot: Half charged."));
	}
	else
	{
		UE_LOG(LogPlayerInput, Display, TEXT("Charge Shoot: Full charged."));
	}
	UE_LOG(LogPlayerInput, Display, TEXT("Charge Shoot: %f"), ElapsedTime);
}

void AActionPlayerBase::AddDefaultInputMappingContext()
{
	check(PlayerController);
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	check(Subsystem);

	checkf(DefaultIMC, TEXT("%s does NOT setup a property of AActionPlayerBase: " UE_CONVERT_TO_TEXT(DefaultIMC)), *GetName());
	Subsystem->AddMappingContext(DefaultIMC, 0);
}