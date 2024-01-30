
#include "VFX/FlashComponent.h"
#include "VFX/LogVFX.h"
#include "Curves/CurveLinearColor.h"
#include "Curves/CurveFloat.h"
#include "Characters/ActionCharBase.h"

/*********************************************************************************************************************************/
/** FFlashInfo */

float FFlashInfo::GetTimeLength() const
{
	return TimeLength;
}

void FFlashInfo::SetTime(float InTime)
{
	Time = InTime;
}

void FFlashInfo::Tick(float DeltaTime, UMaterialInstanceDynamic& FlashedMaterialRef)
{
	if (!bPlaying) return;

	check(DeltaTime > 0.f);
	Time += DeltaTime;

	float EndTime = 0.f;
	if (CurveTimeRatio == EFlashCurveTimeRatio::EFCTR_Proportional)
	{
		PlayRate = 1.f / TimeLength;
		EndTime = 1.f;
	}
	else if (CurveTimeRatio == EFlashCurveTimeRatio::EFCTR_Absolute)
	{
		EndTime = TimeLength;
	}

	check(PlayRate > 0.f);
	const float Position = Time * PlayRate;
	
	if (Position > EndTime)
	{
		// Finish to flash the material.
		FlashedMaterialRef.SetVectorParameterValue(MaterialColorParamName, FLinearColor(0.f,0.f,0.f,0.f));
		FlashedMaterialRef.SetScalarParameterValue(MaterialFlashPowerParamName, 0.f);

		bPlaying = false;

		UE_LOG(LogVFX, Display, TEXT("The flash is finished."));
	}
	else
	{
		SetFlashColor(Position, FlashedMaterialRef);
		SetFlashPower(Position, FlashedMaterialRef);
	}
}

void FFlashInfo::SetTimeLength(float InTimeLength)
{
	checkf(InTimeLength > 0.f, TEXT("TimeLength should be a positive number."));
	TimeLength = InTimeLength;
}

void FFlashInfo::SetPlayRate(float InPlayRate)
{
	check(InPlayRate > 0.f);
	PlayRate = InPlayRate;
}

void FFlashInfo::SetFlashColor(float Position, UMaterialInstanceDynamic& FlashedMaterialRef)
{
	const FLinearColor NewFlashColor = FlashColorCurve->GetLinearColorValue(Position);
	FlashedMaterialRef.SetVectorParameterValue(MaterialColorParamName, NewFlashColor);
}

void FFlashInfo::SetFlashPower(float Position, UMaterialInstanceDynamic& FlashedMaterialRef)
{
	const float NewFlashPower = FlashPowerFloatCurve->GetFloatValue(Position);
	FlashedMaterialRef.SetScalarParameterValue(MaterialFlashPowerParamName, NewFlashPower);
}

/*********************************************************************************************************************************/
/** UFlashComponent */

UFlashComponent::UFlashComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UFlashComponent::Play()
{
	Activate();
	FlashInfoMap[CurrentInfo].bPlaying = true;

	UE_LOG(LogVFX, Display, TEXT("The %s played."), *CurrentInfo.ToString());
}

void UFlashComponent::Play(const FName& InFlashInfo)
{
	SetFlashInfo(InFlashInfo);
	Play();
}

void UFlashComponent::PlayFromStart()
{
	Play();
	FlashInfoMap[CurrentInfo].Time = 0.f;

	UE_LOG(LogVFX, Display, TEXT("The %s played from start."), *CurrentInfo.ToString());
}

void UFlashComponent::PlayFromStart(const FName& InFlashInfo)
{
	SetFlashInfo(InFlashInfo);
	PlayFromStart();
}

void UFlashComponent::Stop()
{
	FlashInfoMap[CurrentInfo].bPlaying = false;

	UE_LOG(LogVFX, Display, TEXT("The %s stopped."), *CurrentInfo.ToString());
}

bool UFlashComponent::IsPlaying() const
{
	return FlashInfoMap[CurrentInfo].bPlaying;
}

void UFlashComponent::SetFlashInfo(const FName& InFlashInfo)
{
	if (CurrentInfo == InFlashInfo) return;

	if (FlashInfoMap.Contains(InFlashInfo))
	{
		CurrentInfo = InFlashInfo;
	}
	else
	{
		UE_LOG(LogVFX, Warning, TEXT("FlashInfoMap does NOT contain the FlashInfo inputed."));
	}
}

void UFlashComponent::Activate(bool bReset)
{
	Super::Activate(bReset);
	PrimaryComponentTick.SetTickFunctionEnable(true);
}

void UFlashComponent::Deactivate()
{
	Super::Deactivate();
	PrimaryComponentTick.SetTickFunctionEnable(false);
}

void UFlashComponent::AddFlashInfo(const FName& InFlashInfoName, FFlashInfo InFlashInfo)
{
	FlashInfoMap.Add(InFlashInfoName, MoveTemp(InFlashInfo));
}

void UFlashComponent::TickComponent(float DeltaTime,ELevelTick TickType,FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FFlashInfo& CurrentFlash = FlashInfoMap[CurrentInfo];
	UMaterialInstanceDynamic& FlashedMaterial = OwningActionChar->GetSpriteMaterialDynamic();

	CurrentFlash.Tick(DeltaTime, FlashedMaterial);
}

void UFlashComponent::BeginPlay()
{
	Super::BeginPlay();

	OwningActionChar = Cast<AActionCharBase>(GetOwner());
	check(OwningActionChar);
}

