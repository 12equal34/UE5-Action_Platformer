// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ActionGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class MEGAACTIONPLATFORMER_API AActionGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
public:
	void OnPlayerLoses(AController* Player);

private:
	UPROPERTY(Category=Game,EditDefaultsOnly)
	float PlayerRestartTime = 3.f;

	FTimerHandle PlayerRestartTimer;
};
