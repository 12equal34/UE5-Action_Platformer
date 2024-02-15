// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CameraRestrictor.generated.h"

UCLASS()
class MEGAACTIONPLATFORMER_API ACameraRestrictor : public AActor
{
	GENERATED_BODY()
	
public:	
	ACameraRestrictor();

	FORCEINLINE FVector GetCamreaPosition() const;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnCameraBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnCameraEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UBoxComponent> BoxComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class USceneComponent> CameraPosition;

#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UBillboardComponent> BillboardComponent;
#endif // WITH_EDITORONLY_DATA
};
