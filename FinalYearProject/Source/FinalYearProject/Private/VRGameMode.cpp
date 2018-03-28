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
void AVRGameMode::GenerateObjectiveItemList(int32 NumOfItems, int32 ShelfItemList)
{
	if (NumOfItems == 0)
	{
		return;
	}

	if (NumOfItems > ItemLists[ShelfItemList].Num())
	{
		NumOfItems = ItemLists[ShelfItemList].Num();
	}

	const int32 TotalNumInList = ItemLists[ShelfItemList].Num();
	TArray<int32> ItemTracker;
	for (int32 i = 0; i < TotalNumInList; i++)
	{
		ItemTracker.Add(0);
	}
	
	for (int32 i = 0; i < NumOfItems; )
	{
		int32 RandNum = FMath::RandRange(0, TotalNumInList - 1);
		if (ItemTracker[RandNum] == 0)
		{
			ItemTracker[RandNum]++;
			ItemList.Add(ItemLists[ShelfItemList][RandNum]);
			i++;
		}
	}
}

void AVRGameMode::GenerateObjectiveItemListFromAllItemLists()
{
	TArray<int32> ItemsPerList{ 0,0,0 };

	for (int32 i = 0; i < NumberOfItemsRequired; i++)
	{
		int32 RandNum = FMath::RandRange(0, ItemLists.Num() - 1);
		ItemsPerList[RandNum]++;
	}

	for (int32 i = 0; i < ItemsPerList.Num(); i++)
	{
		GenerateObjectiveItemList(ItemsPerList[i], i);
	}

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
		GenerateObjectiveItemList(NumberOfItemsRequired, 0);
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
		GenerateObjectiveItemList(NumberOfItemsRequired, 1);
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
		GenerateObjectiveItemList(NumberOfItemsRequired, 2);
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
		GenerateObjectiveItemListFromAllItemLists();
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
	ItemList.Empty();
	SetCurrentGameMode(MenuSelect);
}
