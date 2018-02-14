// Fill out your copyright notice in the Description page of Project Settings.

#include "VRGameMode.h"
#include "Engine/Engine.h"




AVRGameMode::AVRGameMode()
	: Super()
{
	UE_LOG(LogTemp, Warning, TEXT("AVRGameMode::AVRGameMode"))
	TimePerRound = 120.0f;

	GameModeSelected = Timed;
}

void AVRGameMode::BeginPlay()
{
	Super::BeginPlay();
	if (GameModeSelected == Timed)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			World->GetTimerManager().SetTimer(TimerHandle, this, &AVRGameMode::EndTimedGame, TimePerRound);

		}
	}
	
}

void AVRGameMode::EndTimedGame()
{
	GEngine->AddOnScreenDebugMessage(0, 0.5f, FColor::Yellow, "Time's up", true);
	UE_LOG(LogTemp, Warning, TEXT("Time's up!"))
}