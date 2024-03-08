// Copyright Epic Games, Inc. All Rights Reserved.

#include "TirestormWheelFront.h"
#include "UObject/ConstructorHelpers.h"

UTirestormWheelFront::UTirestormWheelFront()
{
	AxleType = EAxleType::Front;
	bAffectedBySteering = true;
	MaxSteerAngle = 40.f;
}