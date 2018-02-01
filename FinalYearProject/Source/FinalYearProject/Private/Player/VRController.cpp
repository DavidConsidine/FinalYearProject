#include "VRController.h"
#include "MotionControllerComponent.h"
#include "Components/StaticMeshComponent.h"

AVRController::AVRController()
{
	//PrimaryActorTick.bCanEverTick = true;
	
	MotionControllerComp = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionControllerComp"));
	RootComponent = MotionControllerComp;

	MotionControllerMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MotionControllerMeshComp"));
	MotionControllerMeshComp->SetupAttachment(RootComponent);

	

}

void AVRController::BeginPlay()
{
	Super::BeginPlay();
}

void AVRController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	
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

