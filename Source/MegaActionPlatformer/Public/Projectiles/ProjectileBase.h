// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectileBase.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogProjectile,Display,All);

class USphereComponent;
class UPaperFlipbookComponent;
class UProjectileMovementComponent;
class UActionFactionComponent;
class AActionCharBase;

UCLASS()
class MEGAACTIONPLATFORMER_API AProjectileBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectileBase();

	FORCEINLINE UActionFactionComponent* GetFactionComponent() const { return FactionComponent; }
protected:
	//~ Begin AActor Interface.
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor Interface.

	FORCEINLINE USphereComponent* GetSphere() const { return SphereComponent; }
	FORCEINLINE UPaperFlipbookComponent* GetPaperFlipbook() const { return PaperFlipbookComponent; }
	FORCEINLINE UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovementComponent; }

	/** When this projectile hits a player or a enemy, acts */
	virtual void OnOverlapPlayerOrEnemy(AActionCharBase& ActionChar);

	/** e.g. this projectile hits a wall. */
	virtual void OnHitStructure(AActor& Structure);

private:
	void PlayDespawnVFX();

	UFUNCTION()
	void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnSphereHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> SphereComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPaperFlipbookComponent> PaperFlipbookComponent;

	UPROPERTY(Category=Movement,VisibleAnywhere)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;

	UPROPERTY(Category=VFX,EditDefaultsOnly)
	TSubclassOf<class APaperProjectileVFX> DespawnVfxClass;

	UPROPERTY(Category=Faction,VisibleAnywhere)
	TObjectPtr<UActionFactionComponent> FactionComponent;
};
