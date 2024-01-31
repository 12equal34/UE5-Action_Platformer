// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FlashComponent.generated.h"

UENUM()
enum class EFlashInfoPlayPurpose
{
	EFIPP_WantsFixPlayTime,
	EFIPP_WantsUsePlayRate
};

USTRUCT()
struct FFlashInfo
{
	GENERATED_BODY()

	UPROPERTY(Category=VFX,EditDefaultsOnly)
	FName MaterialColorParamName;

	UPROPERTY(Category=VFX,EditDefaultsOnly)
	FName MaterialFlashPowerParamName;

	UPROPERTY(Category=VFX,EditDefaultsOnly)
	TObjectPtr<UCurveLinearColor> FlashColorCurve;

	UPROPERTY(Category=VFX,EditDefaultsOnly)
	TObjectPtr<UCurveFloat> FlashPowerFloatCurve;

	UPROPERTY(Category=VFX,EditDefaultsOnly)
	EFlashInfoPlayPurpose PlayPurpose = EFlashInfoPlayPurpose::EFIPP_WantsFixPlayTime;

	UPROPERTY(Category=VFX,EditDefaultsOnly)
	float MinCurvePos = 0.f;

	UPROPERTY(Category=VFX,EditDefaultsOnly)
	float MaxCurvePos = 1.f;

	UPROPERTY(Category=VFX,EditDefaultsOnly,meta=(EditCondition="PlayPurpose==EFlashInfoPlayPurpose::EFIPP_WantsFixPlayTime",ClampMin="0.01"))
	float WantedPlayTime = 1.f;

	UPROPERTY(Category=VFX,EditDefaultsOnly,meta=(EditCondition="PlayPurpose==EFlashInfoPlayPurpose::EFIPP_WantsUsePlayRate",ClampMin="0.01"))
	float WantedPlayRate = 1.f;

	UPROPERTY(Category=VFX,EditDefaultsOnly)
	bool bLooping = false;

	UPROPERTY(Transient,Category=VFX,VisibleInstanceOnly)
	bool bPlaying = false;

	UPROPERTY(Transient,Category=VFX,VisibleInstanceOnly)
	float CurvePos;

	void SetFlashColor(float Position, UMaterialInstanceDynamic& FlashedMaterialRef);

	void SetFlashPower(float Position, UMaterialInstanceDynamic& FlashedMaterialRef);

	void Tick(float DeltaTime, UMaterialInstanceDynamic& FlashedMaterialRef);

	void FinishFlash(UMaterialInstanceDynamic& FlashedMaterialRef);
};

class AActionCharBase;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MEGAACTIONPLATFORMER_API UFlashComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UFlashComponent();

	void PlayFlash();
	void PlayFlash(const FName& InFlashInfo);
	void PlayFlashFromStart();
	void PlayFlashFromStart(const FName& InFlashInfo);
	void StopFlash();
	void FinishFlash();

	const FName& GetCurrentInfo() const;
	bool IsPlaying() const;
	bool IsLooping() const;
	bool CurrentFlashIs(const FName& OtherFlashInfo) const;

	void SetFlashInfo(const FName& InFlashInfo);
	void AddFlashInfo(const FName& InFlashInfoName, FFlashInfo InFlashInfo);

	//~ Begin ActorComponent Interface.
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	virtual void Activate(bool bReset=false) override;
	virtual void Deactivate() override;
	//~ End ActorComponent Interface.

#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif

private:
	UPROPERTY(Transient)
	TObjectPtr<AActionCharBase> OwningActionChar;

	UPROPERTY(Category=VFX,EditDefaultsOnly)
	TMap<FName,FFlashInfo> FlashInfoMap;

	UPROPERTY(Category=VFX,VisibleInstanceOnly,Transient)
	FName CurrentInfo;
};
