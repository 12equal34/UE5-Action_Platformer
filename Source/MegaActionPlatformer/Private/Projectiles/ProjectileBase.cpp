
#include "Projectiles/ProjectileBase.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "PaperFlipbookComponent.h"
#include "Kismet/GameplayStatics.h"
#include "VFX/PaperProjectileVFX.h"
#include "VFX/LogVFX.h"
#include "Engine/World.h"
#include "Factions/ActionFactionComponent.h"
#include "Characters/ActionCharBase.h"
#include "Combat/DamageComponent.h"

DEFINE_LOG_CATEGORY(LogProjectile);

AProjectileBase::AProjectileBase()
{
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	check(SphereComponent != nullptr);
	SetRootComponent(SphereComponent);
	SphereComponent->SetCollisionProfileName(TEXT("Projectile"));

	Sprite = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("Projectile"));
	check(Sprite != nullptr);
	Sprite->SetupAttachment(SphereComponent);
	Sprite->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	check(ProjectileMovementComponent != nullptr);
	ProjectileMovementComponent->ProjectileGravityScale = 0.f;
	ProjectileMovementComponent->MaxSpeed     = 1000.f;
	ProjectileMovementComponent->InitialSpeed = 1000.f;

	FactionComponent = CreateDefaultSubobject<UActionFactionComponent>(TEXT("Faction"));
	check(FactionComponent);

	DamageComponent = CreateDefaultSubobject<UDamageComponent>(TEXT("Damage"));
	check(DamageComponent);

	InitialLifeSpan = 3.f;
}

void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();
	
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, & AProjectileBase::OnSphereBeginOverlap);
	SphereComponent->OnComponentHit.AddDynamic(this, &AProjectileBase::OnSphereHit);

	if (BeginPlaySound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, BeginPlaySound, GetActorLocation());
	}
}

void AProjectileBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (EndPlayReason == EEndPlayReason::Destroyed)
	{
		PlayDestructVFX();
	}
}

void AProjectileBase::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor,UPrimitiveComponent* OtherComp,int32 OtherBodyIndex,bool bFromSweep,const FHitResult& SweepResult)
{
	AActionCharBase* ActionChar = Cast<AActionCharBase>(OtherActor);
	if (ActionChar)
	{
		ApplyDamageTo(*ActionChar);
	}
}

void AProjectileBase::OnSphereHit(UPrimitiveComponent* HitComponent,AActor* OtherActor,UPrimitiveComponent* OtherComp,FVector NormalImpulse,const FHitResult& Hit)
{
	check(OtherActor != nullptr);
	UE_LOG(LogProjectile, Display, TEXT("%s hits %s."), *GetName(), *OtherActor->GetName());
	Destroy();
}

void AProjectileBase::ApplyDamageTo(AActionCharBase& ActionChar)
{
	const UActionFactionComponent& MyFaction = *GetFactionComponent();
	const UActionFactionComponent& CharacterFaction = *ActionChar.GetFactionComponent();
	const bool bHostile = MyFaction.IsHostile(CharacterFaction);
	if (bHostile)
	{
		DamageComponent->ApplyDamage(ActionChar);
		Destroy();
	}
}

void AProjectileBase::PlayDestructVFX()
{
	if (DestructVfxClass)
	{
		FActorSpawnParameters Params;
		Params.bNoFail = true;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		APaperProjectileVFX* DespawnVFX = GetWorld()->SpawnActor<APaperProjectileVFX>(DestructVfxClass.Get(), GetActorTransform(), Params);
	}
	else
	{
		UE_LOG(LogVFX, Warning, TEXT("The %s::DestructVfxClass is NOT set."), *GetClass()->GetName() );
	}
}
