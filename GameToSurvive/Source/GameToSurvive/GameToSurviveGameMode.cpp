// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "GameToSurvive.h"
#include "GameToSurviveGameMode.h"
#include "GameToSurviveCharacter.h"

AGameToSurviveGameMode::AGameToSurviveGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Characters/MainCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
