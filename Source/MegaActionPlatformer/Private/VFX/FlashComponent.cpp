
#include "VFX/FlashComponent.h"
#include "VFX/LogVFX.h"
#include "Curves/CurveLinearColor.h"
#include "Curves/CurveFloat.h"
#include "Characters/ActionCharBase.h"

/*********************************************************************************************************************************/
/** FFlashInfo */

void FFlashInfo::Tick(float DeltaTime, UMaterialInstanceDynamic& FlashedMaterialRef)
{
	if (!bPlaying) return;

	check(DeltaTime > 0.f);

	const float CurvePosLength = MaxCurvePos - MinCurvePos;
	checkf(CurvePosLength > 0.f, TEXT("You should set the MinCurvePos value less than MaxCurvePos."));

	// Find the CurveInterval for a DeltaTime to fit its purpose.
	float PlayRate = 0.f;
	if (PlayPurpose == EFlashInfoPlayPurpose::EFIPP_WantsFixPlayTime)
	{
		PlayRate = CurvePosLength / WantedPlayTime;
		check(PlayRate > 0.f);
	}
	else if (PlayPurpose == EFlashInfoPlayPurpose::EFIPP_WantsUsePlayRate)
	{
		PlayRate = WantedPlayRate;
		check(PlayRate > 0.f);
	}
	else 
	{ 
		checkNoEntry();
	}
	const float CurveInterval = PlayRate * DeltaTime;

	// Set a current CurvePos. Then flash the material or finish if not loopping and over the max.
	CurvePos += CurveInterval;
	if (CurvePos > MaxCurvePos)
	{
		if (bLooping)
		{
			CurvePos -= CurvePosLength;
		}
		else
		{
			FinishFlash(FlashedMaterialRef);
			return;
		}
	}
	SetFlashColor(CurvePos, FlashedMaterialRef);
	SetFlashPower(CurvePos, FlashedMaterialRef);
}

void FFlashInfo::FinishFlash(UMaterialInstanceDynamic& FlashedMaterialRef)
{
	FlashedMaterialRef.SetVectorParameterValue(MaterialColorParamName, FLinearColor(0.f,0.f,0.f,0.f));
	FlashedMaterialRef.SetScalarParameterValue(MaterialFlashPowerParamName, 0.f);

	bPlaying = false;

	UE_LOG(LogVFX, Display, TEXT("The flash is finished."));
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

void UFlashComponent::PlayFlash()
{
	Activate();
	FlashInfoMap[CurrentInfo].bPlaying = true;

	UE_LOG(LogVFX, Display, TEXT("The %s played."), *CurrentInfo.ToString());
}

void UFlashComponent::PlayFlash(const FName& InFlashInfo)
{
	SetFlashInfo(InFlashInfo);
	PlayFlash();
}

void UFlashComponent::PlayFlashFromStart()
{
	PlayFlash();
	FlashInfoMap[CurrentInfo].CurvePos = FlashInfoMap[CurrentInfo].MinCurvePos;

	UE_LOG(LogVFX, Display, TEXT("The %s played from start."), *CurrentInfo.ToString());
}

void UFlashComponent::PlayFlashFromStart(const FName& InFlashInfo)
{
	SetFlashInfo(InFlashInfo);
	PlayFlashFromStart();
}

void UFlashComponent::StopFlash()
{
	FlashInfoMap[CurrentInfo].bPlaying = false;

	UE_LOG(LogVFX, Display, TEXT("The %s stopped."), *CurrentInfo.ToString());
}

void UFlashComponent::FinishFlash()
{
	UMaterialInstanceDynamic& FlashedMaterial = OwningActionChar->GetSpriteMaterialDynamic();
	FlashInfoMap[CurrentInfo].FinishFlash(FlashedMaterial);
}

const FName& UFlashComponent::GetCurrentInfo() const
{
	check(FlashInfoMap.Contains(CurrentInfo) == true);
	return CurrentInfo;
}

bool UFlashComponent::IsPlaying() const
{
	return FlashInfoMap[CurrentInfo].bPlaying;
}

bool UFlashComponent::IsLooping() const
{
	return FlashInfoMap[CurrentInfo].bLooping;
}

bool UFlashComponent::CurrentFlashIs(const FName& OtherFlashInfo) const
{
	return CurrentInfo == OtherFlashInfo;
}

void UFlashComponent::SetFlashInfo(const FName& InFlashInfo)
{
	if (CurrentInfo == InFlashInfo) return;

	checkf(FlashInfoMap.Contains(InFlashInfo), TEXT("FlashInfoMap does NOT contain the FlashInfo inputed."));

	CurrentInfo = InFlashInfo;
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

#if WITH_EDITOR
void UFlashComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);

	FProperty* ChangedProperty = PropertyChangedEvent.Property;
	if (!ChangedProperty) return;

	const FName PropertyName = ChangedProperty->GetFName();

	if (PropertyName == GET_MEMBER_NAME_CHECKED(FFlashInfo,MinCurvePos) || PropertyName == GET_MEMBER_NAME_CHECKED(FFlashInfo,MaxCurvePos))
	{
		for (auto& [Name,FlashInfoRef] : FlashInfoMap)
		{
			if (FlashInfoRef.MinCurvePos < FlashInfoRef.MaxCurvePos)
			{
				continue;
			}

			if (FlashInfoRef.MinCurvePos > FlashInfoRef.MaxCurvePos)
			{
				UE_LOG(LogTemp,Warning,TEXT("You should set MinCurvePos less than MaxCurvePos. So these just swaped."));
				Swap(FlashInfoRef.MinCurvePos,FlashInfoRef.MaxCurvePos);
			}
			else if (FlashInfoRef.MinCurvePos == FlashInfoRef.MaxCurvePos)
			{
				UE_LOG(LogTemp,Warning,TEXT("You can't set MinCurvePos equal to MaxCurvePos. So MaxCurvePos is added by 1.f"));
				FlashInfoRef.MaxCurvePos = FlashInfoRef.MinCurvePos + 1.f;
			}
			FlashInfoRef.CurvePos = FlashInfoRef.MinCurvePos;
		}
	}
}
#endif