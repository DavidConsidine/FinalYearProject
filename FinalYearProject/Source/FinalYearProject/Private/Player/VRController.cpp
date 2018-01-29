#include "VRController.h"
#include "MotionControllerComponent.h"
#include "Components/StaticMeshComponent.h"

AVRController::AVRController()
{
	MotionControllerComp = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionControllerComp"));

	MotionControllerMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MotionControllerMeshComp"));
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