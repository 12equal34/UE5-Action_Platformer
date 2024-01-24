// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectileBase.generated.h"

class USphereComponent;
class UPaperFlipbookComponent;
class UProjectileMovementComponent;

UCLASS()
class MEGAACTIONPLATFORMER_API AProjectileBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectileBase();

protected:
	virtual void BeginPlay() override;

	FORCEINLINE USphereComponent* GetSphere() const { return SphereComponent; }
	FORCEINLINE UPaperFlipbookComponent* GetPaperFlipbook() const { return PaperFlipbookComponent; }
	FORCEINLINE UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovementComponent; }

public:	
	virtual void Tick(float DeltaTime) override;


private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> SphereComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPaperFlipbookComponent> PaperFlipbookComponent;

	UPROPERTY(Category=Movement,VisibleAnywhere)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;
};
