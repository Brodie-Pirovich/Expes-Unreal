// Fill out your copyright notice in the Description page of Project Settings.


#include "RailBeam.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/SphereComponent.h"

// Sets default values
ARailBeam::ARailBeam()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    auto* RootSphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("RootSphereComponent"));
    RootSphereComponent->SetCollisionProfileName("NoCollision");
    RootComponent = RootSphereComponent;

    BeamComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("BeamComponent"));
    BeamComponent->SetupAttachment(RootComponent);
    BeamComponent->SetCollisionProfileName("NoCollision");
}

// Called when the game starts or when spawned
void ARailBeam::BeginPlay()
{
	Super::BeginPlay();

    SetLifeSpan(1.0f);
}

// Called every frame
void ARailBeam::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

UParticleSystemComponent* ARailBeam::GetBeamComponent()
{
    return BeamComponent;
}

//------------------------------------------------------------
//------------------------------------------------------------
void ARailBeam::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}

