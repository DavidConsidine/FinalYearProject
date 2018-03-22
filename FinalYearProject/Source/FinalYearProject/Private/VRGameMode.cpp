// Fill out your copyright notice in the Description page of Project Settings.

#include "VRGameMode.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "VRCharacter.h"
#include "Camera/PlayerCameraManager.h"




AVRGameMode::AVRGameMode()
	: Super()
{
	UE_LOG(LogTemp, Warning, TEXT("AVRGameMode::AVRGameMode"))
	TimePerRound = 120.0f;

	FadeDelayTime = 1.0f;

	CurrentGameMode = MenuSelect;
}

EGameMode AVRGameMode::GetCurrentGameMode()
{
	return CurrentGameMode;
}

void AVRGameMode::SetCurrentGameMode(EGameMode NewGameMode)
{
	CurrentGameMode = NewGameMode;

	PrepareGameMode();
}

int AVRGameMode::GetTimeRemaining()
{
	UWorld* World = GetWorld();
	if (World && World->GetTimerManager().IsTimerActive(RoundTimerHandle))
	{
		return World->GetTimerManager().GetTimerRemaining(RoundTimerHandle);
	}
	
	return -1;
}

void AVRGameMode::ItemCollected(FString ItemTag)
{
	if(ItemList.Contains(ItemTag))
	{
		ItemList.Remove(ItemTag);
		// update score

		// update ui list display
		OnItemListUpdated.Broadcast(ItemList);
	}
}

void AVRGameMode::BeginPlay()
{
	Super::BeginPlay();

	// get reference to player
	PlayerChar = Cast<AVRCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	
	PlayerStartPos = PlayerChar->GetActorLocation();
	PlayerStartRot = PlayerChar->GetActorRotation();

	PrepareGameMode();
}

void AVRGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//OnItemListUpdated.Broadcast(ItemList);
}

// helper function to generate item list
void AVRGameMode::GenerateItemList()
{
	switch (CurrentGameMode)
	{
	case TimedLow:
		break;
	case TimedMid:
		break;
	case TimedHigh:
		break;
	case TimedAll:
		break;
	}
	ItemList.Add("Sphere");
	ItemList.Add("Cube");
	ItemList.Add("Pyramid");

}

void AVRGameMode::EndTimedGame()
{
	GEngine->AddOnScreenDebugMessage(0, 0.5f, FColor::Yellow, "Time's up", true);
	UE_LOG(LogTemp, Warning, TEXT("Time's up!"));

	// round finished, return to mode select
	SetCurrentGameMode(ModeReset);
	
}

// determines what needs to be done for each game mode
void AVRGameMode::PrepareGameMode()
{
	// Game mode has changed, perform the respective task for the new mode

	switch (CurrentGameMode)
	{
	case MenuSelect:
		// disable player movement
		PlayerChar->SetCanMove(false);
		PlayerChar->EnableMenuComponents();
		break;
	case TimedLow:
		//deactivate/hide menu components from vrcharacter
		PlayerChar->DisableMenuComponents();
		// initialise timer for round.
		StartRoundTimer();
		// generate list of required items (specifically low positioned items)
		// enable player movement
		PlayerChar->SetCanMove(true);
		GenerateItemList();
		OnItemListUpdated.Broadcast(ItemList);
		break;
	case TimedMid:
		//deactivate/hide menu components from vrcharacter
		PlayerChar->DisableMenuComponents();
		// initialise timer for round.
		StartRoundTimer();
		// generate list of required items (specifically mid positioned items)
		// enable player movement
		PlayerChar->SetCanMove(true);
		GenerateItemList();
		OnItemListUpdated.Broadcast(ItemList);
		break;
	case TimedHigh:
		//deactivate/hide menu components from vrcharacter
		PlayerChar->DisableMenuComponents();
		// initialise timer for round.
		StartRoundTimer();
		// generate list of required items (specifically high positioned items)
		// enable player movement
		PlayerChar->SetCanMove(true);
		GenerateItemList();
		OnItemListUpdated.Broadcast(ItemList);
		break;
	case TimedAll:
		//deactivate/hide menu components from vrcharacter
		PlayerChar->DisableMenuComponents();
		// initialise timer for round.
		StartRoundTimer();
		// generate list of required items (items in any position)
		// enable player movement
		PlayerChar->SetCanMove(true);
		GenerateItemList();
		OnItemListUpdated.Broadcast(ItemList);
		break;
	case FreeRoam:
		//deactivate/hide menu components from vrcharacter
		PlayerChar->DisableMenuComponents();
		// enable player movement
		PlayerChar->SetCanMove(true);
		break;
	case ModeReset:
		UE_LOG(LogTemp, Warning, TEXT("ModeReset"));
		// disable player movement
		PlayerChar->SetCanMove(false);
		Cast<APlayerController>(PlayerChar->GetController())->PlayerCameraManager->StartCameraFade(0.0, 1.0, FadeDelayTime, FLinearColor::Black, false, true);
		OnGameReset.Broadcast();
		// set timer for delay during camera fade so reposition happens at max fade out.
		UWorld* World = GetWorld();
		if (World)
		{
			World->GetTimerManager().SetTimer(FadeDelayTimerHandle, this, &AVRGameMode::RepositionPlayer, FadeDelayTime);
		}
		break;
	}
}

bool AVRGameMode::StartRoundTimer()
{
	UWorld * World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(RoundTimerHandle, this, &AVRGameMode::EndTimedGame, TimePerRound);
		return true;
	}
	return false;
}

void AVRGameMode::RepositionPlayer()
{
	// fade player back to starting position
	PlayerChar->SetActorLocation(PlayerStartPos);
	PlayerChar->SetActorRotation(PlayerStartRot);

	Cast<APlayerController>(PlayerChar->GetController())->PlayerCameraManager->StartCameraFade(1.0f, 0.0f, FadeDelayTime, FLinearColor::Black, false, true);
	
	// set to mode select mode
	//recall prepare game mode
	SetCurrentGameMode(MenuSelect);
}
