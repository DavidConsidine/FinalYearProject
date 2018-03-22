// Fill out your copyright notice in the Description page of Project Settings.

#include "VRShoppingBasket.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Interactable/BasePickup.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
AVRShoppingBasket::AVRShoppingBasket()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComp"));
	RootComponent = StaticMeshComp;

	BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComp"));
	BoxComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BoxComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	BoxComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	BoxComp->SetupAttachment(RootComponent);
	BoxComp->OnComponentBeginOverlap.AddDynamic(this, &AVRShoppingBasket::OnOverlapBegin);

	
	UE_LOG(LogTemp, Warning, TEXT("ShoppingBasket constructor"));
	
}

// Called when the game starts or when spawned
void AVRShoppingBasket::BeginPlay()
{
	Super::BeginPlay();
	// set up volume overlap events
	
}

void AVRShoppingBasket::OnOverlapBegin(UPrimitiveComponent * OverlappedComp, AActor * OtherActor,
	UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("OnOverlapBegin"));
	ABasePickup* ShoppingItem = Cast<ABasePickup>(OtherActor);
	if (ShoppingItem != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Item added to basket"));
		ShoppingItem->AddedToBasket();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("no overlap with basepickup"));
	}
}


// Called every frame
void AVRShoppingBasket::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

