// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VRTeleportCursor.generated.h"

class UStaticMeshComponent;

UCLASS()
class FINALYEARPROJECT_API AVRTeleportCursor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVRTeleportCursor();

protected:

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "CursorMeshes")
	UStaticMeshComponent* CursorImpactMeshComp;

	bool bVisible;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

	void SetVisible(const bool Visibility);

	bool IsVisible();

	void UpdateCursor(const FVector& ImpactLocation);
	
};
