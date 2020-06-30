// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BasePickup.h"
#include "BaseWeaponPickup.generated.h"

/**
 * 
 */
UCLASS()
class EXPES_API ABaseWeaponPickup : public ABasePickup
{
	GENERATED_BODY()
	
public:
    ABaseWeaponPickup();

protected:
    /** The weapon class the pickup represents */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Weapon)
        TSubclassOf<class AExpesWeapon> Weapon;

    /** The skeletal mesh that represents the weapon */
    UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
        class USkeletalMeshComponent* WeaponMesh;

    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
