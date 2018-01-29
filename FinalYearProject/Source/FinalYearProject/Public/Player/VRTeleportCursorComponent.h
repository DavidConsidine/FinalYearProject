// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "VRTeleportCursorComponent.generated.h"

class UStaticMeshComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FINALYEARPROJECT_API UVRTeleportCursorComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UVRTeleportCursorComponent();

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Cursor Meshes")
	UStaticMeshComponent* CursorImpactMeshComp;
	
	bool bVisible;

public:
	void SetVisibility(bool Visibility);

	bool GetVisibility();


	//void UpdateCursor(const FVector& StartPosition, const FVector& EndLocation, const FRotator& Orientation, const float CursorLength);
	// focus on just one static mesh for now( only need impact location to set down mesh)
	void UpdateCursor(const FVector& ImpactLocation);
		
	
};
