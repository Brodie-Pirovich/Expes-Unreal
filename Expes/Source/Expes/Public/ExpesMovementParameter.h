// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ExpesMovementStyle.h"
#include "Engine/DataAsset.h"
#include "ExpesMovementParameter.generated.h"

/**
 * 
 */
UCLASS()
class EXPES_API UExpesMovementParameter : public UObject
{
	GENERATED_BODY()

public:
    UExpesMovementParameter();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++Property")
    float MaxWalkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++Property")
    float MaxAcceleration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++Property")
    float AirControl;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++Property")
    float AccelerationConstant;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++Property")
    float SpeedLimitAlongAcceleration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++Property")
    float GroundAccelerationMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++Property")
    float AirAccelerationMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++Property")
    float SpeedUpperLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++Property")
    int NumOfJumpRequestToleranceFrames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++Property")
    float BrakingDecelerationWalking;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++Property")
    float PenaltyScaleFactorForHoldingJumpButton;
	
};
