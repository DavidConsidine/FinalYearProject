// Fill out your copyright notice in the Description page of Project Settings.

#include "BasePickup.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "VRGameMode.h"

// Sets default values
ABasePickup::ABasePickup()
{
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MeshComp->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

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
		AVRGameMode* GM = Cast<AVRGameMode>(World->GetAuthGameMode());
		if (GM != nullptr)
		{
			GM->ItemCollected(ItemTag);
			RemoveFromBasket();
		}
	}
}

void ABasePickup::RemoveFromBasket()
{
	// play sound

	// TODO: store reset location, disable object and move back to original position for mode reset.
	SetActorHiddenInGame(true);
	MeshComp->SetSimulatePhysics(false);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABasePickup::BeginPlay()
{
	StartingTransform = GetActorTransform();

	UWorld* World = GetWorld();
	if (World)
	{
		AVRGameMode* GM = Cast<AVRGameMode>(World->GetAuthGameMode());
		if (GM != nullptr)
		{
			GM->OnGameReset.AddDynamic(this, &ABasePickup::ResetVisibilityAndPosition);
		}
	}
}

void ABasePickup::ResetVisibilityAndPosition()
{
	MeshComp->SetAllPhysicsLinearVelocity(FVector::ZeroVector, false);
	MeshComp->SetAllPhysicsAngularVelocity(FVector::ZeroVector, false);
	SetActorTransform(StartingTransform);
	if (bHidden)
	{
		SetActorHiddenInGame(false);
		MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		MeshComp->SetSimulatePhysics(true);
	}
}
