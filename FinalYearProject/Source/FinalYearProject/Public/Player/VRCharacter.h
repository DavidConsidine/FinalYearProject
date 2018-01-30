// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "VRCharacter.generated.h"

//////////////////////////////////////////////////////
// Enumerations

/** State machine enum for managing Teleportation */
UENUM()
enum ETeleportState {
	Wait = 0,
	Aiming,
	FadeOut,
	Teleport,
	FadeIn,
};

/** States of active motion controllers */
UENUM()
enum EMControllerGestureActiveState {
	Both = 0,
	Left,
	Right
};

///////////////////////////////////////////////////////////
// Forward Declarations
class USkeletalMeshComponent;
class UInputComponent;
class UCameraComponent;
class UMotionControllerComponent;
class UVRTeleportCursorComponent;
class AVRTeleportCursor;
class AVRController;
class USceneComponent;
class UUserWidget;

UCLASS()
class FINALYEARPROJECT_API AVRCharacter : public ACharacter
{
	GENERATED_BODY()

protected:

	UPROPERTY(VisibleDefaultsOnly, Category = "VR")
	USceneComponent* VROrigin;

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* Mesh1P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* CameraComp;

	///** Motion controller (right hand) */	// AllowPrivateAccess: If true, properties defined in the C++ private scope will be accessible to blueprints
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	//UMotionControllerComponent* R_MotionController;

	///** Motion controller (left hand) */
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	//UMotionControllerComponent* L_MotionController;

	/** Teleport cursor */
	/*UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UVRTeleportCursorComponent* TeleportCursorComp;*/

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TeleportCursor")
	TSubclassOf<AVRTeleportCursor> TeleportCursorClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	AVRTeleportCursor* TeleportCursor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MotionController")
	TSubclassOf<AVRController> MotionControllerClass;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "MotionController")
	AVRController* VRController_L;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "MotionController")
	AVRController* VRController_R;

	// fade in screen
	TSubclassOf<UUserWidget> WidgetClass;

	UUserWidget* ScreenFadeWidget;





protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float DefaultPlayerHeight;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;

	/** Current Teleport state machine */
	ETeleportState TelState;

	/** Stores currently selected position to move player to */
	FVector CurrentTeleportPosition;

	/** Stores previously selected position to move player to */
	FVector PreviousTeleportPosition;

	bool bTeleporting;

	bool bValidTeleportPosition;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Teleport")
	float MaxTeleportDistance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Teleport")
	float FadeCoefficient;

public:
	// Sets default values for this character's properties
	AVRCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/** Resets HMD orientation and position in VR. */
	void OnResetVR();

	/////////////////////////////////////////
	// Teleport-related functions
	/**  */
	void StartTeleport();

	/** */
	void StopTeleport();

	void CancelTeleport();

	/** Update current teleport state */
	void SetTelState(ETeleportState NewState);

	/** Fires a projectile. */
	void OnTeleport();

	/** called to update teleport visual aid */
	void UpdateTeleportCursor();

	bool CheckValidTeleportLocation();

	/** Perform fade in/out */
	bool DoScreenFade(bool FadeOut);

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

	/**
	* Called via input to turn at a given rate.
	* @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	*/
	void TurnAtRate(float Rate);

	/**
	* Called via input to turn look up/down at a given rate.
	* @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	*/
	void LookUpAtRate(float Rate);

public:	
	
	/** Returns Mesh1P subobject **/
	//FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return CameraComp; }
	
};
