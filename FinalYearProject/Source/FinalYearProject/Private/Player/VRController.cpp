#include "VRController.h"
#include "MotionControllerComponent.h"
#include "Components/SphereComponent.h"
#include "BasePickup.h"
#include "VRTeleportCursor.h"
#include "Components/SkeletalMeshComponent.h"

AVRController::AVRController()
{
	PrimaryActorTick.bCanEverTick = true;
	MotionControllerComp = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionControllerComp"));
	RootComponent = MotionControllerComp;

	MotionControllerMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MotionControllerMeshComp"));
	MotionControllerMeshComp->SetupAttachment(RootComponent);

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetupAttachment(RootComponent);

	SphereRadius = 18.0f;
	SphereComp->SetSphereRadius(SphereRadius);

	// TODO: haptic feedback on collisions
	/*SphereComp->OnComponentBeginOverlap.AddDynamic();
	SphereComp->OnComponentEndOverlap.AddDynamic();*/

	// default anim state flags
	bTeleporting = false;
	bGrabbing = false;

	MaxTeleportDistance = 1500.0f;

	bValidTeleportPosition = false;

	PreviousTeleportPosition = CurrentTeleportPosition = FVector::ZeroVector;
}

void AVRController::BeginPlay()
{
	Super::BeginPlay();
	
	// create instance of AVRTeleportCursor ///////
	FActorSpawnParameters SpawnParams;
	
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	//// spawn teleport cursor
	TeleportCursor = GetWorld()->SpawnActor<AVRTeleportCursor>(TeleportCursorClass, GetActorLocation(), GetActorRotation(), SpawnParams);

	
}

void AVRController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bValidTeleportPosition)
	{
		// update teleport cursor
		UpdateTeleportCursor();
	}
	

}

ABasePickup* AVRController::GetObjectNearestToHand()
{
	TArray<AActor*> ListOfObjects;
	TSubclassOf<ABasePickup> FilterClass;
	SphereComp->GetOverlappingActors(ListOfObjects, FilterClass);

	// check if nothing is found
	if (ListOfObjects.Num() == 0)
	{
		return nullptr;
	}

	ABasePickup* NearestObject = nullptr;
	
	float MinDeltaDist = 1000.0f; // arbitrary, large value

	
	// find closest object to controller
	for(int i = 0; i < ListOfObjects.Num(); i++)
	{
		ABasePickup* TempPickup = Cast<ABasePickup>(ListOfObjects[i]);
		if (TempPickup != nullptr)
		{
			if (TempPickup->IsGrabbed())
			{
				continue;
			}

			float DeltaDist = (GetActorLocation() - TempPickup->GetActorLocation()).Size();
			if (DeltaDist < MinDeltaDist)
			{
				MinDeltaDist = DeltaDist;
				NearestObject = TempPickup;
			}
		}
		
	}

	return NearestObject;
}



void AVRController::SetHand(EControllerHand Hand)
{
	MotionControllerComp->Hand = Hand;
	if (MotionControllerComp->Hand == EControllerHand::Right)
	{
		MotionControllerMeshComp->SetRelativeScale3D(FVector(1.0f, 1.0f, -1.0));
	}
}

FVector AVRController::GetControllerRelativeLocation()
{
	return MotionControllerComp->GetRelativeTransform().GetLocation();
}

FVector AVRController::GetControllerForwardVector()
{
	return MotionControllerComp->GetForwardVector();
}

void AVRController::GrabObject()
{
	CurrentPickupObject = GetObjectNearestToHand();
	if (CurrentPickupObject != nullptr)
	{
		CurrentPickupObject->Grab(MotionControllerMeshComp, "grab_pos");	// TODO: promote string here to a variable.
		//CurrentPickupObject->SetGrabbed(true);
	}
}

void AVRController::DropObject()
{
	if (CurrentPickupObject != nullptr)
	{
		CurrentPickupObject->Drop();
		//CurrentPickupObject->SetGrabbed(false);
		CurrentPickupObject = nullptr;
	}
}

void AVRController::SetTeleporting(bool Teleporting)
{
	bTeleporting = Teleporting;
}

void AVRController::SetGrabbing(bool Grabbing)
{
	bGrabbing = Grabbing;
}

bool AVRController::GetTeleporting()
{
	return bTeleporting;
}

bool AVRController::GetGrabbing()
{
	return bGrabbing;
}

void AVRController::CheckValidTeleportLocation()
{
	FHitResult Hit;
	FCollisionQueryParams RV_TraceParams = FCollisionQueryParams(FName(TEXT("RV_Trace")), true, this);

	FVector StartPos;
	FVector EndPos;

	// if using hmd, then create the line trace using the motion controller.
	// otherwise, use the player camera

	//StartPos = GetActorLocation();
	//set start position to socket location on skeletal mesh
	StartPos = MotionControllerMeshComp->GetSocketLocation("teleport_start_pos");
	FRotator ControllerRot = GetActorRotation();
	//FVector ControllerDir = VRController_R->GetControllerForwardVector();
	EndPos = StartPos + (ControllerRot.Vector() * MaxTeleportDistance);



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

	bValidTeleportPosition = DidTrace;
}

bool AVRController::StopTeleport()
{
	if (bValidTeleportPosition)
	{
		// if valid teleport location, move to that location
		// TODO: when fading in / out, update this state change
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (PC)
		{
			DisableInput(PC);
		}
		return true;
	}
		
	return false;
}

void AVRController::CancelTeleport()
{
	if (TeleportCursor->IsVisible())
	{
		TeleportCursor->SetVisible(false);
	}
}

bool AVRController::OnTeleport(FVector& OutTeleportLocation)
{
	OutTeleportLocation = CurrentTeleportPosition;

	if (OutTeleportLocation == FVector(0))
	{

		if (TeleportCursor->IsVisible())
		{
			TeleportCursor->SetVisible(false);
		}
		return false;
	}

	if (TeleportCursor->IsVisible())
	{
		TeleportCursor->SetVisible(false);
	}

	return true;
}

void AVRController::UpdateTeleportCursor()
{
	// update where the cursor is 
	if (TeleportCursor && TeleportCursor->IsVisible())
	{
		TeleportCursor->UpdateCursor(CurrentTeleportPosition);
	}
}

