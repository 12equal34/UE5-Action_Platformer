
#include "VFX/FlashComponent.h"
#include "VFX/LogVFX.h"
#include "Curves/CurveLinearColor.h"
#include "Curves/CurveFloat.h"
#include "Characters/ActionCharBase.h"

UFlashComponent::UFlashComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;

	static ConstructorHelpers::FObjectFinder<UCurveLinearColor> FlashColorCurveRef(TEXT("/Game/MegaActionPlatformer/Curves/C_FlashColorAterHit_Color.C_FlashColorAterHit_Color"));
	check(FlashColorCurveRef.Succeeded());
	FlashColorCurve = FlashColorCurveRef.Object;

	static ConstructorHelpers::FObjectFinder<UCurveFloat> FlashPowerFloatCurveRef(TEXT("/Game/MegaActionPlatformer/Curves/C_FlashPowerAterHit_Float.C_FlashPowerAterHit_Float"));
	check(FlashPowerFloatCurveRef.Succeeded());
	FlashPowerFloatCurve = FlashPowerFloatCurveRef.Object;

	FlashColorName = TEXT("FlashColor");
	FlashPowerName = TEXT("FlashPower");
}

void UFlashComponent::Play()
{
	Activate();
	bPlaying = true;

	UE_LOG(LogVFX, Display, TEXT("The flash played."));
}

void UFlashComponent::PlayFromStart()
{
	Play();
	Time = 0.f;

	UE_LOG(LogVFX, Display, TEXT("The flash played from start."));
}

void UFlashComponent::Stop()
{
	bPlaying = false;

	UE_LOG(LogVFX, Display, TEXT("The flash stopped."));
}

bool UFlashComponent::IsPlaying() const
{
	return bPlaying;
}

void UFlashComponent::SetTime(float InTime)
{
	Time = InTime;
}

void UFlashComponent::SetTimeLength(float InTimeLength)
{
	check(InTimeLength > 0.f);
	TimeLength = InTimeLength;
}

void UFlashComponent::SetFlashColorName(FName InName)
{
	FlashColorName = InName;
}

void UFlashComponent::SetFlashPowerName(FName InName)
{
	FlashPowerName = InName;
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

void UFlashComponent::TickComponent(float DeltaTime,ELevelTick TickType,FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bPlaying)
	{
		// const float OldTime = Time;
		Time += DeltaTime;

		const float Pos = Time * PlayRate;
		if (Pos > 1.f)
		{
			SetFlashColor(FLinearColor(0.f,0.f,0.f,0.f));
			SetFlashPower(0.f);

			bPlaying = false;

			UE_LOG(LogVFX, Display, TEXT("The flash is finished."));
		}
		else
		{
			const FLinearColor NewFlashColor = FlashColorCurve->GetLinearColorValue(Pos);
			SetFlashColor(NewFlashColor);

			const float NewFlashPower = FlashPowerFloatCurve->GetFloatValue(Pos);
			SetFlashPower(NewFlashPower);
		}
	}
}

void UFlashComponent::BeginPlay()
{
	Super::BeginPlay();

	OwningActionChar = Cast<AActionCharBase>(GetOwner());
	check(OwningActionChar);

	checkf(TimeLength > 0.f, TEXT("TimeLength should be a positive number."));

	PlayRate = 1.f / TimeLength;
	check(PlayRate > 0.f);
}

void UFlashComponent::SetFlashColor(FLinearColor FlashColor)
{
	OwningActionChar->GetSpriteMaterialDynamic().SetVectorParameterValue(FlashColorName, FlashColor);
}

void UFlashComponent::SetFlashPower(float FlashPower)
{
	OwningActionChar->GetSpriteMaterialDynamic().SetScalarParameterValue(FlashPowerName, FlashPower);
}
