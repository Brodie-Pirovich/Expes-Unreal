// Fill out your copyright notice in the Description page of Project Settings.


#include "ExpesMovementParameter.h"

UExpesMovementParameter::UExpesMovementParameter()
{
    MaxWalkSpeed = 600.0f;
    MaxAcceleration = 600.0f;
    AirControl = 0.5f;
    GroundAccelerationMultiplier = 10.0f;
    AirAccelerationMultiplier = 1.5f;
    SpeedUpperLimit = 3000.0f;
    NumOfJumpRequestToleranceTimeInterval = 0.2f;
    NumOfTrailingTimeInterval = 0.2f;
    BrakingDecelerationChainStrafeJump = 50.0f;
    BrakingDecelerationWalking = 600.0f;
    PenaltyScaleFactorForUnchainedStrafeJump = 0.8f;
    PenaltyScaleFactorForHoldingJumpButton = 0.25f;
    JumpZVelocity = 380.0f;
}