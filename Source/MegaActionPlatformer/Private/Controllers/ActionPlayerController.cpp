// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/ActionPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Characters/ActionPlayerBase.h"

AActionPlayerController::AActionPlayerController()
{
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> DefaultIMC_Ref(TEXT("/Game/MegaActionPlatformer/Input/IMC_Action.IMC_Action"));
	check(DefaultIMC_Ref.Succeeded());
	DefaultIMC = DefaultIMC_Ref.Object;
}

void AActionPlayerController::BeginPlay()
{
	Super::BeginPlay();

	AddDefaultInputMappingContext();
}

void AActionPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (AActionPlayerBase* PlayerPawn = Cast<AActionPlayerBase>(GetPawn()))
	{
		PlayerPawn->SetPlayerController(*this);
	}
}

void AActionPlayerController::AddDefaultInputMappingContext()
{
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	check(Subsystem);

	checkf(DefaultIMC, TEXT("%s does NOT setup a property of AActionPlayerBase: DefaultIMC"), *GetName());
	Subsystem->AddMappingContext(DefaultIMC, 0);
}
