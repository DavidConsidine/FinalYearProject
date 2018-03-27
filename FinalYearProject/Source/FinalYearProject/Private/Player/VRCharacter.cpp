#include "VRCharacter.h"
#include "Camera/CameraComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Engine/Engine.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "VRController.h"
#include "VRGameMode.h"
#include "Components/WidgetInteractionComponent.h"
#include "Components/BoxComponent.h"
#include "BasePickup.h"


AVRCharacter::AVRCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	VROrigin = CreateDefaultSubobject<USceneComponent>(TEXT("VROrigin"));
	VROrigin->SetupAttachment(RootComponent);
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(VROrigin);

	// set up trigger volume
	PickupTriggerVolumeComp = CreateDefaultSubobject<UBoxComponent>(TEXT("PickupTriggerVolumeComp"));
	PickupTriggerVolumeComp->SetupAttachment(VROrigin);
	//PickupTriggerVolumeComp->SetRelativeTransform(VROrigin->GetComponentTransform());
	PickupTriggerVolumeComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickupTriggerVolumeComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupTriggerVolumeComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	PickupTriggerVolumeComp->OnComponentBeginOverlap.AddDynamic(this, &AVRCharacter::OnOverlapBegin);
	

	//GetCapsuleComponent()->SetupAttachment(RootComponent);

	// setting default values
	BaseTurnRate = 45.0f;
	BaseLookUpRate = 45.0f;

	DefaultPlayerHeight = 180.0f;

	FadeCoefficient = 2.0f;

	SetTelState(Wait);

	bIsLeftGripPressed = false;
	bIsRightGripPressed = false;

	GripMovementSpeed = 1.0f;

	// fade screen
	ConstructorHelpers::FClassFinder<UUserWidget> WidgetHelper(TEXT("/Game/UI/WBP_FadeScreen"));
	if (WidgetHelper.Succeeded())
	{
		WidgetClass = WidgetHelper.Class;
	}

	WidgetInteractionComp = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("WidgetInteractionComp"));
	WidgetInteractionComp->Activate(false);
	WidgetInteractionComp->InteractionDistance = 100.0f;
	WidgetInteractionComp->PointerIndex = 1.0f;
	WidgetInteractionComp->TraceChannel = ECollisionChannel::ECC_Visibility;
	WidgetInteractionComp->VirtualUserIndex = 1;
}

void AVRCharacter::SetCanMove(bool CanMove)
{
	bCanMove = CanMove;

	// prevent teleporting
	CancelTeleport();
}

void AVRCharacter::DisableMenuComponents()
{
	if (ModeSelectMenu != nullptr)
	{
		ModeSelectMenu->SetActorHiddenInGame(true);
	}

	if (WidgetInteractionComp != nullptr && WidgetInteractionComp->bIsActive)
	{
		WidgetInteractionComp->Activate(false);
	}
}

void AVRCharacter::EnableMenuComponents()
{
	if (ModeSelectMenu != nullptr)
	{
		ModeSelectMenu->SetActorHiddenInGame(false);
	}

	if (WidgetInteractionComp != nullptr && WidgetInteractionComp->bIsActive)
	{
		WidgetInteractionComp->Activate(true);
	}
}



void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled())
	{
		// if anything needs to be done HMD related
		//RootComponent = VROrigin;
		SetRootComponent(VROrigin);
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);

		FAttachmentTransformRules TriggerVolumeTransformRules(EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, EAttachmentRule::KeepWorld, true);

		PickupTriggerVolumeComp->AttachToComponent(CameraComp, TriggerVolumeTransformRules);
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
				PreviousLeftMControllerPos = CurrentLeftMControllerPos = VRController_L->GetControllerRelativeLocation();
			}
			// Right
			VRController_R = GetWorld()->SpawnActor<AVRController>(MotionControllerClass, ControllerTransform, SpawnParams);
			if (VRController_R)
			{
				VRController_R->SetHand(EControllerHand::Right);
				VRController_R->AttachToComponent(RootComponent, TransformRules);
				PreviousRightMControllerPos = CurrentRightMControllerPos = VRController_R->GetControllerRelativeLocation();
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

	
	FTransform MenuTransform = GetTransform();

	// check what game mode player is currently in (expected to be menuselect mode)
	AVRGameMode* GameMode = Cast<AVRGameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode != nullptr) 
	{
		UE_LOG(LogTemp, Warning, TEXT("AVRCharacter::BeginPlay - VRGameMode valid"));
		switch (GameMode->GetCurrentGameMode())
		{
		case MenuSelect:
			// set up mode select widget
			ModeSelectMenu = GetWorld()->SpawnActor<AActor>(ModeSelectClass, MenuTransform, SpawnParams);
			if (ModeSelectMenu != nullptr)
			{
				UE_LOG(LogTemp, Warning, TEXT("AVRCharacter::BeginPlay - ModeSelectMenu valid"));
				FAttachmentTransformRules ModeSelectTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false);
				// add menu to left hand controller
				if (VRController_L)
				{

					ModeSelectMenu->AttachToComponent(VRController_L->GetSkeletalMeshComponent(), ModeSelectTransformRules, "menu_pos");

					// rotate widget so it's visible to player camera
					FRotator NewRot = ModeSelectMenu->GetActorRotation();
					NewRot.Add(90.0f, 180.0f, 90.0f);
					ModeSelectMenu->SetActorRotation(NewRot);
				}
				else
				{
					// error with set up. comment out following else statement
					// rotate widget so it's visible to player camera
				}
				// set up widget interaction component on right controller
				FAttachmentTransformRules WidgetInteractionCompTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false);

				if (VRController_R)
				{
					WidgetInteractionComp->AttachToComponent(VRController_R->GetSkeletalMeshComponent(), WidgetInteractionCompTransformRules, "menu_pointer_pos");
					WidgetInteractionComp->Activate(true);
					
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("AVRCharacter::BeginPlay - VRController_R not valid. WidgetInteractionComponent setup failed."));
				}

			}

			UE_LOG(LogTemp, Warning, TEXT("AVRCharacter::BeginPlay - GameMode = menu select"));
			break;
		case FreeRoam:
			UE_LOG(LogTemp, Warning, TEXT("AVRCharacter::BeginPlay - GameMode = free roam"));
			break;
		case TimedLow:
			UE_LOG(LogTemp, Warning, TEXT("AVRCharacter::BeginPlay - GameMode = timed low"));
			break;
		case TimedMid:
			UE_LOG(LogTemp, Warning, TEXT("AVRCharacter::BeginPlay - GameMode = timed mid"));
			break;
		case TimedHigh:
			UE_LOG(LogTemp, Warning, TEXT("AVRCharacter::BeginPlay - GameMode = timed high"));
			break;
		case TimedAll:
			UE_LOG(LogTemp, Warning, TEXT("AVRCharacter::BeginPlay - GameMode = timed all"));
			break;
		default:
			break;
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AVRCharacter::BeginPlay - VRGameMode not valid"));
	}
	

	// ui set up
	if (WidgetClass)
	{
		ScreenFadeWidget = CreateWidget<UUserWidget>(GetWorld(), WidgetClass);
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
		CheckValidTeleportLocation();
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
			VRController_R->SetTeleporting(false);
			APlayerController* PC = GetWorld()->GetFirstPlayerController();
			if (PC)
			{
				EnableInput(PC);
			}
		}
		break;
	}

	//update controllers relative positions
	if (VRController_L != nullptr)
	{
		PreviousLeftMControllerPos = CurrentLeftMControllerPos;
		CurrentLeftMControllerPos = VRController_L->GetControllerRelativeLocation();
	}
	if (VRController_R != nullptr)
	{
		PreviousRightMControllerPos = CurrentRightMControllerPos;
		CurrentRightMControllerPos = VRController_R->GetControllerRelativeLocation();
	}

	// check for gesture movement
	if (bCanMove && (bIsLeftGripPressed || bIsRightGripPressed))
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

	// bind right controller trigger for widget interactions
	PlayerInputComponent->BindAction("Click", IE_Pressed, this, &AVRCharacter::Click);
	PlayerInputComponent->BindAction("Click", IE_Released, this, &AVRCharacter::Release);

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

void AVRCharacter::OnOverlapBegin(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("OnOverlapBegin"));
	ABasePickup* ShoppingItem = Cast<ABasePickup>(OtherActor);
	AVRController* MotionController = Cast<AVRController>(OtherActor);

	// if overlapping actor is a pickup object
	if (ShoppingItem != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Item added to basket"));
		ShoppingItem->AddedToBasket();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("no overlap with basepickup"));
	}

	// if overlapping actor is a motioncontroller
		// check whether it was the left or right controller
	if (MotionController != nullptr)
	{
		if (MotionController->GetHand() == EControllerHand::Left)
		{
			UE_LOG(LogTemp, Warning, TEXT("Left controller overlap"));
		}
		else if (MotionController->GetHand() == EControllerHand::Right)
		{
			UE_LOG(LogTemp, Warning, TEXT("Right controller overlap"));
		}
	}
}

void AVRCharacter::Click()
{
	AVRGameMode* GameMode = Cast<AVRGameMode>(GetWorld()->GetAuthGameMode());
	// check that mode select is current game mode
	if (GameMode != nullptr && GameMode->GetCurrentGameMode() == MenuSelect)
	{
		// assuming interaction widget & R motion controller are valid 
		if (WidgetInteractionComp)
		{
			WidgetInteractionComp->PressPointerKey(EKeys::LeftMouseButton);
		}
	}
}

void AVRCharacter::Release()
{
	AVRGameMode* GameMode = Cast<AVRGameMode>(GetWorld()->GetAuthGameMode());
	// check that mode select is current game mode
	if (GameMode != nullptr && GameMode->GetCurrentGameMode() == MenuSelect)
	{
		// assuming interaction widget & R motion controller are valid 
		if (WidgetInteractionComp)
		{
			WidgetInteractionComp->ReleasePointerKey(EKeys::LeftMouseButton);
		}
	}
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
	if (!VRController_R->GetTeleporting() && TelState == Wait)
	{
		SetTelState(Aiming);
		VRController_R->SetTeleporting(true);
	}
}


void AVRCharacter::StopTeleport()
{
	if (Aiming && VRController_R->GetTeleporting() && VRController_R->IsValidTeleportLocation())
	{
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (PC)
		{
			DisableInput(PC);
		}
		SetTelState(FadeOut);
	}
	else
	{
		CancelTeleport();
	}
}

void AVRCharacter::CancelTeleport()
{
	if (VRController_R != nullptr && VRController_R->GetTeleporting() && TelState == Aiming)
	{
		SetTelState(Wait);

		VRController_R->CancelTeleport();
		VRController_R->SetTeleporting(false);
	}
}


void AVRCharacter::SetTelState(ETeleportState NewState)
{
	TelState = NewState;
}


void AVRCharacter::OnTeleport()
{
	FVector TeleportPosition;

	if (VRController_R != nullptr && !VRController_R->OnTeleport(TeleportPosition))
	{
		SetTelState(Wait);
		VRController_R->SetTeleporting(false);
	}
	else
	{
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
}

void AVRCharacter::CheckValidTeleportLocation()
{
	if (VRController_R != nullptr)
	{
		VRController_R->CheckValidTeleportLocation();
	}
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
	bIsLeftGripPressed = true;
}

void AVRCharacter::LeftGripReleased()
{
	bIsLeftGripPressed = false;
}

void AVRCharacter::RightGripPressed()
{
	bIsRightGripPressed = true;
}

void AVRCharacter::RightGripReleased()
{
	bIsRightGripPressed = false;
}

void AVRCharacter::CheckVRGestureMovement()
{
	float Distance;
	FVector Direction;
	if (bIsLeftGripPressed && bIsRightGripPressed)
	{
		// check movement distance of both controllers
		Distance = GetControllerDistance(EMControllerGripActiveState::Both);
		Direction = GetControllerDirection(EMControllerGripActiveState::Both);
		AddPlayerMovement(Direction * (Distance * GripMovementSpeed));
	}
	else if (bIsLeftGripPressed)
	{
		// check movement distance of left controller
		Distance = GetControllerDistance(EMControllerGripActiveState::Left);
		Direction = GetControllerDirection(EMControllerGripActiveState::Left);
		AddPlayerMovement(Direction * (Distance * GripMovementSpeed));
	}
	else if (bIsRightGripPressed)
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

		DeltaPositionL = CurrentLeftMControllerPos - PreviousLeftMControllerPos;
		DeltaPositionR = CurrentRightMControllerPos - PreviousRightMControllerPos;
		return DeltaPositionL.Size() + DeltaPositionR.Size();
	}
	case EMControllerGripActiveState::Left:
	{
		FVector DeltaPosition;
		DeltaPosition = CurrentLeftMControllerPos - PreviousLeftMControllerPos;
		return DeltaPosition.Size();
	}
	case EMControllerGripActiveState::Right:
	{
		FVector DeltaPosition;
		DeltaPosition = CurrentRightMControllerPos - PreviousRightMControllerPos;
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
	if (!VRController_L->GetGrabbing())
	{
		VRController_L->SetGrabbing(true);
		VRController_L->GrabObject();
	}
}

void AVRCharacter::DropLeft()
{
	// left controller drop
	if (VRController_L->GetGrabbing())
	{
		VRController_L->SetGrabbing(false);
		VRController_L->DropObject();
	}
}

void AVRCharacter::GrabRight()
{
	// right controller grab
	if (!VRController_R->GetGrabbing())
	{
		VRController_R->SetGrabbing(true);
		VRController_R->GrabObject();
	}
}

void AVRCharacter::DropRight()
{
	// right controller drop
	VRController_R->DropObject();
	if (VRController_R->GetGrabbing())
	{
		VRController_R->SetGrabbing(false);
		VRController_R->DropObject();
	}
}

void AVRCharacter::MoveForward(float Val)
{
	if (bCanMove)
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
}


void AVRCharacter::MoveRight(float Val)
{
	if (bCanMove)
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
}


void AVRCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}


void AVRCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}