// Fill out your copyright notice in the Description page of Project Settings.

#include "BasePickup.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
ABasePickup::ABasePickup()
{
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	//MeshComp->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);

	MeshComp->SetSimulatePhysics(true);

	bGrabbed = false;
}



void ABasePickup::Grab(USceneComponent * AttachToComp)
{
	if (!AttachToComp)
	{
		return;
	}

	bGrabbed = true;
	// disable simulating physics on static mesh
	MeshComp->SetSimulatePhysics(false);

	// attach root component to passed in component (i.e attach object to controller)
	FAttachmentTransformRules TransformRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false);
	RootComponent->AttachToComponent(AttachToComp, TransformRules);

	

}

void ABasePickup::Drop()
{
	bGrabbed = false;
	// detach root component from controller
	FDetachmentTransformRules TransformRules(EDetachmentRule::KeepWorld, false);
	DetachFromActor(TransformRules);

	// enable simulating physics on static mesh
	MeshComp->SetSimulatePhysics(true);

	
}

void ABasePickup::SetGrabbed(bool Grabbed)
{
	bGrabbed = Grabbed;
}

bool ABasePickup::IsGrabbed()
{
	return bGrabbed;
}

