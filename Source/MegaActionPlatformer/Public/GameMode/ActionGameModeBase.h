// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ActionGameModeBase.generated.h"

class AActionPlayerBase;
class AActionPlayerController;
class APlayerStart;

UCLASS()
class MEGAACTIONPLATFORMER_API AActionGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
public:
	void OnPlayerLoses(AActionPlayerController& Player);

	void AddPlayerStart(APlayerStart* InPlayerStart);

	FORCEINLINE float GetPlayerRespawnTime() const { return PlayerRespawnTime; }

	UFUNCTION(BlueprintImplementableEvent)
	void WinPlayer(AActionPlayerController* Player);

protected:
	virtual void BeginPlay() override;

private:
	void RespawnPlayer(AActionPlayerController* Player);

	AActionPlayerBase* SpawnActionPlayerPawnFor(AActionPlayerController* PlayerController, AActor* StartSpot);

	UPROPERTY(Category=Game,EditDefaultsOnly)
	float PlayerRespawnTime = 3.f;

	FTimerHandle PlayerRespawnTimer;

	UPROPERTY(Transient)
	TArray<APlayerStart*> PlayerStarts;

	UPROPERTY(Category=Sounds,EditDefaultsOnly)
	TObjectPtr<USoundBase> PlayerRespawnSound;
};
