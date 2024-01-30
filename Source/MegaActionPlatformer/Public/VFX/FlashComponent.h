// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FlashComponent.generated.h"

class AActionCharBase;

UENUM()
enum class EFlashCurveTimeRatio
{
	EFCTR_Absolute,
	EFCTR_Proportional
};

USTRUCT()
struct FFlashInfo
{
	GENERATED_BODY()

	FFlashInfo() : 
		CurveTimeRatio(EFlashCurveTimeRatio::EFCTR_Absolute),
		Time(0.f), 
		bPlaying(false), 
		TimeLength(1.f),
		PlayRate(1.f)
	{}

	UPROPERTY(Category=VFX,EditDefaultsOnly)
	FName MaterialColorParamName;

	UPROPERTY(Category=VFX,EditDefaultsOnly)
	FName MaterialFlashPowerParamName;

	UPROPERTY(Category=VFX,EditDefaultsOnly)
	TObjectPtr<UCurveLinearColor> FlashColorCurve;

	UPROPERTY(Category=VFX,EditDefaultsOnly)
	TObjectPtr<UCurveFloat> FlashPowerFloatCurve;

	UPROPERTY(Category=VFX,EditDefaultsOnly)
	EFlashCurveTimeRatio CurveTimeRatio;

	UPROPERTY(Category=VFX,VisibleInstanceOnly)
	float Time;
	
	UPROPERTY(Category=VFX,VisibleInstanceOnly)
	bool bPlaying;

	float GetTimeLength() const;

	void SetTime(float InTime);
	void SetTimeLength(float InTimeLength);
	void SetPlayRate(float InPlayRate);
	void SetFlashColor(float Position, UMaterialInstanceDynamic& FlashedMaterialRef);
	void SetFlashPower(float Position, UMaterialInstanceDynamic& FlashedMaterialRef);

	void Tick(float DeltaTime, UMaterialInstanceDynamic& FlashedMaterialRef);

private:
	/** TimeLength should be always positive. */
	UPROPERTY(Category=VFX,EditDefaultsOnly)
	float TimeLength;

	UPROPERTY(Category=VFX,EditDefaultsOnly)
	float PlayRate;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MEGAACTIONPLATFORMER_API UFlashComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UFlashComponent();

	void Play();
	void Play(const FName& InFlashInfo);
	void PlayFromStart();
	void PlayFromStart(const FName& InFlashInfo);
	void Stop();

	bool IsPlaying() const;

	void SetFlashInfo(const FName& InFlashInfo);

	void AddFlashInfo(const FName& InFlashInfoName, FFlashInfo InFlashInfo);

	//~ Begin ActorComponent Interface.
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	virtual void Activate(bool bReset=false) override;
	virtual void Deactivate() override;
	//~ End ActorComponent Interface.

private:
	UPROPERTY(Transient)
	TObjectPtr<AActionCharBase> OwningActionChar;

	UPROPERTY(Category=VFX,EditDefaultsOnly)
	TMap<FName,FFlashInfo> FlashInfoMap;

	UPROPERTY(Category=VFX,VisibleInstanceOnly,Transient)
	FName CurrentInfo;
};
