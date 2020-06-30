// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ExpesPlayerController.generated.h"

class AExpesCharacter;
/**
 * 
 */
UCLASS()
class EXPES_API AExpesPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
    AExpesPlayerController();

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    float GetFrameRate() const;

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    float GetControlledPawnSpeed() const;

    virtual void Tick(float DeltaSeconds) override;

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    void AddUMG();

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    void ShowDamageOnScreen(float DamageAmount, const FVector& WorldTextLocation);

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    void ShowAbilityMenu();

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    void OnRestartLevel();

protected:
    //------------------------------------------------------------
    //------------------------------------------------------------
    virtual void PostInitializeComponents() override;

    //------------------------------------------------------------
    //------------------------------------------------------------
    virtual void BeginPlay() override;

    //------------------------------------------------------------
    //------------------------------------------------------------
    virtual void OnPossess(APawn* ControlledPawn) override;

    //------------------------------------------------------------
    //------------------------------------------------------------
    virtual void SetupInputComponent() override;

    float FPS;
};
