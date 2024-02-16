#include "GameMode/ActionGameModeBase.h"
#include "Controllers/ActionPlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/PlayerStartPIE.h"
#include "Characters/ActionPlayerBase.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

void AActionGameModeBase::OnPlayerLoses(AActionPlayerController& Player)
{
	UE_LOG(LogGameMode,Display,TEXT("Game Lose : The player will be respawned after %f seconds."), PlayerRespawnTime);
	FTimerDelegate RestartThePlayer = FTimerDelegate::CreateUObject(this, &ThisClass::RespawnPlayer, &Player);
	Player.GetWorldTimerManager().SetTimer(PlayerRespawnTimer, RestartThePlayer, PlayerRespawnTime, false);
	
}

void AActionGameModeBase::AddPlayerStart(APlayerStart* InPlayerStart)
{
	check(InPlayerStart);
	PlayerStarts.Add(InPlayerStart);
}

void AActionGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	// Choose a player start
	APlayerStart* FoundPlayerStart = nullptr;
	UWorld* World = GetWorld();
	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		APlayerStart* PlayerStart = *It;
		FoundPlayerStart = PlayerStart;
		if (PlayerStart->IsA<APlayerStartPIE>())
		{
			// Always prefer the first "Play from Here" PlayerStart, if we find one while in PIE mode
			break;
		}
	}

	AddPlayerStart(FoundPlayerStart);
}

void AActionGameModeBase::RespawnPlayer(AActionPlayerController* PlayerController)
{
	AActionPlayerBase* ActionPlayer = SpawnActionPlayerPawnFor(PlayerController, PlayerStarts.Last());

	// Play a respawn sound.
	if (PlayerRespawnSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, PlayerRespawnSound, ActionPlayer->GetActorLocation());
	}

	PlayerController->SetPawn(ActionPlayer);
	PlayerController->Possess(ActionPlayer);
	PlayerController->PlayerCameraManager->SetViewTarget(ActionPlayer);
	ActionPlayer->FadeInCamera();
}

AActionPlayerBase* AActionGameModeBase::SpawnActionPlayerPawnFor(AActionPlayerController* PlayerController, AActor* StartSpot)
{
	return CastChecked<AActionPlayerBase>(SpawnDefaultPawnFor(PlayerController, StartSpot));
}
