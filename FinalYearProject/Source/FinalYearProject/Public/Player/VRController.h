// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VRController.generated.h"


////////////////////////////////////////////
// FORWARD DECLARATIONS
class UMotionControllerComponent;
class USphereComponent;
class ABasePickup;
class USkeletalMeshComponent;
class AVRTeleportCursor;

UCLASS()
class FINALYEARPROJECT_API AVRController : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVRController();

public:

protected:

	// TeleportCursor related code. Uncomment when ready to move it
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TeleportCursor")
	TSubclassOf<AVRTeleportCursor> TeleportCursorClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	AVRTeleportCursor* TeleportCursor;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "MotionController")
	UMotionControllerComponent* MotionControllerComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MotionController")
	USkeletalMeshComponent* MotionControllerMeshComp;

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Teleport")
	float MaxTeleportDistance;

	/** Stores currently selected position to move player to */
	FVector CurrentTeleportPosition;

	/** Stores previously selected position to move player to */
	FVector PreviousTeleportPosition;

	bool bValidTeleportPosition;

	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	ABasePickup* GetObjectNearestToHand();

	void UpdateTeleportCursor();

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

	// teleport related code
	void CheckValidTeleportLocation();

	bool IsValidTeleportLocation();

	/*bool StopTeleport();*/

	void CancelTeleport();

	bool OnTeleport(FVector& OutTeleportLocation);
	
};
