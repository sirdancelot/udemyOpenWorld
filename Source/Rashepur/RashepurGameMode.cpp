// Copyright Epic Games, Inc. All Rights Reserved.

#include "RashepurGameMode.h"
#include "RashepurCharacter.h"
#include "UObject/ConstructorHelpers.h"

ARashepurGameMode::ARashepurGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
