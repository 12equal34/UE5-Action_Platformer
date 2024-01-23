// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/ActionCharBase.h"
#include "ActionPlayerBase.generated.h"


UCLASS()
class MEGAACTIONPLATFORMER_API AActionPlayerBase : public AActionCharBase
{
	GENERATED_BODY()
public:
	AActionPlayerBase();

private:
	UPROPERTY(Category=Camera,VisibleAnywhere)
	TObjectPtr<class USpringArmComponent> SpringArmComponent;

	UPROPERTY(Category=Camera,VisibleAnywhere)
	TObjectPtr<class UCameraComponent> CameraComponent;
};
