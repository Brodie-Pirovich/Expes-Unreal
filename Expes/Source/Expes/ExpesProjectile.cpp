// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "ExpesProjectile.h"
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
#include "DrawDebugHelpers.h"

AExpesProjectile::AExpesProjectile() 
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    RootSphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("RootSphereComponent"));
    RootSphereComponent->InitSphereRadius(20.0f);
    RootSphereComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    RootSphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    RootSphereComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
    RootComponent = RootSphereComponent;

    ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
    ProjectileMovementComponent->UpdatedComponent = RootSphereComponent;
    ProjectileMovementComponent->SetVelocityInLocalSpace(FVector(0.0f, 0.0f, 0.0f));

    StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
    StaticMeshComponent->SetupAttachment(RootComponent);
    StaticMeshComponent->SetCollisionProfileName(TEXT("NoCollision"));

    ExplosionParticleSystem = nullptr;
    ProjectileLifeSpan = 5.0f;
    BlastRadius = 400.0f;
    BlastSpeedChange = 1200.0f;
    BasicDamage = 100.0f;
    PlayerController = nullptr;
    DamageMultiplier = 1.0f;
    BasicDamageAdjusted = BasicDamage;
    BlastSpeedChangeSelfDamageScale = 1.25f;
    ExplosionParticleSystemScale = 1.0f;

    bIsSelfDirectHit = false;
    bIsDirectHit = false;
}

//------------------------------------------------------------
// Called when the game starts or when spawned
//------------------------------------------------------------
void AExpesProjectile::BeginPlay()
{
    Super::BeginPlay();

    SetLifeSpan(ProjectileLifeSpan);
}

//------------------------------------------------------------
// Called every frame
//------------------------------------------------------------
void AExpesProjectile::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

}

//------------------------------------------------------------
//------------------------------------------------------------
void AExpesProjectile::OnBeginOverlapForComponent(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // This function avoids self direct hit. Such case can happen, for example, when the player is
    // firing the projectile down, and the projectile overlaps with the player's vertical capsule immediately.
    // When such case happens, the function returns so as to ignore the current overlap event.
    // When such case does not happen, this function is guaranteed to be called only once,
    // because even though the projectile may overlap several components,
    // it is destroyed instantly upon the first overlap event.
   if (OtherActor)
    {
        HandleDirectHit(OtherActor, bIsSelfDirectHit, bIsDirectHit);
        if (bIsSelfDirectHit)
        {
            return;
        }

        HandleSplashHit(OtherActor, bIsDirectHit);

        //DrawDebugSphere(GetWorld(), GetActorLocation(), BlastRadius, 20, FColor::Red, false, 10, 0, 1);
        Destroy();
    }
}

//------------------------------------------------------------
//------------------------------------------------------------
void AExpesProjectile::HandleDirectHit(AActor* OtherActor, bool& bSelfDirectHit, bool& bDirectHit)
{
    bDirectHit = false;
    bSelfDirectHit = false;

    AExpesCharacter* Character = Cast<AExpesCharacter>(OtherActor);
    if (Character)
    {
        // prevent self direct hit from happening
        if (PlayerController.IsValid() && OtherActor == PlayerController->GetCharacter())
        {
            bSelfDirectHit = true;
            return;
        }

        float DamageAmount = BasicDamageAdjusted;
        const FPointDamageEvent DamageEvent;
        DamageAmount = Character->TakeDamage(DamageAmount, DamageEvent, PlayerController.Get(), this);

        bDirectHit = true;
    }
    else
    {
        float DamageAmount = BasicDamageAdjusted;
        const FPointDamageEvent DamageEvent;
        DamageAmount = OtherActor->TakeDamage(DamageAmount, DamageEvent, PlayerController.Get(), this);

        bDirectHit = true;
    }

    float DamageAmount = BasicDamageAdjusted;
    const FPointDamageEvent DamageEvent;
    DamageAmount = OtherActor->TakeDamage(DamageAmount, DamageEvent, PlayerController.Get(), this);

    bDirectHit = true;
}

//------------------------------------------------------------
//------------------------------------------------------------
void AExpesProjectile::HandleSplashHit(AActor* OtherActor, bool bDirectHit)
{
    // get victims within the blast radius
    FVector Epicenter = GetActorLocation();
    TArray<FOverlapResult> OutOverlaps;
    FCollisionObjectQueryParams CollisionObjectQueryParams(ECollisionChannel::ECC_Pawn);
    FCollisionQueryParams CollisionQueryParams;

    GetWorld()->OverlapMultiByObjectType(OutOverlaps,
        Epicenter,
        FQuat(GetActorRotation()),
        CollisionObjectQueryParams,
        FCollisionShape::MakeSphere(BlastRadius),
        CollisionQueryParams);

    // iterate victims
    for (auto&& Result : OutOverlaps)
    {
        AActor* HitActor = Result.GetActor();

        if (HitActor)
        {
            AExpesCharacter* Character = Cast<AExpesCharacter>(HitActor);
            if (Character)
            {
                SplashDamageVictimList.push_back(TWeakObjectPtr<AExpesCharacter>(Character));

                // change victim velocity
                UCharacterMovementComponent* CharacterMovementComponent = Character->GetCharacterMovement();
                if (CharacterMovementComponent)
                {
                    // a less good approach is LaunchCharacter()
                    // Character->LaunchCharacter(FVector(0.0f, 0.0f, 100.0f), true, true);

                    float ActualBlastSpeedChange = BlastSpeedChange;
                    if (PlayerController.IsValid() && Character == PlayerController->GetCharacter())
                    {
                        ActualBlastSpeedChange *= BlastSpeedChangeSelfDamageScale;
                    }

                    CharacterMovementComponent->AddRadialImpulse(
                        GetActorLocation(),
                        BlastRadius,
                        ActualBlastSpeedChange,
                        ERadialImpulseFalloff::RIF_Linear,
                        true); // velocity change (true) or impulse (false)
                }

                // inflict damage
                // if direct hit damage has already been applied, skip to the next victim
                if (bDirectHit && OtherActor == HitActor)
                {
                    continue;
                }

                // reduce self splash damage (e.g. rocket jump, nailgun jump)
                float DamageAmount = BasicDamageAdjusted;

                if (PlayerController.IsValid() && Character == PlayerController->GetCharacter())
                {
                    DamageAmount = ReduceSelfDamage(DamageAmount);
                }

                FRadialDamageEvent DamageEvent;
                DamageEvent.Params.BaseDamage = DamageAmount;
                DamageEvent.Params.OuterRadius = BlastRadius;
                DamageEvent.Params.MinimumDamage = 0.0;

                //DamageAmount = Character->TakeDamage(DamageAmount, DamageEvent, PlayerController.Get(), this);
                Character->TakeDamage(DamageAmount, DamageEvent, PlayerController.Get(), this);

            }
            else
            {
                // inflict damage
                // if direct hit damage has already been applied, skip to the next victim
                if (bDirectHit && OtherActor == HitActor)
                {
                    continue;
                }

                // reduce self splash damage (e.g. rocket jump, nailgun jump)
                float DamageAmount = BasicDamageAdjusted;

                FRadialDamageEvent DamageEvent;
                DamageEvent.Params.BaseDamage = DamageAmount;
                DamageEvent.Params.OuterRadius = BlastRadius;
                DamageEvent.Params.MinimumDamage = 0.0;

                DamageAmount = HitActor->TakeDamage(DamageAmount, DamageEvent, PlayerController.Get(), this);
            }
        }
    }
}

//------------------------------------------------------------
//------------------------------------------------------------
UProjectileMovementComponent* AExpesProjectile::GetProjectileMovementComponent()
{
    return ProjectileMovementComponent;
}

//------------------------------------------------------------
//------------------------------------------------------------
void AExpesProjectile::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    if (EndPlayReason == EEndPlayReason::Destroyed)
    {
        // play explosion particle system
        if (ExplosionParticleSystem)
        {
            FTransform Transform(FRotator::ZeroRotator,
                GetActorLocation(),
                FVector(ExplosionParticleSystemScale)); // scale

            UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),
                ExplosionParticleSystem,
                Transform,
                true, // auto destroy
                EPSCPoolMethod::AutoRelease);
        }

        PlaySoundFireAndForget(FName(TEXT("Explode")));
    }
}

//------------------------------------------------------------
//------------------------------------------------------------
void AExpesProjectile::QLSetPlayerController(AExpesPlayerController* PlayerControllerExt)
{
    PlayerController = PlayerControllerExt;
}

//------------------------------------------------------------
//------------------------------------------------------------
float AExpesProjectile::ReduceSelfDamage(const float InDamage)
{
    return InDamage * 0.3;
}

//------------------------------------------------------------
//------------------------------------------------------------
void AExpesProjectile::SetDamageMultiplier(const float Value)
{
    DamageMultiplier = Value;

    BasicDamageAdjusted = Value * BasicDamage;
}

//------------------------------------------------------------
//------------------------------------------------------------
void AExpesProjectile::PlaySoundFireAndForget(const FName& SoundName)
{
    USoundBase** Result = SoundList.Find(SoundName);
    if (Result)
    {
        /*USoundBase* Sound = *Result;
        if (Sound && SoundAttenuation)
        {
            UGameplayStatics::PlaySoundAtLocation(GetWorld(),
                Sound,
                GetActorLocation(),
                GetActorRotation(),
                1.0f, // VolumeMultiplier
                1.0f, // PitchMultiplier
                0.0f, // StartTime
                SoundAttenuation);
        }*/
    }
}

