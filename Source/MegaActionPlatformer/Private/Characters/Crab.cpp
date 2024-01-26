// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Crab.h"
#include "Components/CapsuleComponent.h"
#include "PaperFlipbook.h"
#include "PaperFlipbookComponent.h"

ACrab::ACrab()
{
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	Capsule->SetCapsuleHalfHeight(45.f);
	Capsule->SetCapsuleRadius(45.f);

	UPaperFlipbookComponent* MySprite = GetSprite();
	static ConstructorHelpers::FObjectFinder<UPaperFlipbook> PF_Ref(TEXT("/Game/MegaActionPlatformer/PaperAssets/Characters/Enemies/crab/crab__Idle.crab__Idle"));
	check(PF_Ref.Succeeded());
	MySprite->SetFlipbook(PF_Ref.Object);
	MySprite->SetRelativeLocation(FVector(0.f, 0.f, 15.9f));
}
