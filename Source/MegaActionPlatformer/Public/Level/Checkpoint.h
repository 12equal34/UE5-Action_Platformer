// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Checkpoint.generated.h"

class USceneComponent;
class UBoxComponent;

UCLASS()
class MEGAACTIONPLATFORMER_API ACheckpoint : public AActor
{
	GENERATED_BODY()
	
public:	
	ACheckpoint();

	FORCEINLINE UBoxComponent* GetTriggerBox() const { return BoxComponent; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE bool IsChecked() const { return bCheck; }

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintNativeEvent)
	void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> RootTransform;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> PlayerStartSpawnTransform;

	UPROPERTY(Category=Trigger,VisibleAnywhere)
	TObjectPtr<UBoxComponent> BoxComponent;

	UPROPERTY(Category=Checkpoint,VisibleInstanceOnly)
	bool bCheck;
};
