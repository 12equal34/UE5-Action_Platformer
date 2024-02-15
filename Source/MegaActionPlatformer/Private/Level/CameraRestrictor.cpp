// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/CameraRestrictor.h"
#include "Components/BoxComponent.h"
#include "Characters/ActionPlayerBase.h"
#include "Components/BillboardComponent.h"

ACameraRestrictor::ACameraRestrictor()
{
	PrimaryActorTick.bCanEverTick = false;

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CameraDetector"));
	check(BoxComponent);
	SetRootComponent(BoxComponent);
	BoxComponent->SetBoxExtent(FVector(400.f, 100.f, 300.f));
	BoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BoxComponent->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel3);
	BoxComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	BoxComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	CameraPosition = CreateDefaultSubobject<USceneComponent>(TEXT("CameraPosition"));
	check(CameraPosition);
	CameraPosition->SetupAttachment(BoxComponent);

#if WITH_EDITORONLY_DATA
	BillboardComponent = CreateDefaultSubobject<UBillboardComponent>(TEXT("CameraPosBillboard"));
	BillboardComponent->SetupAttachment(CameraPosition);
#endif // WITH_EDITORONLY_DATA
}

FVector ACameraRestrictor::GetCamreaPosition() const
{
	return CameraPosition->GetComponentLocation();
}

void ACameraRestrictor::BeginPlay()
{
	Super::BeginPlay();
	
	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &ACameraRestrictor::OnCameraBeginOverlap);
	BoxComponent->OnComponentEndOverlap.AddDynamic(this, &ACameraRestrictor::OnCameraEndOverlap);
}

void ACameraRestrictor::OnCameraBeginOverlap(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor,UPrimitiveComponent* OtherComp,int32 OtherBodyIndex,bool bFromSweep,const FHitResult& SweepResult)
{
	if (AActionPlayerBase* PlayerChar = Cast<AActionPlayerBase>(OtherActor))
	{
		PlayerChar->SetOverlappingCameraRestrictor(this);
	}
}

void ACameraRestrictor::OnCameraEndOverlap(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor,UPrimitiveComponent* OtherComp,int32 OtherBodyIndex)
{
	if (AActionPlayerBase* PlayerChar = Cast<AActionPlayerBase>(OtherActor))
	{
		PlayerChar->SetOverlappingCameraRestrictor(nullptr);
	}
}
