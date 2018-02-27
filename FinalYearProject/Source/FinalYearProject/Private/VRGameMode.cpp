// Fill out your copyright notice in the Description page of Project Settings.

#include "VRGameMode.h"
#include "Engine/Engine.h"




AVRGameMode::AVRGameMode()
	: Super()
{
	UE_LOG(LogTemp, Warning, TEXT("AVRGameMode::AVRGameMode"))
	TimePerRound = 120.0f;

	CurrentGameMode = FreeRoam;
}

EGameMode AVRGameMode::GetCurrentGameMode()
{
	return CurrentGameMode;
}

void AVRGameMode::SetCurrentGameMode(EGameMode NewGameMode)
{
	CurrentGameMode = NewGameMode;
}

int AVRGameMode::GetTimeRemaining()
{
	UWorld* World = GetWorld();
	if (World && World->GetTimerManager().IsTimerActive(TimerHandle))
	{
		return World->GetTimerManager().GetTimerRemaining(TimerHandle);
	}
	
	return -1;
}

void AVRGameMode::BeginPlay()
{
	Super::BeginPlay();

	PrepareGameMode();

		
	
}

void AVRGameMode::EndTimedGame()
{
	GEngine->AddOnScreenDebugMessage(0, 0.5f, FColor::Yellow, "Time's up", true);
	UE_LOG(LogTemp, Warning, TEXT("Time's up!"))
}

// determines what needs to be done for each game mode
void AVRGameMode::PrepareGameMode()
{
	UWorld * World = GetWorld();
	switch (CurrentGameMode)
	{
	case MenuSelect:
		break;
	case FreeRoam:
		break;
	case TimedLow:
	case TimedMid:
	case TimedHigh:
	case TimedAll:
		if (World)
		{
			World->GetTimerManager().SetTimer(TimerHandle, this, &AVRGameMode::EndTimedGame, TimePerRound);
		}
		break;
	default:
		break;
	}
}
