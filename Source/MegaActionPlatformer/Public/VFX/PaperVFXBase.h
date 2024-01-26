// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PaperVFXBase.generated.h"

class UPaperFlipbookComponent;

UCLASS()
class MEGAACTIONPLATFORMER_API APaperVFXBase : public AActor
{
	GENERATED_BODY()
	
public:	
	APaperVFXBase();

protected:
	//~ Begin AActor Interface.
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor Interface.

	FORCEINLINE UPaperFlipbookComponent* GetPaperFlipbook() const { return PaperFlipbookComponent; }

	UFUNCTION()
	void OnFinishedPlaying();

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPaperFlipbookComponent> PaperFlipbookComponent;
};
