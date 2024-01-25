
#include "Projectiles/ProjectileBase.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "PaperFlipbookComponent.h"

DEFINE_LOG_CATEGORY(LogProjectile);

AProjectileBase::AProjectileBase()
{
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	check(SphereComponent != nullptr);
	SetRootComponent(SphereComponent);
	SphereComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SphereComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);

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
	
	SphereComponent->OnComponentHit.AddDynamic(this, &AProjectileBase::OnSphereHit);
}

void AProjectileBase::OnSphereHit(UPrimitiveComponent* HitComponent,AActor* OtherActor,UPrimitiveComponent* OtherComp,FVector NormalImpulse,const FHitResult& Hit)
{
	check(OtherActor != nullptr);
	UE_LOG(LogProjectile, Display, TEXT("%s hits %s."), *GetName(), *OtherActor->GetName());

	const bool bDestroy = Destroy();
	if (bDestroy)
	{
		UE_LOG(LogProjectile, Display, TEXT("%s is destroyed."), *GetName());
	}
	else
	{
		UE_LOG(LogProjectile, Display, TEXT("Trying to destroy %s is failed."), *GetName());
	}
}
