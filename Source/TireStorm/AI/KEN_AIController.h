// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "KEN_AIController.generated.h"

class AWheeledVehiclePawn;

UCLASS()
class TIRESTORM_API AKEN_AIController : public AAIController
{
	GENERATED_BODY()
	AKEN_AIController();

public:
	// Tweak these values to your liking
	UPROPERTY(BlueprintReadWrite, EditAnywhere) float MinDistanceToPlayer = 1000.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) float MaxDistanceToPlayer = 1500.0f;

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

private:
	void FollowTarget();
	void EvaluateTargetPawn();
	void CheckAndUpdatePawnsList();
	void DrawDebug();

	AWheeledVehiclePawn* m_pCurrentTarget = nullptr;
	TArray<AWheeledVehiclePawn*> m_aAllPawns;

	FVector m_vLastTargetLocation = FVector::ZeroVector;
	FVector m_vLastTargetDirection = FVector::ZeroVector;
};
