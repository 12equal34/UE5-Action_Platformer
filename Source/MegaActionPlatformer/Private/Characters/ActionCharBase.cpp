// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/ActionCharBase.h"
#include "PaperFlipbookComponent.h"
#include "PaperZDAnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Factions/ActionFactionComponent.h"
#include "VFX/PaperDestructionVFX.h"
#include "Combat/HPComponent.h"
#include "VFX/FlashComponent.h"
#include "Curves/CurveLinearColor.h"
#include "Curves/CurveFloat.h"
#include "Kismet/GameplayStatics.h"

AActionCharBase::AActionCharBase()
{
	UPaperFlipbookComponent* MySprite = GetSprite();
	check(MySprite);
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaskedMaterialRef(TEXT("/Game/MegaActionPlatformer/Materials/MI_CustomUnlitSpriteMaterial.MI_CustomUnlitSpriteMaterial"));
	check(MaskedMaterialRef.Succeeded());
	MySprite->SetMaterial(0, MaskedMaterialRef.Object);
	MySprite->SetCastShadow(false);

	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	check(CapsuleComp);
	CapsuleComp->SetCapsuleHalfHeight(60.f);
	CapsuleComp->SetCollisionProfileName(TEXT("ActionChar"));

	UCharacterMovementComponent* CharMovement = GetCharacterMovement();
	check(CharMovement);
	CharMovement->SetPlaneConstraintEnabled(true);
	CharMovement->SetPlaneConstraintAxisSetting(EPlaneConstraintAxisSetting::Y);
	CharMovement->bUseFlatBaseForFloorChecks = true;
	CharMovement->PerchRadiusThreshold = 1.f;
	CharMovement->GravityScale = 5.5f;
	CharMovement->JumpZVelocity = 600.f;
	CharMovement->AirControl = 0.7f;

	HPComponent = CreateDefaultSubobject<UHPComponent>(TEXT("HP"));
	check(HPComponent);

	FactionComponent = CreateDefaultSubobject<UActionFactionComponent>(TEXT("Faction"));
	check(FactionComponent);

	static ConstructorHelpers::FClassFinder<APaperDestructionVFX> DestructionVfxClassRef(TEXT("/Game/MegaActionPlatformer/Blueprints/Characters/BP_DestructionVFX_BASE.BP_DestructionVFX_BASE_C"));
	check(DestructionVfxClassRef.Succeeded());

	DestructionVfxClass = DestructionVfxClassRef.Class;

	FlashComponent = CreateDefaultSubobject<UFlashComponent>(TEXT("HitFlash"));
	check(FlashComponent);

	/** 
	 * Prepare curves for Flash VFXs. 
	*/ 
	static ConstructorHelpers::FObjectFinder<UCurveLinearColor> HitFlashColorCurveRef(TEXT("/Game/MegaActionPlatformer/Curves/C_FlashColorAterHit_Color.C_FlashColorAterHit_Color"));
	check(HitFlashColorCurveRef.Succeeded());

	static ConstructorHelpers::FObjectFinder<UCurveFloat> HitFlashPowerFloatCurveRef(TEXT("/Game/MegaActionPlatformer/Curves/C_FlashPowerAterHit_Float.C_FlashPowerAterHit_Float"));
	check(HitFlashPowerFloatCurveRef.Succeeded());

	// Add a HitFlash Info.
	FFlashInfo HitFlashInfo;
	HitFlashInfo.PlayPurpose = EFlashInfoPlayPurpose::EFIPP_WantsFixPlayTime;
	HitFlashInfo.bLooping = false;
	HitFlashInfo.MinCurvePos = 0.f;
	HitFlashInfo.MaxCurvePos = 1.f;
	HitFlashInfo.WantedPlayTime = 0.3f;
	HitFlashInfo.MaterialColorParamName      = TEXT("FlashColor");
	HitFlashInfo.MaterialFlashPowerParamName = TEXT("FlashPower");
	HitFlashInfo.FlashColorCurve      = HitFlashColorCurveRef.Object;
	HitFlashInfo.FlashPowerFloatCurve = HitFlashPowerFloatCurveRef.Object;
	HitFlashName = TEXT("HitFlash");
	FlashComponent->AddFlashInfo(HitFlashName, MoveTemp(HitFlashInfo));
}

void AActionCharBase::BeginPlay()
{
	Super::BeginPlay();

	bDead = HPComponent->IsZero();
	HPComponent->OnHPBecomeZero.AddDynamic(this, &AActionCharBase::OnStartedDying);

	SpriteMaterialDynamic = GetSprite()->CreateDynamicMaterialInstance(0);
	check(SpriteMaterialDynamic);

	InitialFallingLateralFriction = GetCharacterMovement()->FallingLateralFriction;
}

void AActionCharBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (EndPlayReason == EEndPlayReason::Destroyed)
	{
		PlayDestructionVFX();
	}
}

float AActionCharBase::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// Do not modify damage parameters after this.
	const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	if (ActualDamage > 0.f)
	{
		AActor* DamageInstigator = DamageCauser ? DamageCauser->GetInstigator() : nullptr;
		HPComponent->Injure(Damage, DamageInstigator);
		
		OnInvinciblized(DamagedInvisibleTime);
		OnHurt();
	}

	return ActualDamage;
}

void AActionCharBase::FellOutOfWorld(const UDamageType& dmgType)
{
	// Don't use the Super version.
	if (!bDead)
	{
		HPComponent->SetHP(0.f);
	}
}

void AActionCharBase::OnStartedDying(AActor* Causer)
{
	bDead = true;

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (DyingTime > 0.f)
	{
		GetWorldTimerManager().SetTimer(DyingTimer, this, &ThisClass::OnFinishedDying, DyingTime, false);
	}
	else
	{
		OnFinishedDying();
	}
}

void AActionCharBase::OnFinishedDying()
{
	Destroy();
}

void AActionCharBase::OnHurt()
{
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
	}

	if (HurtTime > 0.f)
	{
		bHurt = true;
		if (UPaperZDAnimInstance* PaperAnimInstance = GetAnimInstance())
		{
			PaperAnimInstance->JumpToNode(TEXT("JumpHurt"));
		}
		GetWorldTimerManager().SetTimer(FinishHurtTimer, this, &AActionCharBase::FinishHurt, HurtTime, false);
	}

	FlashComponent->PlayFlashFromStart(HitFlashName);
}

void AActionCharBase::FinishStop()
{
	RestoreFallingLateralFriction();
	bStop = false;
}

void AActionCharBase::FinishHurt()
{
	bHurt = false;
}

void AActionCharBase::FinishInvincible()
{
	bInvincible = false;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
}

void AActionCharBase::OnKnockbacked(float KnockbackTime)
{
	if (KnockbackTime > 0.f)
	{
		bStop = true;
		GetCharacterMovement()->FallingLateralFriction = 0.f;
		GetWorldTimerManager().SetTimer(FinishStopTimer, this, &AActionCharBase::FinishStop, KnockbackTime, false);
	}
}

void AActionCharBase::OnInvinciblized(float InInvisibleTime)
{
	if (InInvisibleTime > 0.f)
	{
		bInvincible = true;
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		GetWorldTimerManager().SetTimer(FinishInvincibleTimer, this, &AActionCharBase::FinishInvincible, InInvisibleTime, false);
	}
}

void AActionCharBase::RestoreFallingLateralFriction()
{
	GetCharacterMovement()->FallingLateralFriction = InitialFallingLateralFriction;
}

void AActionCharBase::PlayDestructionVFX()
{
	if (DestructionVfxClass == nullptr) return;

	UWorld* World = GetWorld();

	FActorSpawnParameters Params;
	Params.bNoFail = true;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	APaperDestructionVFX* DestructionVFX = World->SpawnActor<APaperDestructionVFX>(DestructionVfxClass.Get(), GetActorTransform(), Params);
}