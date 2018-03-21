// Fill out your copyright notice in the Description page of Project Settings.

#include "BasePickup.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ABasePickup::ABasePickup()
{
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	//MeshComp->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);

	MeshComp->SetSimulatePhysics(true);

	bGrabbed = false;
}



void ABasePickup::Grab(USceneComponent* Parent, FName SocketName)
{
	if (!Parent)
	{
		return;
	}

	bGrabbed = true;
	// disable simulating physics on static mesh
	MeshComp->SetSimulatePhysics(false);

	// attach root component to passed in component (i.e attach object to controller)
	FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false);
	//RootComponent->AttachToComponent(AttachToComp, TransformRules);

	AttachToComponent(Parent, TransformRules, SocketName);

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

void ABasePickup::AddedToBasket()
{
	// set timer 
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(TimerHandle, this, &ABasePickup::RemoveFromBasket, 1.5f);
	}
	
}

void ABasePickup::RemoveFromBasket()
{
	// spawn particle system
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ParticleFX, GetActorTransform());

	// destroy object	- TODO: store reset location, disable object and move back to original position for mode reset.
	Destroy();
}

