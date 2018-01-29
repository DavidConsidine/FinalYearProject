// Fill out your copyright notice in the Description page of Project Settings.

#include "VRTeleportCursorComponent.h"
#include "Components/StaticMeshComponent.h"


// Sets default values for this component's properties
UVRTeleportCursorComponent::UVRTeleportCursorComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	//PrimaryComponentTick.bCanEverTick = false;

	//CursorLineMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CursorLineMeshComp"));
	//CursorLineMeshComp->SetWorldScale3D(FVector(0.2f, 0.2f, 1.0f));
	CursorImpactMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CursorImpactMeshComp"));

}

void UVRTeleportCursorComponent::SetVisibility(bool Visibility)
{
	bVisible = Visibility;
	if (bVisible)
	{
		//CursorLineMeshComp->SetVisibility(true);
		CursorImpactMeshComp->SetVisibility(true);
		UE_LOG(LogTemp, Warning, TEXT("Impact Mesh Visibility: true"));
	}
	else
	{
		//CursorLineMeshComp->SetVisibility(false);
		CursorImpactMeshComp->SetVisibility(false);
		UE_LOG(LogTemp, Warning, TEXT("Impact Mesh Visibility: false"));
	}
}

bool UVRTeleportCursorComponent::GetVisibility()
{
	return bVisible;
}

void UVRTeleportCursorComponent::UpdateCursor(const FVector & ImpactLocation)
{
	// TODO: check if offset need to avoid mesh colliding with surface
	float Offset = 55.f; // test values
	FVector NewLocation = FVector(ImpactLocation.X, ImpactLocation.Y, ImpactLocation.Z + Offset);
	CursorImpactMeshComp->SetWorldLocation(NewLocation);
	

	float x = NewLocation.X;
	float y = NewLocation.Y;
	float z = NewLocation.Z;

	UE_LOG(LogTemp, Warning, TEXT("Impact Mesh location updated to : %.2f, %.2f, %.2f"), x, y, z);
}

//void UVRTeleportCursorComponent::UpdateCursor(const FVector& StartPosition, const FVector& EndLocation, const FRotator& Orientation, const float CursorLength)
//{
//	CursorLineMeshComp->SetWorldScale3D(FVector(0.2f, 0.2f, 1.0f * CursorLength));
//	CursorLineMeshComp->SetWorldLocation(StartPosition);
//	CursorLineMeshComp->SetWorldRotation(Orientation);
//	CursorImpactMeshComp->SetWorldLocation(EndLocation);
//}
