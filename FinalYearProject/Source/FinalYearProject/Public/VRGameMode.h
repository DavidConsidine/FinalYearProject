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

UCLASS()
class FINALYEARPROJECT_API AVRGameMode : public AGameModeBase
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

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

	UPROPERTY(BlueprintAssignable)
	FShoppingListBroadcastDelegate OnItemListUpdated;
	
};
