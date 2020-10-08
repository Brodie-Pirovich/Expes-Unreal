// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BasePickup.h"
#include "HealthPickup.generated.h"

/**
 * 
 */
UCLASS()
class EXPES_API AHealthPickup : public ABasePickup
{
	GENERATED_BODY()

public:
    AHealthPickup();

protected:
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Health)
    float HealthAmount;

    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
