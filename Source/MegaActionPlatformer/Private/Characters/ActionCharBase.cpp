// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/ActionCharBase.h"
#include "PaperFlipbookComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Factions/ActionFactionComponent.h"
#include "VFX/PaperDestructionVFX.h"
#include "Combat/HPComponent.h"
#include "VFX/FlashComponent.h"

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

	FactionComponent = CreateDefaultSubobject<UActionFactionComponent>(TEXT("Faction"));
	check(FactionComponent);

	static ConstructorHelpers::FClassFinder<APaperDestructionVFX> DestructionVfxClassRef(TEXT("/Game/MegaActionPlatformer/Blueprints/Characters/BP_DestructionVFX_BASE.BP_DestructionVFX_BASE_C"));
	check(DestructionVfxClassRef.Succeeded());
	DestructionVfxClass = DestructionVfxClassRef.Class;

	HPComponent = CreateDefaultSubobject<UHPComponent>(TEXT("HP"));
	check(HPComponent);

	HitFlashComponent = CreateDefaultSubobject<UFlashComponent>(TEXT("HitFlash"));
	check(HitFlashComponent);
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
		HitFlashComponent->PlayFromStart();
		OnInvinciblized();
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
	GetCharacterMovement()->FallingLateralFriction = InitialFallingLateralFriction;
	bStop = false;
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
	check(KnockbackTime > 0.f);
	GetWorldTimerManager().SetTimer(FinishStopTimer, this, &AActionCharBase::FinishStop, KnockbackTime, false);
}

void AActionCharBase::OnInvinciblized()
{
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	bInvincible = true;
	check(DamagedInvisibleTime > 0.f);
	GetWorldTimerManager().SetTimer(FinishInvincibleTimer, this, &AActionCharBase::FinishInvincible, DamagedInvisibleTime, false);
}

void AActionCharBase::OnActionCharBeginOverlap(AActionCharBase& OtherActionChar)
{
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