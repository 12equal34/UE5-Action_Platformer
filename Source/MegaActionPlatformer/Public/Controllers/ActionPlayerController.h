// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ActionPlayerController.generated.h"

class UInputMappingContext;

UCLASS()
class MEGAACTIONPLATFORMER_API AActionPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	AActionPlayerController();

protected:
	virtual void BeginPlay() override;

	virtual void OnPossess(APawn* InPawn);

private:
	void AddDefaultInputMappingContext();

	UPROPERTY(Category=Input,EditDefaultsOnly)
	TObjectPtr<UInputMappingContext> DefaultIMC;
};
