// Fill out your copyright notice in the Description page of Project Settings.


#include "ExpesPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Expes/ExpesCharacter.h"

//------------------------------------------------------------
//------------------------------------------------------------
AExpesPlayerController::AExpesPlayerController() :
    FPS(0.0f)
{

}

//------------------------------------------------------------
//------------------------------------------------------------
float AExpesPlayerController::GetFrameRate() const
{
    return FPS;
}

//------------------------------------------------------------
//------------------------------------------------------------
float AExpesPlayerController::GetControlledPawnSpeed() const
{
    APawn* ControlledPawn = GetPawn();
    if (ControlledPawn)
    {
        // !!! note that the pawn must have movement component in order to calculate velocity
        // !!! otherwise the velocity is always zero.
        // !!! todo: for super power mimic matter, the pawn does not use movement component by design
        // !!! how to obtain the velocity then?
        FVector temp = ControlledPawn->GetVelocity();

        // only consider x and y components of the velocity
        return temp.Size2D();
    }
    else
    {
        return 0.0f;
    }
}

//------------------------------------------------------------
//------------------------------------------------------------
void AExpesPlayerController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    FPS = 1.0f / DeltaSeconds;
}

//------------------------------------------------------------
//------------------------------------------------------------
void AExpesPlayerController::AddUMG()
{

}

//------------------------------------------------------------
//------------------------------------------------------------
void AExpesPlayerController::PostInitializeComponents()
{
    Super::PostInitializeComponents();
}

//------------------------------------------------------------
//------------------------------------------------------------
void AExpesPlayerController::BeginPlay()
{
    Super::BeginPlay();
}

//------------------------------------------------------------
//------------------------------------------------------------
void AExpesPlayerController::OnPossess(APawn* ControlledPawn)
{
    Super::OnPossess(ControlledPawn);

    AExpesCharacter* ControlledCharacter = Cast<AExpesCharacter>(ControlledPawn);
    if (ControlledCharacter)
    {
        // controlled character does not see his own health and armor bar
        //ControlledCharacter->SetHealthArmorBarVisible(false);

        AddUMG();
    }
}

//------------------------------------------------------------
//------------------------------------------------------------
void AExpesPlayerController::ShowDamageOnScreen(float DamageAmount, const FVector& WorldTextLocation)
{

}

//------------------------------------------------------------
//------------------------------------------------------------
void AExpesPlayerController::ShowAbilityMenu()
{

}

//------------------------------------------------------------
//------------------------------------------------------------
void AExpesPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

}

//------------------------------------------------------------
//------------------------------------------------------------
void AExpesPlayerController::OnRestartLevel()
{

}