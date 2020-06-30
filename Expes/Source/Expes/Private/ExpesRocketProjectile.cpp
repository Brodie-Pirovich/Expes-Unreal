// Fill out your copyright notice in the Description page of Project Settings.


#include "ExpesRocketProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Expes/ExpesCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "ExpesPlayerController.h"

//------------------------------------------------------------
// Sets default values
//------------------------------------------------------------
AExpesRocketProjectile::AExpesRocketProjectile()
{
    RootSphereComponent->SetEnableGravity(false);

    ProjectileMovementComponent->ProjectileGravityScale = 0.0f;
    StaticMeshComponent->SetEnableGravity(false);

    ProjectileLifeSpan = 5.0f;
    BlastRadius = 400.0f;
    BlastSpeedChange = 1200.0f;
    BasicDamage = 100.0f;
    BasicDamageAdjusted = BasicDamage;
    BlastSpeedChangeSelfDamageScale = 1.25f;
    ExplosionParticleSystemScale = 4.0f;
}

//------------------------------------------------------------
//------------------------------------------------------------
void AExpesRocketProjectile::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    RootSphereComponent->OnComponentBeginOverlap.RemoveDynamic(this, &AExpesRocketProjectile::OnBeginOverlapForComponent);
    RootSphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AExpesRocketProjectile::OnBeginOverlapForComponent);
}
