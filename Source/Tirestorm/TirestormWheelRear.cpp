// Copyright Epic Games, Inc. All Rights Reserved.

#include "TirestormWheelRear.h"
#include "UObject/ConstructorHelpers.h"

UTirestormWheelRear::UTirestormWheelRear()
{
	AxleType = EAxleType::Rear;
	bAffectedByHandbrake = true;
	bAffectedByEngine = true;
}