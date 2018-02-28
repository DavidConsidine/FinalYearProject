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
	if (World && World->GetTimerManager().IsTimerActive(TimerHandle))
	{
		return World->GetTimerManager().GetTimerRemaining(TimerHandle);
	}
	
	return -1;
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
		
		break;
	case TimedLow:
		// initialise timer for round.
		StartRoundTimer();
		// generate list of required items (specifically low positioned items)
		// enable player movement
		PlayerChar->SetCanMove(true);
		break;
	case TimedMid:
		// initialise timer for round.
		StartRoundTimer();
		// generate list of required items (specifically mid positioned items)
		PlayerChar->SetCanMove(true);
		// enable player movement
		break;
	case TimedHigh:
		// initialise timer for round.
		StartRoundTimer();
		// generate list of required items (specifically high positioned items)
		// enable player movement
		PlayerChar->SetCanMove(true);
		break;
	case TimedAll:
		// initialise timer for round.
		StartRoundTimer();
		// generate list of required items (items in any position)
		// enable player movement
		PlayerChar->SetCanMove(true);
		break;
	case FreeRoam:
		// enable player movement
		PlayerChar->SetCanMove(true);
		break;
	case ModeReset:
		UE_LOG(LogTemp, Warning, TEXT("ModeReset"));
		// disable player movement
		PlayerChar->SetCanMove(false);
		Cast<APlayerController>(PlayerChar->GetController())->PlayerCameraManager->StartCameraFade(0.0, 1.0, 0.5f, FLinearColor::Black, false, false);
		// if timer is active, disable it.
		// fade player back to starting position
		PlayerChar->SetActorLocation(PlayerStartPos);
		PlayerChar->SetActorRotation(PlayerStartRot);
		/*auto PC = GetWorld()->GetFirstPlayerController();
		if (PC)
		{
			UE_LOG(LogTemp, Warning, TEXT("valid PC"));
			RestartPlayerAtPlayerStart(PC, FindPlayerStart(PC, "DefaultPlayerStartPosition"));
		}*/
		// previous mode was score based, display score
		// set to mode select mode
		//recall prepare game mode
		SetCurrentGameMode(MenuSelect);
		break;
	}
}

bool AVRGameMode::StartRoundTimer()
{
	UWorld * World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(TimerHandle, this, &AVRGameMode::EndTimedGame, TimePerRound);
		return true;
	}
	return false;
}
