#include "GameMode/ActionGameModeBase.h"

void AActionGameModeBase::OnPlayerLoses(AController* Player)
{
	check(Player);

	UE_LOG(LogGameMode,Display,TEXT("Game Lose : The player will be respawned after %f seconds."), PlayerRestartTime);
	FTimerDelegate RestartThePlayer = FTimerDelegate::CreateUObject(this, &ThisClass::RestartPlayer, Player);
	Player->GetWorldTimerManager().SetTimer(PlayerRestartTimer, RestartThePlayer, PlayerRestartTime, false);
}