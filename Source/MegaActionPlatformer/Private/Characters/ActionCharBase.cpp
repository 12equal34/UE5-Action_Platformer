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

AActionCharBase::AActionCharBase()
{
	UPaperFlipbookComponent* MySprite = GetSprite();
	check(MySprite);
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaskedMaterialRef(TEXT("/Game/MegaActionPlatformer/Materials/MI_CustomLitSpriteMaterial.MI_CustomLitSpriteMaterial"));
	check(MaskedMaterialRef.Succeeded());
	MySprite->SetMaterial(0, MaskedMaterialRef.Object);
	MySprite->SetCastShadow(true);

	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	check(CapsuleComp);
	CapsuleComp->SetCapsuleHalfHeight(60.f);
	CapsuleComp->SetCollisionProfileName(TEXT("ActionChar"));

	UCharacterMovementComponent* CharMovement = GetCharacterMovement();
	check(CharMovement);
	CharMovement->SetPlaneConstraintEnabled(true);
	CharMovement->SetPlaneConstraintAxisSetting(EPlaneConstraintAxisSetting::Y);
	CharMovement->bUseFlatBaseForFloorChecks = true;
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

	OnTakeAnyDamage.AddDynamic(this, &AActionCharBase::OnAppliedAnyDamage);

	HPComponent->OnHPBecameZero.BindUObject(this, &AActionCharBase::OnStartedDying);
	bDead = HPComponent->IsLeft();

	SpriteMaterialDynamic = GetSprite()->CreateDynamicMaterialInstance(0);
	check(SpriteMaterialDynamic);

	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &AActionCharBase::OnCapsuleBeginOverlap);

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
	return Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
}

void AActionCharBase::OnAppliedAnyDamage(AActor* DamagedActor,float Damage,const UDamageType* DamageType,AController* InstigatedBy,AActor* DamageCauser)
{
	if (Damage > 0.f)
	{
		HPComponent->Injure(Damage);
		FlashComponent->PlayFlashFromStart(HitFlashName);
		OnInvinciblized(DamagedInvisibleTime);
	}
}

void AActionCharBase::OnStartedDying()
{
	bDead = true;
	OnFinishedDying();
}

void AActionCharBase::OnFinishedDying()
{
	Destroy();
}

void AActionCharBase::FinishStop()
{
	RestoreFallingLateralFriction();
	bStop = false;
	bHurt = false;
}

void AActionCharBase::FinishInvincible()
{
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	bInvincible = false;
}

void AActionCharBase::OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor,UPrimitiveComponent* OtherComp,int32 OtherBodyIndex,bool bFromSweep,const FHitResult& SweepResult)
{
	if (AActionCharBase* OtherActionChar = Cast<AActionCharBase>(OtherActor))
	{
		OnActionCharBeginOverlap(*OtherActionChar);
	}
}

void AActionCharBase::OnKnockbacked(float KnockbackTime)
{
	GetCharacterMovement()->FallingLateralFriction = 0.f;
	bStop = true;

	GetAnimInstance()->JumpToNode(TEXT("JumpHurt"));
	bHurt = true;

	check(KnockbackTime > 0.f);
	GetWorldTimerManager().SetTimer(FinishStopTimer, this, &AActionCharBase::FinishStop, KnockbackTime, false);
}

void AActionCharBase::OnInvinciblized(float InInvisibleTime)
{
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	bInvincible = true;
	check(InInvisibleTime > 0.f);
	GetWorldTimerManager().SetTimer(FinishInvincibleTimer, this, &AActionCharBase::FinishInvincible, InInvisibleTime, false);
}

void AActionCharBase::OnActionCharBeginOverlap(AActionCharBase& OtherActionChar)
{
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