// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/ActionCharBase.h"
#include "PaperFlipbookComponent.h"
#include "Components/CapsuleComponent.h"

AActionCharBase::AActionCharBase()
{
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaskedMaterialRef(TEXT("/Paper2D/MaskedLitSpriteMaterial.MaskedLitSpriteMaterial"));
	check(MaskedMaterialRef.Succeeded());

	UPaperFlipbookComponent* MySprite = GetSprite();
	check(MySprite);
	MySprite->SetRelativeLocation(FVector(-5.f, 0.f, 13.9f));
	MySprite->SetMaterial(0, MaskedMaterialRef.Object);
	MySprite->SetCastShadow(true);

	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	check(CapsuleComp);
	CapsuleComp->SetCapsuleHalfHeight(60.f);
}
