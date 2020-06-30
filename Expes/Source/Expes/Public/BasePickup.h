// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "Components/BoxComponent.h"
#include "BasePickup.generated.h"

UCLASS()
class EXPES_API ABasePickup : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABasePickup();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

    /** Makes the pickup rotate */
    UPROPERTY(VisibleDefaultsOnly, Category = Movement)
    class URotatingMovementComponent* RotatingMovementComponent;

    /** The trigger for the pickup */
    UPROPERTY(VisibleDefaultsOnly, Category = Collision)
    class UBoxComponent* BoxCollision;

    /** The sound to play on pickup */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Audio)
    class USoundBase* PickupSound;

    UFUNCTION()
    virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
