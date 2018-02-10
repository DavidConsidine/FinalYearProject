#include "VRController.h"
#include "MotionControllerComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "BasePickup.h"

AVRController::AVRController()
{
	//PrimaryActorTick.bCanEverTick = true;
	
	MotionControllerComp = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionControllerComp"));
	RootComponent = MotionControllerComp;

	MotionControllerMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MotionControllerMeshComp"));
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
}

void AVRController::BeginPlay()
{
	Super::BeginPlay();
	
}

void AVRController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


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
	
	float MinDeltaDist = 1000.f; // arbitrary, large value

	
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
		CurrentPickupObject->Grab(RootComponent);
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

