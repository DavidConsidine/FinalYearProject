// Fill out your copyright notice in the Description page of Project Settings.

#include "VRShoppingBasket.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"


// Sets default values
AVRShoppingBasket::AVRShoppingBasket()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComp"));
	RootComponent = StaticMeshComp;

	BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComp"));
	BoxComp->SetupAttachment(RootComponent);

	// set up volume overlap events
	BoxComp->OnComponentBeginOverlap.AddDynamic(this, &AVRShoppingBasket::OnOverlapBegin);
	BoxComp->OnComponentEndOverlap.AddDynamic(this, &AVRShoppingBasket::OnOverlapEnd);
}

// Called when the game starts or when spawned
void AVRShoppingBasket::BeginPlay()
{
	Super::BeginPlay();
	
}

void AVRShoppingBasket::OnOverlapBegin(UPrimitiveComponent * OverlappedComp, AActor * OtherActor,
	UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
}

void AVRShoppingBasket::OnOverlapEnd(UPrimitiveComponent * OverlappedComp, AActor * OtherActor,
	UPrimitiveComponent * OtherComp, int32 OtherBodyIndex)
{
}

// Called every frame
void AVRShoppingBasket::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

