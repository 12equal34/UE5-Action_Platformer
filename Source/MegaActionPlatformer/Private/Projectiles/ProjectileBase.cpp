

#include "Projectiles/ProjectileBase.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "PaperFlipbookComponent.h"

AProjectileBase::AProjectileBase()
{
	PrimaryActorTick.bCanEverTick = true;

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	check(SphereComponent != nullptr);
	SetRootComponent(SphereComponent);

	PaperFlipbookComponent = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("PaperFlipbook"));
	check(PaperFlipbookComponent != nullptr);
	PaperFlipbookComponent->SetupAttachment(SphereComponent);
	PaperFlipbookComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	check(ProjectileMovementComponent != nullptr);
	ProjectileMovementComponent->ProjectileGravityScale = 0.f;
	ProjectileMovementComponent->MaxSpeed     = 1000.f;
	ProjectileMovementComponent->InitialSpeed = 1000.f;
}

void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void AProjectileBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

