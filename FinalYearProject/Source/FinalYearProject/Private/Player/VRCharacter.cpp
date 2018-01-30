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


AVRCharacter::AVRCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	VROrigin = CreateDefaultSubobject<USceneComponent>(TEXT("VROrigin"));
	VROrigin->SetupAttachment(RootComponent);
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(VROrigin);

	//GetCapsuleComponent()->SetupAttachment(RootComponent);

	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh1P"));
	Mesh1P->SetupAttachment(RootComponent);

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
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// spawn teleport cursor
	TeleportCursor = GetWorld()->SpawnActor<AVRTeleportCursor>(TeleportCursorClass, GetActorLocation(), GetActorRotation(), SpawnParams);

	if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled())
	{
		// if anything needs to be done HMD related
		RootComponent = VROrigin;
		CameraComp->SetupAttachment(RootComponent);
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
	//PlayerInputComponent->BindAction("VR_GestureMovement_L", IE_Pressed, this, &AVRCharacter::SetIsActiveLeftMController);
	//PlayerInputComponent->BindAction("VR_GestureMovement_L", IE_Released, this, &AVRCharacter::SetIsNotActiveLeftMController);
	//// Bind VR Gesture Movement (Right Motion Controller) event
	//PlayerInputComponent->BindAction("VR_GestureMovement_R", IE_Pressed, this, &AVRCharacter::SetIsActiveRightMController);
	//PlayerInputComponent->BindAction("VR_GestureMovement_R", IE_Released, this, &AVRCharacter::SetIsNotActiveRightMController);

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
	else if(bTeleporting && Aiming && bValidTeleportPosition)
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

	// offset location z value to avoid getting stuck in ground
	TeleportPosition.Z += 50.0f;
	SetActorLocation(TeleportPosition);

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
	FVector CamPos = CameraComp->GetComponentLocation();
	FRotator CameraRot = CameraComp->GetComponentRotation();
	FVector CamDir = CameraComp->GetForwardVector();
	// TODO: Expose variable to the Editor
	FVector End = CamPos + (CameraRot.Vector() * MaxTeleportDistance);

	RV_TraceParams.bTraceComplex = true;
	RV_TraceParams.bTraceAsyncScene = true;
	RV_TraceParams.bReturnPhysicalMaterial = true;

	//  do the line trace
	bool DidTrace = GetWorld()->LineTraceSingleByChannel(
		Hit,        //result
		CamPos,        //start
		End,        //end
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


void AVRCharacter::MoveForward(float Val)
{
	GEngine->AddOnScreenDebugMessage(0, 0.5f, FColor::Yellow, "MoveForward", true);
	if (Val != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Val);
	}
}


void AVRCharacter::MoveRight(float Val)
{
	GEngine->AddOnScreenDebugMessage(0, 0.5f, FColor::Yellow, "MoveRight", true);
	if (Val != 0.0f)
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