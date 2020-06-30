// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ExpesMovementStyle.h"
#include "ExpesMovementComponent.generated.h"

class UExpesMovementParameter;
class AExpesCharacter;
class ExpesMovementStyle;

/**
 * 
 */
UCLASS()
class EXPES_API UExpesMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
public:
    UExpesMovementComponent();

    void SetMovementStyle(EQLMovementStyle MyStyle);

    EQLMovementStyle GetMovementStyle();

    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    void SetMovementParameter(UExpesMovementParameter* MovementParameterQuake);

    virtual void CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration) override;

    virtual void QueueJump();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    virtual void PostInitProperties() override;

    // check if jump button has been pressed and released in the past k frames, including the current frame
    void CheckJumpInfo();

    void PrepareForNextFrame();

    // If the player has already landed for a frame, and breaking may be applied.
    bool bFallingLastFrame;

    // The multiplier for acceleration when on ground.
    UPROPERTY()
    float GroundAccelerationMultiplier;

    // The multiplier for acceleration when in air.
    UPROPERTY()
    float AirAccelerationMultiplier;

    UPROPERTY()
    float SpeedUpperLimit;

    UPROPERTY()
    int NumOfJumpRequestToleranceFrames;

    UPROPERTY()
    float PenaltyScaleFactorForHoldingJumpButton;

    // in the last k frames, if the jump button has been pressed
    bool bHasJumpPressed;

    int FirstElementIndexForJumpPressed;

    TArray<bool> HasJumpPressedList;

    // in the last k frames, if the jump button has been released
    bool bHasJumpReleased;

    int FirstElementIndexForJumpReleased;

    TArray<bool> HasJumpReleasedList;

    FVector InputVectorCached;

    FVector AccelerationCached;

    TWeakObjectPtr<AExpesCharacter> MyCharacter;

    EQLMovementStyle MovementStyle;
};
