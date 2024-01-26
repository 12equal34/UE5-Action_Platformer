// Fill out your copyright notice in the Description page of Project Settings.


#include "VFX/PaperVFXBase.h"
#include "VFX/LogVFX.h"
#include "PaperFlipbookComponent.h"

APaperVFXBase::APaperVFXBase()
{
	PaperFlipbookComponent = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("PaperFlipbook"));
	check(PaperFlipbookComponent != nullptr);
	SetRootComponent(PaperFlipbookComponent);
	PaperFlipbookComponent->SetCollisionProfileName(TEXT("VFX"));
}

void APaperVFXBase::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogVFX, Display, TEXT("%s is spawned."), *GetName());
	check(PaperFlipbookComponent != nullptr);
	PaperFlipbookComponent->SetLooping(false);
	PaperFlipbookComponent->PlayFromStart();
	PaperFlipbookComponent->OnFinishedPlaying.AddDynamic(this, &APaperVFXBase::OnFinishedPlaying);
}

void APaperVFXBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (EndPlayReason == EEndPlayReason::Destroyed)
	{
		UE_LOG(LogVFX, Display, TEXT("%s is destroyed."), *GetName());
	}
}

void APaperVFXBase::OnFinishedPlaying()
{
	ensure(Destroy() == true);
}
