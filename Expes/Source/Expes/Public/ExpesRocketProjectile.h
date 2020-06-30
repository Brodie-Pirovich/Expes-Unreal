// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Expes/ExpesProjectile.h"
#include "ExpesRocketProjectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
class AExpesPlayerController;
/**
 * 
 */
UCLASS()
class EXPES_API AExpesRocketProjectile : public AExpesProjectile
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	AExpesRocketProjectile();

protected:
	virtual void PostInitializeComponents() override;
};
