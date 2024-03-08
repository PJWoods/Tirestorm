// Copyright Epic Games, Inc. All Rights Reserved.

#include "TirestormGameMode.h"
#include "TirestormPlayerController.h"

ATirestormGameMode::ATirestormGameMode()
{
	PlayerControllerClass = ATirestormPlayerController::StaticClass();
}
