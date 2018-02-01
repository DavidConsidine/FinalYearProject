// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "FinalYearProjectGameMode.h"
#include "FinalYearProjectHUD.h"
#include "FinalYearProjectCharacter.h"
#include "UObject/ConstructorHelpers.h"

AFinalYearProjectGameMode::AFinalYearProjectGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AFinalYearProjectHUD::StaticClass(); //uncomment to get crosshair HUD
}
