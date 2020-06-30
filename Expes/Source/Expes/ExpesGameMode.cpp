// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "ExpesGameMode.h"
#include "ExpesHUD.h"
#include "ExpesCharacter.h"
#include "UObject/ConstructorHelpers.h"

AExpesGameMode::AExpesGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AExpesHUD::StaticClass();
}
