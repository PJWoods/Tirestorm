// Fill out your copyright notice in the Description page of Project Settings.


#include "KEN_AIController.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "GameFramework/Character.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "WheeledVehiclePawn.h"
#include "EngineUtils.h"

AKEN_AIController::AKEN_AIController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AKEN_AIController::BeginPlay()
{
	Super::BeginPlay();
	SetTickableWhenPaused(false);
}

void AKEN_AIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	DrawDebug();
	EvaluateTargetPawn();
	FollowTarget();
}

void AKEN_AIController::CheckAndUpdatePawnsList()
{
	APawn* playerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	TArray<AWheeledVehiclePawn*> pawns;
	if (m_aAllPawns.Num() <= 0)
	{
		for (TActorIterator<AActor> it(GetWorld()); it; ++it)
		{
			if (AWheeledVehiclePawn* pawn = Cast<AWheeledVehiclePawn>(*it))
			{
				if (pawn == GetPawn())
					continue;
				if (pawn == playerPawn)
					continue;
				pawns.Add(pawn);
			}
		}
		m_aAllPawns = pawns;
		m_pCurrentTarget = nullptr;
		return;
	}

	//if(m_aAllPawns.Num() != pawns.Num())
	//{
	//	m_aAllPawns = pawns;
	//	m_pCurrentTarget = nullptr;
	//	return;
	//}

	//bool foundDiff = false;
	//for(AWheeledVehiclePawn* pawn : pawns)
	//{
	//	if(m_aAllPawns.Find(pawn) == INDEX_NONE)
	//	{
	//		foundDiff = true;
	//		break;
	//	}
	//}
	//if (!foundDiff)
	//{
	//	for (AWheeledVehiclePawn* pawn : m_aAllPawns)
	//	{
	//		if (pawns.Find(pawn) == INDEX_NONE)
	//		{
	//			foundDiff = true;
	//			break;
	//		}
	//	}
	//}
	//if(foundDiff)
	//{
	//	m_aAllPawns = pawns;
	//	m_pCurrentTarget = nullptr;
	//}
}

void AKEN_AIController::EvaluateTargetPawn()
{
	CheckAndUpdatePawnsList();
	if(m_aAllPawns.Num() <= 0)
		return;
	if (!m_pCurrentTarget)
	{
		const int32 targetIndex = FMath::RandRange(0, m_aAllPawns.Num() - 1);
		m_pCurrentTarget = m_aAllPawns[targetIndex];
	}
	else if(m_pCurrentTarget->GetVelocity().IsNearlyZero(0.01f))
	{
		if(m_aAllPawns.Num() > 1)
		{
			const int32 failSafeCount = 100;
			int32 currentCount = 0;

			AWheeledVehiclePawn* newTarget = m_pCurrentTarget;
			while (m_pCurrentTarget == newTarget && currentCount < failSafeCount)
			{
				const int32 targetIndex = FMath::RandRange(0, m_aAllPawns.Num() - 1);
				newTarget = m_aAllPawns[targetIndex];
				++currentCount;
			}
			m_pCurrentTarget = newTarget;
		}
		else
			m_pCurrentTarget = nullptr;
	}
}

void AKEN_AIController::FollowTarget()
{
	if (!m_pCurrentTarget)
		return;
	APawn* aiPawn = GetPawn();
	if (!aiPawn)
		return;
	UChaosWheeledVehicleMovementComponent* moveComponent = aiPawn->FindComponentByClass<UChaosWheeledVehicleMovementComponent>();
	if (!moveComponent)
		return;

	const FVector targetPawnLocation = m_pCurrentTarget->GetActorLocation();
	const FVector targetForwardVector = m_pCurrentTarget->GetActorForwardVector();
	const FVector aiLocation = aiPawn->GetActorLocation();
	FVector directionToTargetPawn = targetPawnLocation - aiLocation;
	directionToTargetPawn.Normalize();

	const float desiredDistanceToPlayer = MaxDistanceToPlayer;


	const FVector aiForwardVector = aiPawn->GetActorForwardVector();
	const FVector toTargetPawnProjected = FVector::VectorPlaneProject(directionToTargetPawn, FVector::UpVector).GetSafeNormal();
	const FVector forwardProjected = FVector::VectorPlaneProject(aiForwardVector, FVector::UpVector).GetSafeNormal();

	const float forwardDotPlayer = FVector::DotProduct(forwardProjected, toTargetPawnProjected);
	const float rightDotPlayer = FVector::DotProduct(FVector::CrossProduct(FVector::UpVector, forwardProjected), toTargetPawnProjected);

	m_vLastTargetLocation = targetPawnLocation - (targetForwardVector * desiredDistanceToPlayer);
	m_vLastTargetDirection = m_vLastTargetLocation - aiLocation;
	m_vLastTargetDirection.Normalize();

	FCollisionQueryParams collisionParams;
	collisionParams.AddIgnoredActor(aiPawn);

	TArray<FHitResult> hitResults;
	const int32 numTraces = 10;
	const float traceSpreadAngle = 45.0f;

	for (int32 i = 0; i < numTraces; ++i)
	{
		const float traceAngle = ((float)i / (float)(numTraces - 1) - 0.5f) * traceSpreadAngle;
		const FVector traceDirection = FMath::Lerp(aiForwardVector, aiForwardVector.RotateAngleAxis(traceAngle, FVector::UpVector), 0.5f);

		// Perform line trace
		FHitResult hitResult;
		const FVector traceStart = aiLocation;
		const FVector traceEnd = aiLocation + traceDirection * 500.f;
		if (GetWorld()->LineTraceSingleByChannel(hitResult, traceStart, traceEnd, ECC_Visibility, collisionParams))
			hitResults.Add(hitResult);
	}

	// Find the average avoidance direction from hit surfaces
	if(hitResults.Num() > 0)
	{
		FVector avoidanceDirection = FVector::ZeroVector;
		for (const FHitResult& hitResult : hitResults)
			avoidanceDirection += hitResult.ImpactNormal;
		avoidanceDirection /= hitResults.Num();

		const float avoidanceStrength = 500.f;
		const FVector newDirection = FVector::VectorPlaneProject(aiForwardVector, FVector::UpVector).GetSafeNormal() + avoidanceDirection * avoidanceStrength;
		m_vLastTargetDirection = newDirection.GetSafeNormal();
	}

	const float dotProduct = FVector::DotProduct(aiForwardVector, m_vLastTargetDirection);
	float steeringValue = FMath::Clamp(rightDotPlayer, -1.f, 1.f);
	float throttleValue = 1.f;
	float brakeValue = 0.f;
	const float distanceToTarget = (m_vLastTargetLocation - aiLocation).Size();
	//if (distanceToTarget < MinDistanceToPlayer)
	//{
	//	throttleValue = 0.2f;
	//	//brakeValue = 1.f;
	//}
	//else if (distanceToTarget > MaxDistanceToPlayer)
	//{
	//	throttleValue = 1.f;
	//}

	//else
		throttleValue = FMath::GetMappedRangeValueClamped(TRange<float>(MinDistanceToPlayer, MaxDistanceToPlayer), TRange<float>(0.5f, 1.f), distanceToTarget);

	moveComponent->SetHandbrakeInput(false);
	if (forwardDotPlayer < 0.f)
	{
		//moveComponent->SetHandbrakeInput(true);
		throttleValue = 1.f;
		//brakeValue = 1.f;

		int32 newSteeringValue = FMath::RandRange(-1, 1);
		if ((float)newSteeringValue != steeringValue)
		{
			if (FMath::RandRange(0, 2) == 1)
				steeringValue = newSteeringValue;
		}
	}

	moveComponent->SetSteeringInput(steeringValue);
	moveComponent->SetThrottleInput(throttleValue);
	moveComponent->SetBrakeInput(brakeValue);
}

void AKEN_AIController::DrawDebug()
{
	if (m_pCurrentTarget)
	{
		DrawDebugSphere(GetWorld(), GetPawn()->GetActorLocation(), 150.f, 32.f, FColor::Red, false);
		DrawDebugLine(GetWorld(), GetPawn()->GetActorLocation(), m_vLastTargetLocation, FColor::White, false);
		DrawDebugSphere(GetWorld(), m_pCurrentTarget->GetActorLocation(), 250.f, 32.f, FColor::White, false);

		DrawDebugDirectionalArrow(GetWorld(), GetPawn()->GetActorLocation(), GetPawn()->GetActorLocation() + m_vLastTargetDirection * 500.f, 50.f, FColor::Green, false);
		DrawDebugSphere(GetWorld(), m_vLastTargetLocation, 100.f, 32.f, FColor::Green, false);
	}
}