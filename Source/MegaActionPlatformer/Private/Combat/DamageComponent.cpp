
#include "Combat/DamageComponent.h"
#include "Engine/DamageEvents.h"
#include "Characters/ActionCharBase.h"

UDamageComponent::UDamageComponent()
{
	
}

void UDamageComponent::SetDamage(float InDamage)
{
	checkf(InDamage >= 0.f, TEXT("SetDamage: You should set a damage by a nonnegative number."));
	Damage = InDamage;
}

float UDamageComponent::ApplyDamage(AActionCharBase& DamagedActionChar)
{
	if (Damage != 0.f)
	{
		FDamageEvent DamageEvent(DamageTypeClass);
		return DamagedActionChar.TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	}

	return 0.f;
}

void UDamageComponent::BeginPlay()
{
	Super::BeginPlay();

	checkf(Damage >= 0.f, TEXT("Setting: You should set a damage by a nonnegative number."));

	DamageCauser = GetOwner();
	if (DamageCauser)
	{
		EventInstigator = DamageCauser->GetInstigatorController();
	}

	// make sure we have a good damage type
	if (!DamageTypeClass)
	{
		DamageTypeClass = TSubclassOf<UDamageType>(UDamageType::StaticClass());
	}
}