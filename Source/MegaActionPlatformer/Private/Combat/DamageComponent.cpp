
#include "Combat/DamageComponent.h"
#include "Engine/DamageEvents.h"
#include "Characters/ActionCharBase.h"
#include "GameFramework/CharacterMovementComponent.h"

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
		if (bCanKnockback)
		{
			Knockback(DamagedActionChar);
		}
		
		FDamageEvent DamageEvent(DamageTypeClass);
		return DamagedActionChar.TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	}

	return 0.f;
}

void UDamageComponent::Knockback(AActionCharBase& OtherActionChar)
{
	const bool bIsFlying = OtherActionChar.GetCharacterMovement()->MovementMode == EMovementMode::MOVE_Flying;
	if (!bIsFlying)
	{
		FVector LaunchVelocity;
		const bool bOtherIsOnLeftSide = OtherActionChar.GetActorLocation().X < GetOwner()->GetActorLocation().X;
		LaunchVelocity.X = HorizontalKnockbackPower * (bOtherIsOnLeftSide ? -1.f : 1.f);
		LaunchVelocity.Z = VerticalKnockbackPower;
		const bool bXYOverride = true;
		const bool bZOverride = true;
		OtherActionChar.LaunchCharacter(LaunchVelocity, bXYOverride, bZOverride);
	}
	else
	{
		FVector ImpulseDir = OtherActionChar.GetActorLocation() - GetOwner()->GetActorLocation();
		ImpulseDir.Normalize();
		const FVector Impulse = HorizontalKnockbackPower * ImpulseDir;
		const bool bVelocityChange = true;
		OtherActionChar.GetCharacterMovement()->AddImpulse(Impulse, bVelocityChange);
	}
	OtherActionChar.OnKnockbacked(KnockbackTime);
}

void UDamageComponent::SetCanKnockback(bool bInCanKnockback)
{
	bCanKnockback = bInCanKnockback;
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