#include "VRCharacter.h"
#include "Camera/CameraComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Engine/Engine.h"
#include "VRTeleportCursor.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "VRController.h"


AVRCharacter::AVRCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	VROrigin = CreateDefaultSubobject<USceneComponent>(TEXT("VROrigin"));
	VROrigin->SetupAttachment(RootComponent);
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(VROrigin);

	//GetCapsuleComponent()->SetupAttachment(RootComponent);

	// setting default values
	BaseTurnRate = 45.0f;
	BaseLookUpRate = 45.0f;

	DefaultPlayerHeight = 180.0f;

	bTeleporting = false;
	bValidTeleportPosition = false;

	MaxTeleportDistance = 1500.0f;
	FadeCoefficient = 2.0f;

	PreviousTeleportPosition = CurrentTeleportPosition = FVector::ZeroVector;

	SetTelState(Wait);

	bIsLeftActive = false;
	bIsRightActive = false;

	GripMovementSpeed = 1.0f;

	// fade screen
	ConstructorHelpers::FClassFinder<UUserWidget> WidgetHelper(TEXT("/Game/UI/WBP_FadeScreen"));
	if (WidgetHelper.Succeeded())
	{
		WidgetClass = WidgetHelper.Class;
	}
}

void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();

	FActorSpawnParameters SpawnParams;
	//SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// spawn teleport cursor
	TeleportCursor = GetWorld()->SpawnActor<AVRTeleportCursor>(TeleportCursorClass, GetActorLocation(), GetActorRotation(), SpawnParams);

	if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled())
	{
		// if anything needs to be done HMD related
		//RootComponent = VROrigin;
		SetRootComponent(VROrigin);
		//GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		//CameraComp->AttachToComponent(RootComponent, );

		// TODO: remove when testing standing up
		/////////////////////////////////////////////////////////////////////////////////
		UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Floor);
		// set offset
		//FVector RootOffset(0.0f, 0.0f, DefaultPlayerHeight);
		//VROrigin->AddLocalOffset(RootOffset);
		/////////////////////////////////////////////////////////////////////////////////

		// spawn motion controllers and attach to VROrigin component
		if (MotionControllerClass)
		{
			FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, false);
			FTransform ControllerTransform;

			// Left
			VRController_L = GetWorld()->SpawnActor<AVRController>(MotionControllerClass, ControllerTransform, SpawnParams);
			if (VRController_L)
			{
				VRController_L->SetHand(EControllerHand::Left);
				VRController_L->AttachToComponent(RootComponent, TransformRules);

				UE_LOG(LogTemp, Warning, TEXT("left controller spawned successful"));
			}


			// Right
			VRController_R = GetWorld()->SpawnActor<AVRController>(MotionControllerClass, ControllerTransform, SpawnParams);
			if (VRController_R)
			{
				VRController_R->SetHand(EControllerHand::Right);
				VRController_R->AttachToComponent(RootComponent, TransformRules);
				UE_LOG(LogTemp, Warning, TEXT("right controller spawned successful"));
			}
		}
		
		
		UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
	}
	else
	{
		// no hmd active
		VROrigin->RelativeLocation = FVector(-39.56f, 1.75f, 64.f); // Position the camera
		CameraComp->bUsePawnControlRotation = true;
	}


	// ui set up
	if (WidgetClass)
	{
		ScreenFadeWidget = CreateWidget<UUserWidget>(GetWorld(), WidgetClass);
		//FVector2D ScreenSize;
		//GetWorld()->GetGameViewport()->GetViewportSize(ScreenSize);
		if (ScreenFadeWidget)
		{
			ScreenFadeWidget->AddToViewport();
		}	
	}


}


void AVRCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	

	switch (TelState)
	{
	case Wait:
		GEngine->AddOnScreenDebugMessage(0, 0.5f, FColor::Yellow, "TelState: Wait", true);
		break;
	case Aiming:
		GEngine->AddOnScreenDebugMessage(0, 0.5f, FColor::Yellow, "TelState: Aiming", true);
		//update linetrace and determine if valid location
		bValidTeleportPosition = CheckValidTeleportLocation();
		if (bValidTeleportPosition)
		{
			// update cursor
			UpdateTeleportCursor();
		}
		break;
	case FadeOut:
		GEngine->AddOnScreenDebugMessage(0, 0.5f, FColor::Yellow, "TelState: FadeOut", true);
		if (DoScreenFade(true))
		{
			SetTelState(Teleport);
		}
		break;
	case Teleport:
		GEngine->AddOnScreenDebugMessage(0, 0.5f, FColor::Yellow, "TelState: Teleport", true);
		OnTeleport();
		break;
	case FadeIn:
		GEngine->AddOnScreenDebugMessage(0, 0.5f, FColor::Yellow, "TelState: FadeIn", true);
		if (DoScreenFade(false))
		{
			SetTelState(Wait);
			bTeleporting = false;
			APlayerController* PC = GetWorld()->GetFirstPlayerController();
			if (PC)
			{
				EnableInput(PC);
			}
		}
		break;
	}

	// check for gesture movement
	if (bIsLeftActive || bIsRightActive)
	{
		CheckVRGestureMovement();
	}
}


void AVRCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);


	// Bind teleport events
	PlayerInputComponent->BindAction("Teleport", IE_Pressed, this, &AVRCharacter::StartTeleport);
	PlayerInputComponent->BindAction("Teleport", IE_Released, this, &AVRCharacter::StopTeleport);
	PlayerInputComponent->BindAction("CancelTeleport", IE_Pressed, this, &AVRCharacter::CancelTeleport);

	
	//// Bind VR Gesture Movement (Left Motion Controller) event
	PlayerInputComponent->BindAction("VR_GestureMovement_L", IE_Pressed, this, &AVRCharacter::LeftGripPressed);
	PlayerInputComponent->BindAction("VR_GestureMovement_L", IE_Released, this, &AVRCharacter::LeftGripReleased);
	//// Bind VR Gesture Movement (Right Motion Controller) event
	PlayerInputComponent->BindAction("VR_GestureMovement_R", IE_Pressed, this, &AVRCharacter::RightGripPressed);
	PlayerInputComponent->BindAction("VR_GestureMovement_R", IE_Released, this, &AVRCharacter::RightGripReleased);

	// Bind controller grab/drop actions
	PlayerInputComponent->BindAction("Grab_L", IE_Pressed, this, &AVRCharacter::GrabLeft);
	PlayerInputComponent->BindAction("Grab_L", IE_Released, this, &AVRCharacter::DropLeft);
	//// Bind VR Gesture Movement (Right Motion Controller) event
	PlayerInputComponent->BindAction("Grab_R", IE_Pressed, this, &AVRCharacter::GrabRight);
	PlayerInputComponent->BindAction("Grab_R", IE_Released, this, &AVRCharacter::DropRight);

	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AVRCharacter::OnResetVR);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &AVRCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AVRCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AVRCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AVRCharacter::LookUpAtRate);
}


void AVRCharacter::OnResetVR()
{
	if(UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled())
	{
		UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
	}	
	
}


void AVRCharacter::StartTeleport()
{
	if (!bTeleporting && TelState == Wait)
	{
		bTeleporting = true;
		SetTelState(Aiming);
	}
}


void AVRCharacter::StopTeleport()
{
	if (bTeleporting && Aiming && bValidTeleportPosition)
	{
		// if valid teleport location, move to that location
		// TODO: when fading in / out, update this state change
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (PC)
		{
			DisableInput(PC);
		}
		SetTelState(FadeOut);
	}
	else if(bTeleporting && Aiming && !bValidTeleportPosition)
	{
		CancelTeleport();

	}
}

void AVRCharacter::CancelTeleport()
{
	if (bTeleporting && TelState == Aiming)
	{
		SetTelState(Wait);
		bTeleporting = false;

		if (TeleportCursor->IsVisible())
		{
			TeleportCursor->SetVisible(false);
		}
	}
}


void AVRCharacter::SetTelState(ETeleportState NewState)
{
	UE_LOG(LogTemp, Warning, TEXT("SetTelState: Setting State from %d to %d"), (int)TelState, (int)NewState)
	TelState = NewState;
}


void AVRCharacter::OnTeleport()
{
	FVector TeleportPosition = CurrentTeleportPosition;

	if (TeleportPosition == FVector(0))
	{
		SetTelState(Wait);

		if (TeleportCursor->IsVisible())
		{
			TeleportCursor->SetVisible(false);
		}
		return;

	}

	if (TeleportCursor->IsVisible())
	{
		TeleportCursor->SetVisible(false);
	}

	if (!UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled())
	{
		// offset location z value to avoid getting stuck in ground
		TeleportPosition.Z += 50.0f;
		SetActorLocation(TeleportPosition);
	}
	else
	{
		SetActorLocation(TeleportPosition);
		//UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
	}
	
	

	SetTelState(FadeIn);
}


void AVRCharacter::UpdateTeleportCursor()
{
	// update where the cursor is 
	if (TeleportCursor && TeleportCursor->IsVisible())
	{
		TeleportCursor->UpdateCursor(CurrentTeleportPosition);
	}
}

bool AVRCharacter::CheckValidTeleportLocation()
{
	FHitResult Hit;
	FCollisionQueryParams RV_TraceParams = FCollisionQueryParams(FName(TEXT("RV_Trace")), true, this);

	FVector StartPos;
	FVector EndPos;

	// if using hmd, then create the line trace using the motion controller.
	// otherwise, use the player camera

	if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled())
	{
		StartPos = VRController_R->GetActorLocation();
		FRotator ControllerRot = VRController_R->GetActorRotation();
		//FVector ControllerDir = VRController_R->GetControllerForwardVector();
		EndPos = StartPos + (ControllerRot.Vector() * MaxTeleportDistance);
	}
	else
	{
		StartPos = CameraComp->GetComponentLocation();
		FRotator CameraRot = CameraComp->GetComponentRotation();
		//FVector CamDir = CameraComp->GetForwardVector();
		// TODO: Expose variable to the Editor
		EndPos = StartPos + (CameraRot.Vector() * MaxTeleportDistance);
	}
	

	RV_TraceParams.bTraceComplex = true;
	RV_TraceParams.bTraceAsyncScene = true;
	RV_TraceParams.bReturnPhysicalMaterial = true;

	//  do the line trace
	bool DidTrace = GetWorld()->LineTraceSingleByChannel(
		Hit,        //result
		StartPos,        //start
		EndPos,        //end
		ECC_Pawn,    //collision channel
		RV_TraceParams
	);

	if (DidTrace)
	{
		PreviousTeleportPosition = CurrentTeleportPosition;
		CurrentTeleportPosition = Hit.ImpactPoint;
		if (!TeleportCursor->IsVisible())
		{
			TeleportCursor->SetVisible(true);
		}
	}
	else
	{
		if (TeleportCursor->IsVisible())
		{
			TeleportCursor->SetVisible(false);
		}
		PreviousTeleportPosition = CurrentTeleportPosition = FVector::ZeroVector;
	}

	return DidTrace;
}


bool AVRCharacter::DoScreenFade(bool FadeOut)
{
	//Get reference to Blueprint's Image component
	auto WidgetImage = ScreenFadeWidget->GetWidgetFromName("FadeImage");

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

	float FadeRate = FadeOut ? FadeCoefficient : -1.0f * FadeCoefficient;
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

void AVRCharacter::LeftGripPressed()
{
	bIsLeftActive = true;
}

void AVRCharacter::LeftGripReleased()
{
	bIsLeftActive = false;
}

void AVRCharacter::RightGripPressed()
{
	bIsRightActive = true;
}

void AVRCharacter::RightGripReleased()
{
	bIsRightActive = false;
}

void AVRCharacter::CheckVRGestureMovement()
{
	float Distance;
	FVector Direction;
	if (bIsLeftActive && bIsRightActive)
	{
		// check movement distance of both controllers
		Distance = GetControllerDistance(EMControllerGripActiveState::Both);
		Direction = GetControllerDirection(EMControllerGripActiveState::Both);
		AddPlayerMovement(Direction * (Distance * GripMovementSpeed));
	}
	else if (bIsLeftActive)
	{
		// check movement distance of left controller
		Distance = GetControllerDistance(EMControllerGripActiveState::Left);
		Direction = GetControllerDirection(EMControllerGripActiveState::Left);
		AddPlayerMovement(Direction * (Distance * GripMovementSpeed));
	}
	else if (bIsRightActive)
	{
		// check movement distance of right controller
		Distance = GetControllerDistance(EMControllerGripActiveState::Right);
		Direction = GetControllerDirection(EMControllerGripActiveState::Right);
		AddPlayerMovement(Direction * (Distance * GripMovementSpeed));
	}

	// update controller relative positions
	PreviousLeftMControllerPos = VRController_L->GetControllerRelativeLocation();
	PreviousRightMControllerPos = VRController_R->GetControllerRelativeLocation();
}

float AVRCharacter::GetControllerDistance(EMControllerGripActiveState ActiveState)
{
	// get distance motion controller(s) have moved since last check
	switch (ActiveState)
	{
	case EMControllerGripActiveState::Both:
	{
		FVector DeltaPositionL;
		FVector DeltaPositionR;

		DeltaPositionL = VRController_L->GetControllerRelativeLocation() - PreviousLeftMControllerPos;
		DeltaPositionR = VRController_R->GetControllerRelativeLocation() - PreviousRightMControllerPos;
		return DeltaPositionL.Size() + DeltaPositionR.Size();
	}
	case EMControllerGripActiveState::Left:
	{
		FVector DeltaPosition;
		DeltaPosition = VRController_L->GetControllerRelativeLocation() - PreviousLeftMControllerPos;
		return DeltaPosition.Size();
	}
	case EMControllerGripActiveState::Right:
	{
		FVector DeltaPosition;
		DeltaPosition = VRController_R->GetControllerRelativeLocation() - PreviousRightMControllerPos;
		return DeltaPosition.Size();
	}
	default:
		break;
	}
	return 0.0f;
}

FVector AVRCharacter::GetControllerDirection(EMControllerGripActiveState ActiveState)
{
	// get forward direction of active motion controller or the average direction between two active motion controllers
	switch (ActiveState)
	{
	case EMControllerGripActiveState::Both:
	{
		FVector Direction;
		FVector ForVecL;
		FVector ForVecR;
		ForVecL = VRController_L->GetControllerForwardVector();
		ForVecR = VRController_R->GetControllerForwardVector();
		Direction = (FVector(ForVecL.X, ForVecL.Y, 0.0f).GetSafeNormal() + FVector(ForVecR.X, ForVecR.Y, 0.0f).GetSafeNormal()) / 2.0f;
		return Direction;
	}
	case EMControllerGripActiveState::Left:
	{
		FVector Direction;
		FVector ForVec;
		ForVec = VRController_L->GetControllerForwardVector();
		Direction = FVector(ForVec.X, ForVec.Y, 0.0f).GetSafeNormal();
		return Direction;
	}
	case EMControllerGripActiveState::Right:
	{
		FVector Direction;
		FVector ForVec;
		ForVec = VRController_R->GetControllerForwardVector();
		Direction = FVector(ForVec.X, ForVec.Y, 0.0f).GetSafeNormal();
		return Direction;
	}
	default:
		break;
	}
	return FVector();
}

void AVRCharacter::AddPlayerMovement(FVector ControllerVector)
{
	// update player location
	FVector ActorLoc = GetActorLocation();
	SetActorLocation(ActorLoc + ControllerVector);
}

void AVRCharacter::GrabLeft()
{
	// left controller grab
	VRController_L->GrabObject();
}

void AVRCharacter::DropLeft()
{
	// left controller drop
	VRController_L->DropObject();
}

void AVRCharacter::GrabRight()
{
	// right controller grab
	VRController_R->GrabObject();
}

void AVRCharacter::DropRight()
{
	// right controller drop
	VRController_R->DropObject();
}


void AVRCharacter::MoveForward(float Val)
{
	if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled() && Val != 0.0f)
	{
		// set up so forward is always where the camera is facing
		AddMovementInput(CameraComp->GetForwardVector(), Val);
	}
	else if (!UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled() && Val != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Val);
	}
}


void AVRCharacter::MoveRight(float Val)
{
	if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled() && Val != 0.0f)
	{
		// set up so forward is always where the camera is facing
		AddMovementInput(CameraComp->GetRightVector(), Val * 0.5); // half speed going sideways as full speed is not natural, tweak later
	}
	else if (!UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled() && Val != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Val);
	}
}


void AVRCharacter::TurnAtRate(float Rate)
{
	GEngine->AddOnScreenDebugMessage(0, 0.5f, FColor::Yellow, "TurnAtRate", true);
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}


void AVRCharacter::LookUpAtRate(float Rate)
{
	GEngine->AddOnScreenDebugMessage(0, 0.5f, FColor::Yellow, "LookUpAtRate", true);
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}