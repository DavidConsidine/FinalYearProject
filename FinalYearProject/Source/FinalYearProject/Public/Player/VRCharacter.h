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
enum EMControllerGripActiveState {
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
class AVRController;
class USceneComponent;
class UUserWidget;
class UWidgetInteractionComponent;
class UBoxComponent;

UCLASS()
class FINALYEARPROJECT_API AVRCharacter : public ACharacter
{
	GENERATED_BODY()

protected:

	UPROPERTY(VisibleDefaultsOnly, Category = "VR")
	USceneComponent* VROrigin;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* CameraComp;

	/** trigger volume for overlapping pickup objects */
	UPROPERTY(EditDefaultsOnly, Category = "PickupTriggerVolume")
	UBoxComponent* PickupTriggerVolumeComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MotionController")
	TSubclassOf<AVRController> MotionControllerClass;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "MotionController")
	AVRController* VRController_L;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "MotionController")
	AVRController* VRController_R;

	// fade in screen
	TSubclassOf<UUserWidget> WidgetClass;

	UUserWidget* ScreenFadeWidget;

	// Mode Select widget
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MenuWidget")
	TSubclassOf<AActor> ModeSelectClass;

	// Pause Menu widget
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MenuWidget")
	TSubclassOf<AActor> PauseMenuClass;

	// Widget Interaction component for mode select widget
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "MenuWidget")
	UWidgetInteractionComponent* WidgetInteractionComp;

	AActor* ModeSelectMenu;

	AActor* PauseMenu;


protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float DefaultPlayerHeight;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;

	bool bCanMove;

	/** Current Teleport state machine */
	ETeleportState TelState;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Teleport")
	float FadeCoefficient;

	/** Should Left Motion Controller motion be tracked to update player movement */
	bool bIsLeftGripPressed;

	/** Should Right Motion Controller motion be tracked to update player movement */
	bool bIsRightGripPressed;

	/* Exposed player movement speed variable */
	UPROPERTY(EditAnywhere, Category = "MovementSpeed")
	float GripMovementSpeed;

	// store each controller's current/previous relative location
	/////////////////////////////////////////////////////////////
	/** Stores current controller position */
	FVector CurrentLeftMControllerPos;

	/** Stores current controller position */
	FVector CurrentRightMControllerPos;

	/** Stores Last captured controller position */
	FVector PreviousLeftMControllerPos;

	/** Stores Last captured controller position */
	FVector PreviousRightMControllerPos;

	////////////////////////////////////////////////////////////////

public:
	// Sets default values for this character's properties
	AVRCharacter();

	void SetCanMove(bool CanMove);

	void DisableMenuComponents();

	void EnableMenuComponents();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent * OverlappedComp, AActor * OtherActor,
		UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	// interacting with menu widget
	void Click();
	void Release();

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

	void CheckValidTeleportLocation();

	/** Perform fade in/out */
	bool DoScreenFade(bool FadeOut);

	void SetupPauseMenu();

	///////////////////////////////////////////

	// gesture movement related funtions
	////////////////////////////////////////////

	void LeftGripPressed();

	void LeftGripReleased();

	void RightGripPressed();

	void RightGripReleased();

	void CheckVRGestureMovement();

	/** Get Controller relative distance from root, since last update */
	float GetControllerDistance(EMControllerGripActiveState ActiveState);

	/** Get Controller direction vector */
	FVector GetControllerDirection(EMControllerGripActiveState ActiveState);

	/** Update Player movement by vector calculated from active Motion Controller(s) */
	void AddPlayerMovement(FVector ControllerVector);

	////////////////////////////////////////////

	// Object interaction functions
	////////////////////////////////////////

	void GrabLeft();
	void DropLeft();

	void GrabRight();
	void DropRight();
	////////////////////////////////////////

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

	void PauseGame();

public:	
	
	/** Returns Mesh1P subobject **/
	//FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return CameraComp; }
	
};
