// Copyright Epic Games, Inc. All Rights Reserved.

#include "SloMoPointAndClickGameMode.h"
#include "SloMoPointAndClickPlayerController.h"
#include "SloMoPointAndClickCharacter.h"
#include "UObject/ConstructorHelpers.h"

ASloMoPointAndClickGameMode::ASloMoPointAndClickGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = ASloMoPointAndClickPlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/TopDownCPP/Blueprints/BP_SloMoCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}