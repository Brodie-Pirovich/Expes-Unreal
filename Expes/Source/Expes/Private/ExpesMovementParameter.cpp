// Fill out your copyright notice in the Description page of Project Settings.


#include "ExpesMovementParameter.h"

UExpesMovementParameter::UExpesMovementParameter()
{
    MaxWalkSpeed = 600.0f;
    MaxAcceleration = 1000.0f;
    AirControl = 0.0f;
    GroundAccelerationMultiplier = 10.0f;
    AirAccelerationMultiplier = 2.0f;
    SpeedUpperLimit = 3000.0f;
    NumOfJumpRequestToleranceFrames = 5;
    BrakingDecelerationWalking = MaxAcceleration;
    PenaltyScaleFactorForHoldingJumpButton = 0.25f;
}