// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "VRGameMode.generated.h"


class AVRCharacter;

 // game modes
UENUM()
enum EGameMode
{
	MenuSelect,	// Default starting mode, mode selection
	FreeRoam,	// no time limit, free to move around the space, interact with objects
	TimedLow,	// Timed mode, required items positioned on low level shelves
	TimedMid,	// Timed mode, required items positioned on mid-level shelves
	TimedHigh,	// Timed mode, required items positioned on high level shelves
	TimedAll,	// Timed mode, required items positioned on all level shelves
	ModeReset	// Transitions between timed/free roam modes to menu select mode
};


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

	void EndTimedGame();

	void PrepareGameMode();

	/** Starts timer countdown for a timed round. returns true if timer successfully activated. Returns false otherwise*/
	bool StartRoundTimer();

	void RepositionPlayer();

public:
	AVRGameMode();

	EGameMode GetCurrentGameMode();

	UFUNCTION(BlueprintCallable, Category = "Setter")
	void SetCurrentGameMode(EGameMode NewGameMode);

	UFUNCTION(BlueprintCallable, Category = "Gameplay Timer")
	int GetTimeRemaining();
	
};
