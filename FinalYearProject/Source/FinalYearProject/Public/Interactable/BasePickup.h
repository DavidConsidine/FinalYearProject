// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupInterface.h"

#include "BasePickup.generated.h"

class USceneComponent;
class UStaticMeshComponent;

UCLASS()
class FINALYEARPROJECT_API ABasePickup : public AActor, public IPickupInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABasePickup();


public:	

	//////////////////////////////////////////////////////////
	// Functions from IPickupInterface
	virtual void Grab(USceneComponent* AttachToComp) override;

	virtual void Drop() override;
	//////////////////////////////////////////////////////////

	void SetGrabbed(bool Grabbed);

	bool IsGrabbed();
	
protected:
	UPROPERTY(VisibleDefaultsOnly, Category = "Mesh Component")
	UStaticMeshComponent* MeshComp;

	bool bGrabbed;
};
