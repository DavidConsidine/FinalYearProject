#include "VRController.h"
#include "MotionControllerComponent.h"
#include "Components/SphereComponent.h"
#include "BasePickup.h"
#include "VRTeleportCursor.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"

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

	TeleportSplineComp = CreateDefaultSubobject<USplineComponent>(TEXT("TeleportSplineComp"));

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

	if (MotionControllerComp->Hand == EControllerHand::Right && bValidTeleportPosition)
	{
		// update teleport cursor
		UpdateTeleportCursor();
	}
	if (MotionControllerComp->Hand == EControllerHand::Right)
	{
		// update teleport arc
		UpdateTeleportArc();
	}
}

USkeletalMeshComponent * AVRController::GetSkeletalMeshComponent()
{
	if (MotionControllerMeshComp)
	{
		return MotionControllerMeshComp;
	}

	return nullptr;
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
	else if(MotionControllerComp->Hand == EControllerHand::Left)
	{
		// destroy teleport cursor spawned by left hand controller.
		TeleportCursor->Destroy();
		TeleportSplineComp->DestroyComponent();
	}
}

EControllerHand AVRController::GetHand()
{
	return MotionControllerComp->Hand;
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
	RV_TraceParams.bTraceComplex = true;
	RV_TraceParams.bTraceAsyncScene = true;
	RV_TraceParams.bReturnPhysicalMaterial = true;

	FVector StartPos;
	FVector EndPos;

	//set start position to socket location on skeletal mesh
	StartPos = MotionControllerMeshComp->GetSocketLocation("teleport_start_pos");
	FRotator ControllerRot = GetActorRotation();
	EndPos = StartPos + (ControllerRot.Vector() * MaxTeleportDistance);

	//  do the line trace
	bool DidTrace = GetWorld()->LineTraceSingleByChannel(
		Hit,        //result
		StartPos,        //start
		EndPos,        //end
		ECC_Pawn,    //collision channel
		RV_TraceParams
	);

	// check if trace was successful and that blocking object has a physical material with surface type set to "Floor"
	if (DidTrace && Hit.PhysMaterial->SurfaceType == SurfaceType1) //SurfaceType1 == "Floor"
	{
		PreviousTeleportPosition = CurrentTeleportPosition;
		CurrentTeleportPosition = Hit.ImpactPoint;
		if (!TeleportCursor->IsVisible())
		{
			TeleportCursor->SetVisible(true);
		}
		bValidTeleportPosition = true;
	}
	else
	{
		if (TeleportCursor->IsVisible())
		{
			TeleportCursor->SetVisible(false);
		}
		PreviousTeleportPosition = CurrentTeleportPosition = FVector::ZeroVector;
		bValidTeleportPosition = false;
	}	
}

bool AVRController::IsValidTeleportLocation()
{
	return bValidTeleportPosition;
}


void AVRController::CancelTeleport()
{
	if (TeleportCursor->IsVisible())
	{
		TeleportCursor->SetVisible(false);
	}
	bValidTeleportPosition = false;
}

bool AVRController::OnTeleport(FVector& OutTeleportLocation)
{
	OutTeleportLocation = CurrentTeleportPosition;
	bValidTeleportPosition = false;

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

void AVRController::UpdateTeleportArc()
{
	TArray<FVector> Path;

	if (bValidTeleportPosition)
	{
		Path.Add(MotionControllerMeshComp->GetSocketLocation("teleport_start_pos"));
		Path.Add(CurrentTeleportPosition);
		DrawTeleportArc(Path);
	}
	else
	{
		// empty path
		DrawTeleportArc(Path);
	}

}

void AVRController::UpdateTeleportSpline(const TArray<FVector>& SplinePath)
{
	TeleportSplineComp->ClearSplinePoints(false);

	for (int32 i = 0; i < SplinePath.Num(); i++)
	{
		FVector LocalPosition = TeleportSplineComp->GetComponentTransform().InverseTransformPosition(SplinePath[i]);
		FSplinePoint Point(i, LocalPosition, ESplinePointType::Curve);
		TeleportSplineComp->AddPoint(Point, false);
	}
	TeleportSplineComp->UpdateSpline();
}

void AVRController::DrawTeleportArc(const TArray<FVector>& TeleportArc)
{
	UpdateTeleportSpline(TeleportArc);

	for (USplineMeshComponent* SplineMesh : TeleportSpineMeshPool)
	{
		SplineMesh->SetVisibility(false);
	}

	int32 NumSegments = TeleportArc.Num() - 1;
	for (int32 i = 0; i < NumSegments; i++)
	{
		if (TeleportSpineMeshPool.Num() <= 1)
		{
			USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(this);
			//SplineMesh->SetMobility(EComponentMobility::Movable);
			//SplineMesh->AttachToComponent(MotionControllerComp, FAttachmentTransformRules::KeepRelativeTransform);
			SplineMesh->SetStaticMesh(TeleportSplineMesh);
			SplineMesh->SetMaterial(0, TeleportSplineMaterial);
			SplineMesh->RegisterComponent();
			SplineMesh->SetMobility(EComponentMobility::Movable);

			TeleportSpineMeshPool.Add(SplineMesh);
		}

		USplineMeshComponent* SplineMesh = TeleportSpineMeshPool[i];

		SplineMesh->SetStartAndEnd(
			TeleportSplineComp->GetWorldLocationAtSplinePoint(i),
			TeleportSplineComp->GetTangentAtSplinePoint(i, ESplineCoordinateSpace::World),
			TeleportSplineComp->GetWorldLocationAtSplinePoint(i + 1),
			TeleportSplineComp->GetTangentAtSplinePoint(i + 1, ESplineCoordinateSpace::World),
			true
		);

		SplineMesh->SetVisibility(true);
	}
}

