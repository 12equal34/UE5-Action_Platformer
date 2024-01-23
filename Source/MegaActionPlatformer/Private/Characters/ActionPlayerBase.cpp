// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/ActionPlayerBase.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"

DEFINE_LOG_CATEGORY(LogActionPlayerInput);

#pragma warning(disable: 26813)

#define UE_CONVERT_TO_TEXT(a) #a

AActionPlayerBase::AActionPlayerBase()
{
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
	check(SpringArmComponent);
	SpringArmComponent->SetupAttachment(GetRootComponent());
	SpringArmComponent->SetRelativeRotation(FRotator(-15.f, -90.f, 0.f));
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
}

void AActionPlayerBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (ensure(EnhancedInput))
	{
		checkf(IA_Move, TEXT("%s is NOT set."), *IA_Move.GetName());
		checkf(IA_Jump, TEXT("%s is NOT set."), *IA_Jump.GetName());

		EnhancedInput->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AActionPlayerBase::OnIA_Move);
		EnhancedInput->BindAction(IA_Jump, ETriggerEvent::Triggered, this, &AActionPlayerBase::OnIA_Jump);
		EnhancedInput->BindAction(IA_Jump, ETriggerEvent::Completed, this, &AActionPlayerBase::OnIA_Jump);
		EnhancedInput->BindAction(IA_Jump, ETriggerEvent::Canceled,  this, &AActionPlayerBase::OnIA_Jump);
	}
}

void AActionPlayerBase::BeginPlay()
{
	Super::BeginPlay();

	if (IsPlayerControlled())
	{
		AddDefaultInputMappingContext();
	}
}

void AActionPlayerBase::OnIA_Move(const FInputActionValue& Value)
{
	float InputScale = Value.Get<float>();
	AddMovementInput(FVector::XAxisVector, InputScale);
	UE_LOG(LogActionPlayerInput,Display, TEXT("Move: %s"), ( InputScale>0.f ? TEXT("Right") : (InputScale<0.f ? TEXT("Left") : TEXT("Zero")) ));
}

void AActionPlayerBase::OnIA_Jump(const FInputActionInstance& Instance)
{
	ETriggerEvent TriggerEvent = Instance.GetTriggerEvent();
	if (TriggerEvent == ETriggerEvent::Completed)
	{
		UE_LOG(LogActionPlayerInput, Display, TEXT("Jump Completed"));
	}
	else if (TriggerEvent == ETriggerEvent::Canceled)
	{
		UE_LOG(LogActionPlayerInput, Display, TEXT("Jump Canceled"));

		StopJumping();
	}
	else if (TriggerEvent == ETriggerEvent::Triggered)
	{
		UE_LOG(LogActionPlayerInput, Display, TEXT("Jump Triggered"));

		Jump();
	}
}

void AActionPlayerBase::AddDefaultInputMappingContext()
{
	APlayerController* PC = CastChecked<APlayerController>(GetController());
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
	check(Subsystem);

	checkf(DefaultIMC, TEXT("%s does NOT setup a property of AActionPlayerBase: " UE_CONVERT_TO_TEXT(DefaultIMC)), *GetName());
	Subsystem->AddMappingContext(DefaultIMC, 0);
}