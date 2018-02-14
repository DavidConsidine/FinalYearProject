// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "VRGameMode.generated.h"

/**
 * 
 */

UENUM()
enum EGameMode
{
	MenuSelect,
	Timed,
	Endless
};


UCLASS()
class FINALYEARPROJECT_API AVRGameMode : public AGameModeBase
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Timer")
	float TimePerRound;

	UPROPERTY(EditDefaultsOnly, Category = "Game Mode Selection")
	TEnumAsByte<EGameMode> GameModeSelected;

	FTimerHandle TimerHandle;

	void EndTimedGame();


public:
	AVRGameMode();
	
};
