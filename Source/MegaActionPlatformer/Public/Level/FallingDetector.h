// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FallingDetector.generated.h"

UCLASS()
class MEGAACTIONPLATFORMER_API AFallingDetector : public AActor
{
	GENERATED_BODY()
	
public:	
	AFallingDetector();

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintNativeEvent)
	void OnCharacterBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UBoxComponent> BoxComponent;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UDamageType> DamageTypeClass;

	UPROPERTY(Category=FallingDetector, EditDefaultsOnly)
	float FallingDamage = 9999999.f;
};
