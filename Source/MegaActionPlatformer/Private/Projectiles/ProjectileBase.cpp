
#include "Projectiles/ProjectileBase.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "PaperFlipbookComponent.h"
#include "Kismet/GameplayStatics.h"
#include "VFX/PaperProjectileVFX.h"
#include "Engine/World.h"
#include "Factions/ActionFactionComponent.h"
#include "Characters/ActionCharBase.h"

DEFINE_LOG_CATEGORY(LogProjectile);

AProjectileBase::AProjectileBase()
{
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	check(SphereComponent != nullptr);
	SetRootComponent(SphereComponent);
	SphereComponent->SetCollisionProfileName(TEXT("Projectile"));

	PaperFlipbookComponent = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("PaperFlipbook"));
	check(PaperFlipbookComponent != nullptr);
	PaperFlipbookComponent->SetupAttachment(SphereComponent);
	PaperFlipbookComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	check(ProjectileMovementComponent != nullptr);
	ProjectileMovementComponent->ProjectileGravityScale = 0.f;
	ProjectileMovementComponent->MaxSpeed     = 1000.f;
	ProjectileMovementComponent->InitialSpeed = 1000.f;

	FactionComponent = CreateDefaultSubobject<UActionFactionComponent>(TEXT("Faction"));
	check(FactionComponent);

	InitialLifeSpan = 3.f;
}

void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();
	
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, & AProjectileBase::OnSphereBeginOverlap);
	SphereComponent->OnComponentHit.AddDynamic(this, &AProjectileBase::OnSphereHit);
}

void AProjectileBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (EndPlayReason == EEndPlayReason::Destroyed)
	{
		PlayDespawnVFX();
	}
}

void AProjectileBase::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor,UPrimitiveComponent* OtherComp,int32 OtherBodyIndex,bool bFromSweep,const FHitResult& SweepResult)
{
	AActionCharBase* ActionChar = Cast<AActionCharBase>(OtherActor);
	if (ActionChar)
	{
		OnOverlapPlayerOrEnemy(*ActionChar);
	}
}

void AProjectileBase::OnSphereHit(UPrimitiveComponent* HitComponent,AActor* OtherActor,UPrimitiveComponent* OtherComp,FVector NormalImpulse,const FHitResult& Hit)
{
	check(OtherActor != nullptr);
	
	OnHitStructure(*OtherActor);
}

void AProjectileBase::OnOverlapPlayerOrEnemy(AActionCharBase& ActionChar)
{
	const UActionFactionComponent& MyFaction = *GetFactionComponent();
	const UActionFactionComponent& CharacterFaction = *ActionChar.GetFactionComponent();
	const bool bHostile = MyFaction.IsHostile(CharacterFaction);
	if (bHostile)
	{
		ActionChar.Destroy();

		verifyf(Destroy() == true, TEXT("The projectile is indestructive."));
	}
	else
	{

	}
}

void AProjectileBase::OnHitStructure(AActor& Structure)
{
	UE_LOG(LogProjectile, Display, TEXT("%s hits %s."), *GetName(), *Structure.GetName());

	verifyf(Destroy() == true, TEXT("The projectile is indestructive."));
}

void AProjectileBase::PlayDespawnVFX()
{
	checkf(DespawnVfxClass != nullptr, TEXT("The DespawnVfxClass property of %s is NOT set."), *GetClass()->GetName());
	
	UWorld* World = GetWorld();
	
	FActorSpawnParameters Params;
	Params.bNoFail = true;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	APaperProjectileVFX* DespawnVFX = World->SpawnActor<APaperProjectileVFX>(DespawnVfxClass.Get(), GetActorTransform(), Params);
}
