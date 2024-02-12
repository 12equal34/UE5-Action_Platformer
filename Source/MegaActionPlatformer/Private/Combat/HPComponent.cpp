
#include "Combat/HPComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogHP, Display, All);

UHPComponent::UHPComponent()
{
	bWantsInitializeComponent = true;

	checkf(MaxHP > 0.f, TEXT("The maximum HP should be always positive."));
}

void UHPComponent::InitializeComponent()
{
	Super::InitializeComponent();

	checkf(MaxHP > 0.f, TEXT("Setting: The maximum HP should set a positive number."));
	HealFully();
}

void UHPComponent::SetMaximumHP(float InMaxHP)
{
	checkf(InMaxHP > 0.f, TEXT("You can't set the maximum HP by a nonpositive number."));
	MaxHP = InMaxHP;

	UE_LOG(LogHP, Display, TEXT("Set Maximum HP: %f (%s)"), MaxHP, *GetOwner()->GetName());
}

void UHPComponent::SetCurrentHP(float InHP, bool bCanExcessMax)
{
	checkf(InHP >= 0.f, TEXT("You can't set the current HP by a negative number."));
	HP = InHP;
	if (!bCanExcessMax)
	{
		HP = FMath::Min(HP, MaxHP);
	}

	if (IsZero())
	{
		OnHPBecameZero.ExecuteIfBound();
	}

	UE_LOG(LogHP, Display, TEXT("Set HP: %f (%s)(%s)"), HP, HP > MaxHP ? TEXT("Excess") : TEXT("Full"),*GetOwner()->GetName());
}

float UHPComponent::Injure(float InDamage)
{
	checkf(InDamage >= 0.f, TEXT("Received damage is innegative."));
	HP = FMath::Clamp(HP - InDamage, 0.f, MaxHP);

	if (IsZero())
	{
		OnHPBecameZero.ExecuteIfBound();
	}

	UE_LOG(LogHP, Display, TEXT("HP: %f (Injured)(%s)"), HP, *GetOwner()->GetName());

	return HP;
}

float UHPComponent::Heal(float InHealth, bool bCanExcessMax)
{
	checkf(InHealth >= 0.f, TEXT("Received health is innegative."));
	HP += InHealth;
	if (!bCanExcessMax)
	{
		HP = FMath::Min(HP, MaxHP);
	}

	UE_LOG(LogHP, Display, TEXT("HP: %f (Healed)(%s)"), HP, *GetOwner()->GetName());

	return HP;
}

float UHPComponent::HealRatio(float InRatio, bool bCanExcessMax)
{
	check(InRatio >= 0.f && InRatio <= 1.f);
	const float Health = MaxHP * InRatio;

	UE_LOG(LogHP, Display, TEXT("HP: %f (Healed)(%s)"), HP, *GetOwner()->GetName());

	return Heal(Health, bCanExcessMax);
}

void UHPComponent::HealFully()
{
	HP = MaxHP;

	UE_LOG(LogHP, Display, TEXT("HP: %f (Full Healed)(%s)"), HP, *GetOwner()->GetName());
}

bool UHPComponent::IsZero() const
{
	return HP == 0.f;
}

bool UHPComponent::IsLeft() const
{
	return HP > 0.f;
}

bool UHPComponent::IsExcess() const
{
	return HP > MaxHP;
}

bool UHPComponent::IsFull() const
{
	return HP == MaxHP;
}

void UHPComponent::BeginPlay()
{
	Super::BeginPlay();

}