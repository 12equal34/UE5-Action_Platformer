// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/ActionCharBase.h"
#include "PaperFlipbookComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Factions/ActionFactionComponent.h"
#include "VFX/PaperDestructionVFX.h"

AActionCharBase::AActionCharBase()
{
	UPaperFlipbookComponent* MySprite = GetSprite();
	check(MySprite);
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaskedMaterialRef(TEXT("/Paper2D/MaskedLitSpriteMaterial.MaskedLitSpriteMaterial"));
	check(MaskedMaterialRef.Succeeded());
	MySprite->SetMaterial(0, MaskedMaterialRef.Object);
	MySprite->SetCastShadow(true);

	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	check(CapsuleComp);
	CapsuleComp->SetCapsuleHalfHeight(60.f);

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
}

void AActionCharBase::BeginPlay()
{
	Super::BeginPlay();
}

void AActionCharBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (EndPlayReason == EEndPlayReason::Destroyed)
	{
		PlayDestructionVFX();
	}
}

void AActionCharBase::PlayDestructionVFX()
{
	if (DestructionVfxClass == nullptr) return;
	//checkf(DestructionVfxClass != nullptr, TEXT("The DestructionVfxClass property of %s is NOT set."), *GetClass()->GetName());

	UWorld* World = GetWorld();

	FActorSpawnParameters Params;
	Params.bNoFail = true;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	APaperDestructionVFX* DestructionVFX = World->SpawnActor<APaperDestructionVFX>(DestructionVfxClass.Get(), GetActorTransform(), Params);
}
