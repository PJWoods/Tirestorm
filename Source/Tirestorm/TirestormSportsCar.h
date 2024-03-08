// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TirestormPawn.h"
#include "TirestormSportsCar.generated.h"

/**
 *  Sports car wheeled vehicle implementation
 */
UCLASS(abstract)
class TIRESTORM_API ATirestormSportsCar : public ATirestormPawn
{
	GENERATED_BODY()
	
public:

	ATirestormSportsCar();
};
