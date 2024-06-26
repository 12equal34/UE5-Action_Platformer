// Fill out your copyright notice in the Description page of Project Settings.


#include "VFX/PaperProjectileVFX.h"
#include "PaperFlipbook.h"
#include "PaperFlipbookComponent.h"

APaperProjectileVFX::APaperProjectileVFX()
{
	UPaperFlipbookComponent* PF = GetPaperFlipbook();
	static ConstructorHelpers::FObjectFinder<UPaperFlipbook> PF_Ref(TEXT("/Game/MegaActionPlatformer/PaperAssets/Fx/player-shoot-hit/player-shoot-hit__Loop.player-shoot-hit__Loop"));
	check(PF_Ref.Succeeded());
	PF->SetFlipbook(PF_Ref.Object);
}
