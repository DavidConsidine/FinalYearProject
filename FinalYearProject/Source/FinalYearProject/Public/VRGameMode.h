// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "VRGameMode.generated.h"


class AVRCharacter;

 // game modes
UENUM(BlueprintType)
enum EGameMode
{
	MenuSelect	UMETA(DisplayName = "MenuSelect"),	// Default starting mode, mode selection
	FreeRoam	UMETA(DisplayName = "FreeRoam"),	// no time limit, free to move around the space, interact with objects
	TimedLow	UMETA(DisplayName = "TimedLow"),	// Timed mode, required items positioned on low level shelves
	TimedMid	UMETA(DisplayName = "TimedMid"),	// Timed mode, required items positioned on mid-level shelves
	TimedHigh	UMETA(DisplayName = "TimedHigh"),	// Timed mode, required items positioned on high level shelves
	TimedAll	UMETA(DisplayName = "TimedAll"),	// Timed mode, required items positioned on all level shelves
	ModeReset	UMETA(DisplayName = "ModeReset")	// Transitions between timed/free roam modes to menu select mode
};

// delegate to broadcast shopping list to ui in game
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FShoppingListBroadcastDelegate, const TArray<FString>&, ItemList);

// delegate to broadcast change in game mode
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGameModeChangeDelegate, EGameMode, NewGameMode);

// delegate to broadcast game status (paused or unpaused)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGamePauseStatusDelegate, const bool, Paused);

// delegate to broadcast mode reset.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGameResetDelegate);

UCLASS()
class FINALYEARPROJECT_API AVRGameMode : public AGameModeBase
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;


	void GenerateObjectiveItemList(int32 NumOfItems, int32 ItemList);

	void GenerateObjectiveItemListFromAllItemLists();

	UPROPERTY(EditDefaultsOnly, Category = "Timer")
	float TimePerRound;

	UPROPERTY(EditDefaultsOnly, Category = "CameraFadeDelay")
	float FadeDelayTime;

	UPROPERTY(EditDefaultsOnly, Category = "Game Mode Selection")
	TEnumAsByte<EGameMode> CurrentGameMode;

	FTimerHandle RoundTimerHandle;

	FTimerHandle FadeDelayTimerHandle;

	AVRCharacter* PlayerChar;

	FVector PlayerStartPos;

	FRotator PlayerStartRot;

	UPROPERTY(EditDefaultsOnly)
	int32 NumberOfItemsRequired = 10;

	// lists for high, mid and low shelf items
	TArray<TArray<FString>> ItemLists{ {
			"Apple",
			"Watermelon",
			"Lemon",
			"Avocado",
			"Red Cabbage",
			"Bread Loaf",
			"Pretzel",
			"Cookie",
			"Hotdog",
			"Piggy Bank",
			"Chopping Board",
			"Cola Bottle",
			"Ginger",
			"Alarm Clock",
			"Sweet Potato",
			"White Onion",
			"Coconut",
			"Carrot"
		}, 
		{
			"Pineapple",
			"Banana",
			"Pear",
			"Lime",
			"Broccoli",
			"Doughnut",
			"Baguette",
			"Chocolate Brownie",
			"Pizza",
			"Hat",
			"Soda Can",
			"Garlic",
			"Wooden Spoon",
			"Scissors",
			"Cucumber",
			"Cauliflower"
		}, 
		{
			"Mango",
			"Orange",
			"Pumpkin",
			"Green Cabbage",
			"Artichoke",
			"Croissant",
			"Candy Cane",
			"Muffin",
			"Fish",
			"Light Bulb",
			"Ice Cream Cone",
			"Squash",
			"Toy Car",
			"Potato",
			"Tomato",
			"Yellow Onion",
			"Red Onion",
			"Corn"
		} 
	};

	// generated list of items the player will need to collect.
	TArray<FString> ItemList;

	void EndTimedGame();

	void PrepareGameMode();

	/** Starts timer countdown for a timed round. returns true if timer successfully activated. Returns false otherwise*/
	bool StartRoundTimer();

	void RepositionPlayer();

	// broadcast new list generated for round


public:
	AVRGameMode();

	UFUNCTION(BlueprintCallable, Category = "GameMode")
	EGameMode GetCurrentGameMode();

	UFUNCTION(BlueprintCallable, Category = "Setter")
	void SetCurrentGameMode(EGameMode NewGameMode);

	UFUNCTION(BlueprintCallable, Category = "Gameplay Timer")
	int GetTimeRemaining();

	void ItemCollected(FString ItemTag);

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FShoppingListBroadcastDelegate OnItemListUpdated;

	UPROPERTY()
	FGameResetDelegate OnGameReset;

	UPROPERTY()
	FGameModeChangeDelegate OnGameModeChanged;

	UPROPERTY()
	FGamePauseStatusDelegate OnGamePauseStateChanged;

	void PauseGame();

	UFUNCTION(BlueprintCallable)
	void ResumeGame();

	UFUNCTION(BlueprintCallable)
	void ReturnToModeSelect();
	
};
