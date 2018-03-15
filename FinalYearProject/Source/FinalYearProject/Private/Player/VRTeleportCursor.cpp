// Fill out your copyright notice in the Description page of Project Settings.

#include "VRTeleportCursor.h"
#include "Components/StaticMeshComponent.h"


// Sets default values
AVRTeleportCursor::AVRTeleportCursor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bVisible = false;

	CursorImpactMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CursorImpactMeshComp"));
	CursorImpactMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CursorImpactMeshComp->SetVisibility(bVisible);
	CursorImpactMeshComp->CastShadow = 0;
	RootComponent = CursorImpactMeshComp;
}

// Called when the game starts or when spawned
void AVRTeleportCursor::BeginPlay()
{
	Super::BeginPlay();
	
}



void AVRTeleportCursor::SetVisible(const bool Visibility)
{
	bVisible = Visibility;

	CursorImpactMeshComp->SetVisibility(bVisible);

}

bool AVRTeleportCursor::IsVisible()
{
	return bVisible;
}

void AVRTeleportCursor::UpdateCursor(const FVector & ImpactLocation)
{
	// TODO: check if offset need to avoid mesh colliding with surface
	float Offset = 55.f; // test values
	FVector NewLocation = FVector(ImpactLocation.X, ImpactLocation.Y, ImpactLocation.Z + Offset);
	SetActorLocation(NewLocation, true);
	
	float x = NewLocation.X;
	float y = NewLocation.Y;
	float z = NewLocation.Z;
}

