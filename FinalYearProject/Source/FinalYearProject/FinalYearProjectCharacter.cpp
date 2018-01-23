// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "FinalYearProjectCharacter.h"
#include "FinalYearProjectProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"
#include "Blueprint/UserWidget.h"
#include "UObject/ConstructorHelpers.h"
#include "Image.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AFinalYearProjectCharacter

AFinalYearProjectCharacter::AFinalYearProjectCharacter()
{
	//** Instantiate object from blueprint reference path */
	ConstructorHelpers::FClassFinder<UUserWidget> BlueprintObj(TEXT("/Game/FirstPersonCPP/Blueprints/BP_FadeWidget"));
	if (!ensure(BlueprintObj.Class != nullptr))
	{
		return;
	}
	FadeWidgetSubclass = BlueprintObj.Class;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->RelativeLocation = FVector(-39.56f, 1.75f, 64.f); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->RelativeRotation = FRotator(1.9f, -19.19f, 5.2f);
	Mesh1P->RelativeLocation = FVector(-0.5f, -4.4f, -155.7f);

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P, FP_Gun, and VR_Gun 
	// are set in the derived blueprint asset named MyCharacter to avoid direct content references in C++.

	// Create VR Controllers.
	R_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("R_MotionController"));
	R_MotionController->Hand = EControllerHand::Right;
	R_MotionController->SetupAttachment(RootComponent);
	L_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("L_MotionController"));
	L_MotionController->SetupAttachment(RootComponent);

	// Create a gun and attach it to the right-hand VR controller.
	// Create a gun mesh component
	VR_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("VR_Gun"));
	VR_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	VR_Gun->bCastDynamicShadow = false;
	VR_Gun->CastShadow = false;
	VR_Gun->SetupAttachment(R_MotionController);
	VR_Gun->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	VR_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("VR_MuzzleLocation"));
	VR_MuzzleLocation->SetupAttachment(VR_Gun);
	VR_MuzzleLocation->SetRelativeLocation(FVector(0.000004, 53.999992, 10.000000));
	VR_MuzzleLocation->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));		// Counteract the rotation of the VR gun model.

	// Uncomment the following line to turn motion controllers on by default:
	//bUsingMotionControllers = true;
}

void AFinalYearProjectCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	
	/** Create widget and store it */
	if (FadeWidgetSubclass)
	{
		FadeWidget = CreateWidget<UUserWidget>(UGameplayStatics::GetPlayerController(GetWorld(), 0), FadeWidgetSubclass);
		if(FadeWidget != nullptr)
		{ 
			// TODO: Test in VR, make changes accordingly
			/** Add widget to player viewport */
			FadeWidget->AddToViewport();
		}
	}
	

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));

	// Show or hide the two versions of the gun based on whether or not we're using motion controllers.
	if (bUsingMotionControllers)
	{
		VR_Gun->SetHiddenInGame(false, true);
		Mesh1P->SetHiddenInGame(true, true);
	}
	else
	{
		VR_Gun->SetHiddenInGame(true, true);
		Mesh1P->SetHiddenInGame(false, true);
	}

	/** Set initial State of teleport state machine */
	SetTelState(ETeleportState::Wait);
	/** Initialise to a default (0-value Vector) */
	CurrentTeleportPosition = FVector(0);
}

void AFinalYearProjectCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//UE_LOG(LogTemp, Warning, TEXT("Player character Tick function, DeltaTime: %f "), DeltaTime);

	/** Fire projectiles at rate of fire */
	if (bFiring && TimeBetweenShotsFired >= FireRate)
	{
		OnFire();
		TimeBetweenShotsFired = 0.0f;
	}
	else
	{
		TimeBetweenShotsFired += DeltaTime;
	}

	
	/** Update Teleportation State Machine */
	switch (TelState)
	{
		// Default State. 
		case Wait:
			break;
		case FadeOut:
			if(DoScreenFade(true))
			{ 
				SetTelState(ETeleportState::Teleport);
			}
			break;
		case Teleport:
			OnTeleport(CurrentTeleportPosition);
			SetTelState(ETeleportState::FadeIn);
			break;
		case FadeIn:
			if (DoScreenFade(false))
			{
				// reset all variables
				CurrentTeleportPosition = FVector(0);
				SetTelState(ETeleportState::Wait);
			}
			break;
		default:
			break;
	}

	// update movement via gesture based movement
	if (bUsingMotionControllers)
	{
		CheckVRGestureMovement();
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AFinalYearProjectCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind teleport events
	PlayerInputComponent->BindAction("Teleport", IE_Released, this, &AFinalYearProjectCharacter::StartTeleport);

	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AFinalYearProjectCharacter::StartFiring);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AFinalYearProjectCharacter::StopFiring);


	// Bind VR Gesture Movement (Left Motion Controller) event
	PlayerInputComponent->BindAction("VR_GestureMovement_L", IE_Pressed, this, &AFinalYearProjectCharacter::SetIsActiveLeftMController);
	PlayerInputComponent->BindAction("VR_GestureMovement_L", IE_Released, this, &AFinalYearProjectCharacter::SetIsNotActiveLeftMController);
	// Bind VR Gesture Movement (Right Motion Controller) event
	PlayerInputComponent->BindAction("VR_GestureMovement_R", IE_Pressed, this, &AFinalYearProjectCharacter::SetIsActiveRightMController);
	PlayerInputComponent->BindAction("VR_GestureMovement_R", IE_Released, this, &AFinalYearProjectCharacter::SetIsNotActiveRightMController);

	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AFinalYearProjectCharacter::OnResetVR);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &AFinalYearProjectCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AFinalYearProjectCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AFinalYearProjectCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AFinalYearProjectCharacter::LookUpAtRate);
}

// Checks flag for input on firing weapon 
void AFinalYearProjectCharacter::StartFiring()
{
	bFiring = true;
}

// checks flag for end of input on firing weapon
void AFinalYearProjectCharacter::StopFiring()
{
	bFiring = false;
}

void AFinalYearProjectCharacter::StartTeleport()
{
	UE_LOG(LogTemp, Warning, TEXT("StartTeleport"))

	// Check for valid location to teleport to
	if (TelState == ETeleportState::Wait)
	{
		// if valid teleport position
		FHitResult RV_Hit(ForceInit);

		// Store a valid location to move player to and update Teleportation State Machine
		if (CheckValidTeleportLocation(RV_Hit))
		{
			// TODO: Validate location; checking for collisions/clipping with objects/environment
			CurrentTeleportPosition = RV_Hit.Location;
			UE_LOG(LogTemp, Warning, TEXT("StartTeleport - Valid Teleport start"))
			SetTelState(ETeleportState::FadeOut);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("StartTeleport : Invalid Teleport location"));
		}
	}
}

// set player's current state with regard to teleportation
void AFinalYearProjectCharacter::SetTelState(ETeleportState NewState)
{
	UE_LOG(LogTemp, Warning, TEXT("SetTelState: Setting State from %d to %d"), (int)TelState, (int)NewState)
	TelState = NewState;
}

// check flag to track motion controller for gesture based movement
void AFinalYearProjectCharacter::SetIsActiveLeftMController()
{
	bIsActiveLeftMController = true;
}

// check flag to stop tracking motion controller for gesture based movement
void AFinalYearProjectCharacter::SetIsNotActiveLeftMController()
{
	bIsActiveLeftMController = false;
}

// check flag to track motion controller for gesture based movement
void AFinalYearProjectCharacter::SetIsActiveRightMController()
{
	bIsActiveRightMController = true;
}

// check flag to stop tracking motion controller for gesture based movement
void AFinalYearProjectCharacter::SetIsNotActiveRightMController()
{
	bIsActiveRightMController = false;
}

void AFinalYearProjectCharacter::OnFire()
{
	// try and fire a projectile
	if (ProjectileClass != NULL)
	{
		UWorld* const World = GetWorld();
		if (World != NULL)
		{
			if (bUsingMotionControllers)
			{
				const FRotator SpawnRotation = VR_MuzzleLocation->GetComponentRotation();
				const FVector SpawnLocation = VR_MuzzleLocation->GetComponentLocation();
				World->SpawnActor<AFinalYearProjectProjectile>(ProjectileClass, SpawnLocation, SpawnRotation);
			}
			else
			{
				const FRotator SpawnRotation = GetControlRotation();
				// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
				const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

				//Set Spawn Collision Handling Override
				FActorSpawnParameters ActorSpawnParams;
				ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

				// spawn the projectile at the muzzle
				World->SpawnActor<AFinalYearProjectProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
			}
		}
	}

	// try and play the sound if specified
	if (FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// try and play a firing animation if specified
	if (FireAnimation != NULL)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}

//** Postition Player at valid location */
void AFinalYearProjectCharacter::OnTeleport(FVector TeleportPosition)
{
	if (TeleportPosition == FVector(0)) return;
	SetActorLocation(TeleportPosition);
}

/** Check if location to teleport to is within valid distance from the player */
bool AFinalYearProjectCharacter::CheckValidTeleportLocation(FHitResult & HitResult)
{
	FCollisionQueryParams RV_TraceParams = FCollisionQueryParams(FName(TEXT("RV_Trace")), true, this);
	FVector CamPos = FirstPersonCameraComponent->GetComponentLocation();
	FRotator CameraRot = FirstPersonCameraComponent->GetComponentRotation();
	FVector CamDir = FirstPersonCameraComponent->GetForwardVector();
	// TODO: Expose variable to the Editor
	const float PlayerInteractionDistance = 1000;
	FVector End = CamPos + (CameraRot.Vector() * PlayerInteractionDistance);

	RV_TraceParams.bTraceComplex = true;
	RV_TraceParams.bTraceAsyncScene = true;
	RV_TraceParams.bReturnPhysicalMaterial = true;

	//  do the line trace
	bool DidTrace = GetWorld()->LineTraceSingleByChannel(
		HitResult,        //result
		CamPos,        //start
		End,        //end
		ECC_Pawn,    //collision channel
		RV_TraceParams
	);
	return DidTrace;
}

bool AFinalYearProjectCharacter::DoScreenFade(bool FadeOut)
{
	// Get reference to Blueprint's Image component
	auto WidgetImage = FadeWidget->GetWidgetFromName("FadeColour");

	if (WidgetImage == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("DoScreenFade : WidgetImage == nullptr"));
		return false;
	}

	auto Image = Cast<UImage>(WidgetImage);
	if (Image == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("DoScreenFade : Image == nullptr"));
		return false;
	}

	float FadeRate = FadeOut ? 2.0f : -2.0f;
	// Update Alpha value of Image
	float NewAlpha = FadeRate * GetWorld()->GetDeltaSeconds() + Image->ColorAndOpacity.A;
	Image->SetOpacity(FMath::Clamp(NewAlpha, 0.0f, 1.0f));

	// Check if fade in/out has completed
	if ((!FadeOut && NewAlpha <= 0.0f) || (FadeOut && NewAlpha >= 1.0f))
	{
		return true;
	}

	return false;
}

void AFinalYearProjectCharacter::CheckVRGestureMovement()
{
	float Distance;
	FVector Direction;
	if (bIsActiveLeftMController && bIsActiveRightMController)
	{
		// check movement distance of both controllers
		Distance = GetControllerDistance(EMControllerGestureActiveState::Both);
		Direction = GetControllerDirection(EMControllerGestureActiveState::Both);
		AddPlayerMovement(Direction * (Distance * MovementSpeed));
	}
	else if (bIsActiveLeftMController)
	{
		// check movement distance of left controller
		Distance = GetControllerDistance(EMControllerGestureActiveState::Left);
		Direction = GetControllerDirection(EMControllerGestureActiveState::Left);
		AddPlayerMovement(Direction * (Distance * MovementSpeed));
	}
	else if (bIsActiveRightMController)
	{
		// check movement distance of right controller
		Distance = GetControllerDistance(EMControllerGestureActiveState::Right);
		Direction = GetControllerDirection(EMControllerGestureActiveState::Right);
		AddPlayerMovement(Direction * (Distance * MovementSpeed));
	}

	// update controller relative positions
	PreviousLeftMControllerPos = L_MotionController->GetRelativeTransform().GetLocation();
	PreviousRightMControllerPos = R_MotionController->GetRelativeTransform().GetLocation();
}

float AFinalYearProjectCharacter::GetControllerDistance(EMControllerGestureActiveState ActiveState)
{
	// get distance motion controller(s) have moved since last check
	switch(ActiveState)
	{
		case EMControllerGestureActiveState::Both:
		{
			FVector DeltaPositionL;
			FVector DeltaPositionR;

			DeltaPositionL = L_MotionController->GetRelativeTransform().GetLocation() - PreviousLeftMControllerPos;
			DeltaPositionR = R_MotionController->GetRelativeTransform().GetLocation() - PreviousRightMControllerPos;
			return DeltaPositionL.Size() + DeltaPositionR.Size();
		}
		case EMControllerGestureActiveState::Left:
		{
			FVector DeltaPosition;
			DeltaPosition = L_MotionController->GetRelativeTransform().GetLocation() - PreviousLeftMControllerPos;
			return DeltaPosition.Size();
		}
		case EMControllerGestureActiveState::Right:
		{
			FVector DeltaPosition;
			DeltaPosition = R_MotionController->GetRelativeTransform().GetLocation() - PreviousRightMControllerPos;
			return DeltaPosition.Size();
		}
		default:
			break;
	}
	return 0.0f;
}

FVector AFinalYearProjectCharacter::GetControllerDirection(EMControllerGestureActiveState ActiveState)
{
	// get forward direction of active motion controller or the average direction between two active motion controllers
	switch (ActiveState)
	{
	case EMControllerGestureActiveState::Both:
	{
		FVector Direction;
		FVector ForVecL;
		FVector ForVecR;
		ForVecL = L_MotionController->GetForwardVector();
		ForVecR = R_MotionController->GetForwardVector();
		Direction = (FVector(ForVecL.X, ForVecL.Y, 0.0f).GetSafeNormal() + FVector(ForVecR.X, ForVecR.Y, 0.0f).GetSafeNormal()) / 2.0f;
		return Direction;
	}
	case EMControllerGestureActiveState::Left:
	{
		FVector Direction;
		FVector ForVec;
		ForVec = L_MotionController->GetForwardVector();
		Direction = FVector(ForVec.X, ForVec.Y, 0.0f).GetSafeNormal();
		return Direction;
	}
	case EMControllerGestureActiveState::Right:
	{
		FVector Direction;
		FVector ForVec;
		ForVec = R_MotionController->GetForwardVector();
		Direction = FVector(ForVec.X, ForVec.Y, 0.0f).GetSafeNormal();
		return Direction;
	}
	default:
		break;
	}
	return FVector();
}

void AFinalYearProjectCharacter::AddPlayerMovement(FVector ControllerVector)
{
	// update player location
	FVector ActorLoc = GetActorLocation();
	SetActorLocation(ActorLoc + ControllerVector);
}


void AFinalYearProjectCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}


void AFinalYearProjectCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AFinalYearProjectCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AFinalYearProjectCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AFinalYearProjectCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}
