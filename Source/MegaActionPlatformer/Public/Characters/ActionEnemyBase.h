// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/ActionCharBase.h"
#include "ActionEnemyBase.generated.h"

UCLASS()
class MEGAACTIONPLATFORMER_API AActionEnemyBase : public AActionCharBase
{
	GENERATED_BODY()
public:
	AActionEnemyBase();

protected:
	//~ Begin AActor Interface.
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor Interface.

	UFUNCTION(BlueprintNativeEvent)
	void OnPlayerTouched(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor,UPrimitiveComponent* OtherComp,int32 OtherBodyIndex,bool bFromSweep,const FHitResult& SweepResult);

	virtual void OnPlayerTouched_Implementation(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor,UPrimitiveComponent* OtherComp,int32 OtherBodyIndex,bool bFromSweep,const FHitResult& SweepResult);

private:
	UPROPERTY(Category=Combat,VisibleAnywhere)
	TObjectPtr<class UDamageComponent> TouchDamageComponent;
};
