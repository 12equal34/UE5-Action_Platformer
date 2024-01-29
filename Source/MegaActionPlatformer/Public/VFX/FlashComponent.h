// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FlashComponent.generated.h"

class AActionCharBase;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MEGAACTIONPLATFORMER_API UFlashComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UFlashComponent();

	void Play();
	void PlayFromStart();
	void Stop();

	bool IsPlaying() const;
	float GetTimeLength() const { return TimeLength; }

	void SetTime(float InTime);
	void SetTimeLength(float InTimeLength);
	void SetFlashColorName(FName InName);
	void SetFlashPowerName(FName InName);

	//~ Begin ActorComponent Interface.
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	virtual void Activate(bool bReset=false) override;
	virtual void Deactivate() override;
	//~ End ActorComponent Interface.

private:
	void SetFlashColor(FLinearColor FlashColor);
	void SetFlashPower(float FlashPower);

	UPROPERTY(Transient)
	TObjectPtr<AActionCharBase> OwningActionChar;

	UPROPERTY(Category=VFX,EditDefaultsOnly)
	FName FlashColorName;

	UPROPERTY(Category=VFX,EditDefaultsOnly)
	FName FlashPowerName;

	UPROPERTY(Category=VFX,EditDefaultsOnly)
	float TimeLength = 1.f;

	UPROPERTY(Category=VFX,EditDefaultsOnly)
	TObjectPtr<UCurveLinearColor> FlashColorCurve;

	UPROPERTY(Category=VFX,EditDefaultsOnly)
	TObjectPtr<UCurveFloat> FlashPowerFloatCurve;

	float Time = 0.f;
	float PlayRate;
	bool bPlaying = false;
};
