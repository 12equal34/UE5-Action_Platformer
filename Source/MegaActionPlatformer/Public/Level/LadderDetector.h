#pragma once 
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LadderDetector.generated.h"

UCLASS()
class MEGAACTIONPLATFORMER_API ALadderDetector : public AActor
{
	GENERATED_BODY()
public:	
	ALadderDetector();

protected:
	virtual void BeginPlay() override;

	UFUNCTION(Category="ALadderDetector",BlueprintNativeEvent)
	void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor,UPrimitiveComponent* OtherComp,int32 OtherBodyIndex,bool bFromSweep,const FHitResult& SweepResult);

	UFUNCTION(Category="ALadderDetector",BlueprintNativeEvent)
	void OnBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	virtual void OnBoxBeginOverlap_Implementation(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor,UPrimitiveComponent* OtherComp,int32 OtherBodyIndex,bool bFromSweep,const FHitResult& SweepResult);
	virtual void OnBoxEndOverlap_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UBoxComponent> BoxComponent;
};
