// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VRController.generated.h"


////////////////////////////////////////////
// FORWARD DECLARATIONS
class UMotionControllerComponent;
class UStaticMeshComponent;
class USphereComponent;
class ABasePickup;

UCLASS()
class FINALYEARPROJECT_API AVRController : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVRController();

public:

protected:

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "MotionController")
	UMotionControllerComponent* MotionControllerComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MotionController")
	UStaticMeshComponent* MotionControllerMeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SphereComponent")
	USphereComponent* SphereComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SphereComponent")
	float SphereRadius;

	ABasePickup* CurrentPickupObject;

	// flags to inform state of animation
	UPROPERTY(BlueprintReadOnly, Category = "Animation State")
	bool bTeleporting;

	UPROPERTY(BlueprintReadOnly, Category = "Animation State")
	bool bGrabbing;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	ABasePickup* GetObjectNearestToHand();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	void SetHand(EControllerHand Hand);
	
	FVector GetControllerRelativeLocation();

	FVector GetControllerForwardVector();

	void GrabObject();

	void DropObject();

	// setter functions for anim state
	void SetTeleporting(bool Teleporting);

	void SetGrabbing(bool Grabbing);

	// getter functions for anim state
	bool GetTeleporting();

	bool GetGrabbing();
};
