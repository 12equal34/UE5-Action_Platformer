// Fill out your copyright notice in the Description page of Project Settings.


#include "VFX/PaperDestructionVFX.h"
#include "PaperFlipbook.h"
#include "PaperFlipbookComponent.h"

APaperDestructionVFX::APaperDestructionVFX()
{
	UPaperFlipbookComponent* PF = GetPaperFlipbook();
	static ConstructorHelpers::FObjectFinder<UPaperFlipbook> PF_Ref(TEXT("/Game/MegaActionPlatformer/PaperAssets/Fx/explosion/explosion__Loop.explosion__Loop"));
	check(PF_Ref.Succeeded());
	PF->SetFlipbook(PF_Ref.Object);
}
