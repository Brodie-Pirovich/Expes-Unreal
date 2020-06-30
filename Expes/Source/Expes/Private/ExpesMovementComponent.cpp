// Fill out your copyright notice in the Description page of Project Settings.


#include "ExpesMovementComponent.h"
#include "GameFramework/Character.h"
#include "ExpesMovementParameter.h"
#include "Expes/ExpesCharacter.h"

//------------------------------------------------------------
//------------------------------------------------------------
UExpesMovementComponent::UExpesMovementComponent()
{
    bFallingLastFrame = false;
    bHasJumpPressed = false;
    bHasJumpReleased = false;

    // according to UE4 source code comment, 1.0f would be more appropriate than the default 2.0f in the engine.
    BrakingFrictionFactor = 1.0f;

    MovementStyle = EQLMovementStyle::Default;
}

//------------------------------------------------------------
//------------------------------------------------------------
void UExpesMovementComponent::BeginPlay()
{
    Super::BeginPlay();
}

//------------------------------------------------------------
//------------------------------------------------------------
void UExpesMovementComponent::PostInitProperties()
{
    Super::PostInitProperties();
}

//------------------------------------------------------------
//------------------------------------------------------------
void UExpesMovementComponent::SetMovementParameter(UExpesMovementParameter* MovementParameterQuake)
{
    MaxWalkSpeed = MovementParameterQuake->MaxWalkSpeed;
    MaxAcceleration = MovementParameterQuake->MaxAcceleration;
    AirControl = MovementParameterQuake->AirControl;
    GroundAccelerationMultiplier = MovementParameterQuake->GroundAccelerationMultiplier;
    AirAccelerationMultiplier = MovementParameterQuake->AirAccelerationMultiplier;
    SpeedUpperLimit = MovementParameterQuake->SpeedUpperLimit;
    NumOfJumpRequestToleranceFrames = MovementParameterQuake->NumOfJumpRequestToleranceFrames;
    BrakingDecelerationWalking = MovementParameterQuake->BrakingDecelerationWalking;
    PenaltyScaleFactorForHoldingJumpButton = MovementParameterQuake->PenaltyScaleFactorForHoldingJumpButton;

    HasJumpPressedList.Init(false, NumOfJumpRequestToleranceFrames);
    FirstElementIndexForJumpPressed = 0;

    HasJumpReleasedList.Init(false, NumOfJumpRequestToleranceFrames);
    FirstElementIndexForJumpReleased = 0;
}

//------------------------------------------------------------
// enum ENetRole
// {
//     ROLE_None,
//     ROLE_SimulatedProxy,
//     ROLE_AutonomousProxy,
//     ROLE_Authority,
//     ROLE_MAX,
// }
// For the single player game, by default ENetRole::ROLE_Authority is used
//------------------------------------------------------------
void UExpesMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    InputVectorCached = ConsumeInputVector();

    MyCharacter = Cast<AExpesCharacter>(CharacterOwner);

    CheckJumpInfo();

    if (!HasValidData() || ShouldSkipUpdate(DeltaTime))
    {
        return;
    }

    // Super tick may destroy/invalidate CharacterOwner or UpdatedComponent, so we need to re-check.
    if (!HasValidData())
    {
        return;
    }

    // See if we fell out of the world.
    const bool bIsSimulatingPhysics = UpdatedComponent->IsSimulatingPhysics();
    if ((!bCheatFlying || bIsSimulatingPhysics) && !CharacterOwner->CheckStillInWorld())
    {
        return;
    }

    // We don't update if simulating physics (eg ragdolls).
    if (bIsSimulatingPhysics)
    {
        ClearAccumulatedForces();
        return;
    }

    AvoidanceLockTimer -= DeltaTime;

    // Allow root motion to move characters that have no controller.
    if (CharacterOwner->IsLocallyControlled() || (!CharacterOwner->Controller && bRunPhysicsWithNoController) || (!CharacterOwner->Controller && CharacterOwner->IsPlayingRootMotion()))
    {
        {
            // We need to check the jump state before adjusting input acceleration, to minimize latency
            // and to make sure acceleration respects our potentially new falling state.
            CharacterOwner->CheckJumpInput(DeltaTime);

            // apply input to acceleration
            Acceleration = ScaleInputAcceleration(ConstrainInputAcceleration(InputVectorCached));
            AccelerationCached = Acceleration;

            AnalogInputModifier = ComputeAnalogInputModifier();
        }

        PerformMovement(DeltaTime);
    }

    if (bUseRVOAvoidance)
    {
        UpdateDefaultAvoidance();
    }

    if (bEnablePhysicsInteraction)
    {
        ApplyDownwardForce(DeltaTime);
        ApplyRepulsionForce(DeltaTime);
    }

    PrepareForNextFrame();
}

//------------------------------------------------------------
//------------------------------------------------------------
void UExpesMovementComponent::CheckJumpInfo()
{
    if (MyCharacter.IsValid())
    {
        if (HasJumpPressedList.Num() > 0)
        {
            HasJumpPressedList[FirstElementIndexForJumpPressed] = MyCharacter->IsJumpButtonDown();

            bHasJumpPressed = false;
            for (auto&& item : HasJumpPressedList)
            {
                if (item)
                {
                    bHasJumpPressed = true;
                    break;
                }
            }
        }

        if (HasJumpReleasedList.Num() > 0)
        {
            HasJumpReleasedList[FirstElementIndexForJumpReleased] = !MyCharacter->IsJumpButtonDown();

            bHasJumpReleased = false;
            for (auto&& item : HasJumpReleasedList)
            {
                if (item)
                {
                    bHasJumpReleased = true;
                    break;
                }
            }
        }
    }
}

//------------------------------------------------------------
//------------------------------------------------------------
void UExpesMovementComponent::PrepareForNextFrame()
{
    // call DoJump to change the movement mode to falling
    if (IsMovingOnGround() && // in current frame the player is on the ground
        bFallingLastFrame && // in last frame the player is falling
        bHasJumpPressed) // the player has recently pressed jump button
    {
        // jump in the next frame
        DoJump(true);
    }

    // play the "huh" sound only if the strafe jump is not successfully chained
    // and regular jump is just performed
    if (IsFalling() && // in current frame the player is falling
        !bFallingLastFrame && // in last frame the player is on the ground
        bHasJumpPressed) // the player has recently pressed jump button
    {
        if (MyCharacter.IsValid())
        {
            MyCharacter->PlaySoundFireAndForget(FName(TEXT("QuakeJump")));
        }
    }

    bFallingLastFrame = !IsMovingOnGround();

    ++FirstElementIndexForJumpPressed;
    if (FirstElementIndexForJumpPressed >= NumOfJumpRequestToleranceFrames)
    {
        FirstElementIndexForJumpPressed = 0;
    }

    ++FirstElementIndexForJumpReleased;
    if (FirstElementIndexForJumpReleased >= NumOfJumpRequestToleranceFrames)
    {
        FirstElementIndexForJumpReleased = 0;
    }
}

//------------------------------------------------------------
//------------------------------------------------------------
void UExpesMovementComponent::CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration)
{
    // Do not update velocity when using root motion or when SimulatedProxy and not simulating root motion - SimulatedProxy are repped their Velocity
    if (!HasValidData() || HasAnimRootMotion() || DeltaTime < MIN_TICK_TIME || (CharacterOwner && CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy && !bWasSimulatingRootMotion))
    {
        return;
    }

    Friction = FMath::Max(0.0f, Friction);
    const float MaxAccel = GetMaxAcceleration();
    float MaxSpeed = GetMaxSpeed();

    // Check if path following requested movement
    bool bZeroRequestedAcceleration = true;
    FVector RequestedAcceleration = FVector::ZeroVector;
    float RequestedSpeed = 0.0f;
    if (ApplyRequestedMove(DeltaTime, MaxAccel, MaxSpeed, Friction, BrakingDeceleration, RequestedAcceleration, RequestedSpeed))
    {
        bZeroRequestedAcceleration = false;
    }

    // todo: bForceMaxAccel always evaluates to false ?!
    if (bForceMaxAccel)
    {
        // Force acceleration at full speed.
        // In consideration order for direction: Acceleration, then Velocity, then Pawn's rotation.
        if (Acceleration.SizeSquared() > SMALL_NUMBER)
        {
            Acceleration = Acceleration.GetSafeNormal() * MaxAccel;
        }
        else
        {
            Acceleration = MaxAccel * (Velocity.SizeSquared() < SMALL_NUMBER ? UpdatedComponent->GetForwardVector() : Velocity.GetSafeNormal());
        }

        AnalogInputModifier = 1.0f;
    }

    // apply braking
    const bool bZeroAcceleration = AccelerationCached.IsZero();
    const bool bVelocityOverMax = IsExceedingMaxSpeed(MaxSpeed);

    // brake is applicable only when
    // --- there is no input
    // --- the player is on the ground in the current frame
    // --- the player is not falling in the last frame
    if (bZeroAcceleration &&
        IsMovingOnGround() &&
        !bFallingLastFrame)
    {
        const FVector OldVelocity = Velocity;

        const float ActualBrakingFriction = (bUseSeparateBrakingFriction ? BrakingFriction : Friction);
        ApplyVelocityBraking(DeltaTime, ActualBrakingFriction, BrakingDeceleration);

        //// Don't allow braking to lower us below max speed if we started above it.
        //if (bVelocityOverMax && Velocity.SizeSquared() < FMath::Square(MaxSpeed) && FVector::DotProduct(Acceleration, OldVelocity) > 0.0f)
        //{
        //    Velocity = OldVelocity.GetSafeNormal() * MaxSpeed;
        //}
    }

    // Apply fluid friction
    if (bFluid)
    {
        Velocity = Velocity * (1.0f - FMath::Min(Friction * DeltaTime, 1.0f));
    }

    // Apply input acceleration
    // This part is of paramount importance to advanced movement!
    if (!bZeroAcceleration)
    {
        // case 1: ground
        if (IsMovingOnGround() && // in current frame the player is on the ground
            !bFallingLastFrame) // in last frame the player is on the ground as well
        {
            Velocity += AccelerationCached * GroundAccelerationMultiplier * DeltaTime;
            Velocity = Velocity.GetClampedToMaxSize(MaxSpeed);
        }

        // case 2: air strafe
        else if (bFallingLastFrame || // in last frame the player is falling
                                     // in the current frame the player may or may not be on the ground
            IsFalling()) // in the current frame the player is falling as well
        {
            const FVector AccelDirection = AccelerationCached.GetSafeNormal2D();

            const float SpeedProjection = Velocity.X * AccelDirection.X + Velocity.Y * AccelDirection.Y;

            const float AddSpeed = MaxSpeed - SpeedProjection;
            if (AddSpeed > 0.0f)
            {
                float AnotherAddSpeedCandidate = AccelerationCached.Size() * AirAccelerationMultiplier * DeltaTime;

                if (AnotherAddSpeedCandidate > AddSpeed)
                {
                    AnotherAddSpeedCandidate = AddSpeed;
                }

                // if the player keeps pressing the jump button to strafe jump,
                // as a punishment, the acceleration is reduced
                if (bHasJumpPressed && !bHasJumpReleased)
                {
                    AnotherAddSpeedCandidate *= PenaltyScaleFactorForHoldingJumpButton;
                }

                // Apply acceleration
                FVector CurrentAcceleration = AnotherAddSpeedCandidate * AccelDirection;

                Velocity += CurrentAcceleration;
            }
        }
    }

    // todo: bZeroRequestedAcceleration always evaluates to true,
    // i.e. !bZeroRequestedAcceleration always evaluates to false
    // Apply additional requested acceleration
    if (!bZeroRequestedAcceleration)
    {
        Velocity += RequestedAcceleration * DeltaTime;
    }

    // impose final speed cap
    if (Velocity.Size2D() > SpeedUpperLimit)
    {
        Velocity = Velocity.GetClampedToMaxSize2D(SpeedUpperLimit);
    }

    if (bUseRVOAvoidance)
    {
        CalcAvoidanceVelocity(DeltaTime);
    }
}

//------------------------------------------------------------
//------------------------------------------------------------
void UExpesMovementComponent::QueueJump()
{
}

//------------------------------------------------------------
//------------------------------------------------------------
void UExpesMovementComponent::SetMovementStyle(EQLMovementStyle MyStyle)
{
    MovementStyle = MyStyle;
}

//------------------------------------------------------------
//------------------------------------------------------------
EQLMovementStyle UExpesMovementComponent::GetMovementStyle()
{
    return MovementStyle;
}