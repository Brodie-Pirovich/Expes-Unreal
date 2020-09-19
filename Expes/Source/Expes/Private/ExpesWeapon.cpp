// Fill out your copyright notice in the Description page of Project Settings.


#include "ExpesWeapon.h"
#include "Kismet/GameplayStatics.h"
#include "Expes/ExpesCharacter.h"
#include "Engine.h"
#include "DrawDebugHelpers.h"
#include "Expes/ExpesProjectile.h"
#include "ExpesRocketProjectile.h"
#include "ExpesPlayerController.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values
AExpesWeapon::AExpesWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon Mesh"));

	MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	MuzzleLocation->SetupAttachment(WeaponMesh);
	MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

    MuzzleSocketName = "Muzzle";
    TracerTargetName = "Target";

	bReplicates = true;
}

// Called when the game starts or when spawned
void AExpesWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AExpesWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (RemainingReloadTime > 0)
	{
		RemainingReloadTime -= DeltaTime;
	}
}

void AExpesWeapon::Fire(class AExpesCharacter* player)
{
    if (b_IsAI)
    {
        AIFire(player);
    }
    else if (CanFire(player))
    {
        switch (WeaponType)
        {
        case EWeaponType::ENone:
            return;
            break;
        case EWeaponType::EShotgun:
            ShotgunFire(player);
            break;
        case EWeaponType::ESShotgun:
            SuperShotgunFire(player);
            break;
        case EWeaponType::ERocket:
            RocketLauncherFire(player);
            break;
        case EWeaponType::ERail:
            RailgunFire(player);
            break;
        case EWeaponType::EAIGun:
            RailgunFire(player);
            break;
        default:
            return;
            break;
        }
    }
}

void AExpesWeapon::AIFire(AExpesCharacter* Player)
{
    PlayAnimationMontage(Player);
    Player->PlaySoundFireAndForget("Shotgunfire");

    FVector Start = Player->FirstPersonCameraComponent->GetComponentLocation();
    FRotator Rotation = Player->FirstPersonCameraComponent->GetComponentRotation();

    FHitResult Hit(ForceInit);
    FRotationMatrix RotMatrix(Rotation);

    FVector End = Player->FirstPersonCameraComponent->GetForwardVector() * Range + Start;

    FCollisionQueryParams params(FName(TEXT("lineTrace")),
        true, // bTraceComplex
        Player); // ignore actor
    params.bReturnPhysicalMaterial = false;

    GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_Camera, params);

    if (Hit.bBlockingHit)
    {
        DrawDebugLine(GetWorld(), Start, Hit.Location, FColor::Green, false, 1, 0, 1);
        DrawDebugSolidBox(GetWorld(), FVector(Hit.Location), FVector(5, 5, 5), FRotationMatrix::MakeFromX(Hit.ImpactNormal).ToQuat(), FColor::Green, false, 1, 0);

        FVector TargetLocation = Start + Player->GetFirstPersonCameraComponent()->GetForwardVector() * Range;

        PlayImpactEffects(Hit.ImpactPoint);
        TargetLocation = Hit.ImpactPoint;
        PlayFireEffects(TargetLocation);

        HandleDamage(Player, Hit);
    }
    else
    {
        FVector TargetLocation = Start + Player->GetFirstPersonCameraComponent()->GetForwardVector() * Range;
        DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 1, 0, 1);
        PlayFireEffects(TargetLocation);
    }
}

void AExpesWeapon::FireShotgun(AExpesCharacter* Player)
{
    PlayAnimationMontage(Player);

    FCollisionQueryParams TraceParams(FName(TEXT("lineTrace")),
        true, // bTraceComplex
        Player); // ignore actor
    TraceParams.bReturnPhysicalMaterial = false;

    FVector start = Player->FirstPersonCameraComponent->GetComponentLocation();
    FRotator Rotation = Player->FirstPersonCameraComponent->GetComponentRotation();

    FHitResult hitResult(ForceInit);
    FRotationMatrix RotMatrix(Rotation);

    for (int32 i = 0; i < shotsPerSpread; i++)
    {
        //FVector Spread = FVector(0, FMath::RandRange(-maxSpread, maxSpread), FMath::RandRange(-maxSpread, maxSpread));
        FVector Spread = FVector(0, FMath::RandRange(-maxSpreadHorizontal, maxSpreadHorizontal), FMath::RandRange(-maxSpread, maxSpread));
        Spread = RotMatrix.TransformVector(Spread);
        FVector end = Player->FirstPersonCameraComponent->GetForwardVector() * Range + start + Spread;
        FVector TracerEndPoint = end;
        GetWorld()->LineTraceSingleByChannel(hitResult, start, end, ECollisionChannel::ECC_Camera, TraceParams);
        if (hitResult.bBlockingHit)
        {
            //DrawDebugLine(GetWorld(), start, hitResult.Location, FColor::Green, false, 1, 0, 1);
            //DrawDebugSolidBox(GetWorld(), FVector(hitResult.Location), FVector(5, 5, 5), FRotationMatrix::MakeFromX(hitResult.ImpactNormal).ToQuat(), FColor::Green, false, 1, 0);

            PlayImpactEffects(hitResult.ImpactPoint);

            TracerEndPoint = hitResult.ImpactPoint;

            HandleDamage(Player, hitResult);
        }
        else
        {
            //DrawDebugLine(GetWorld(), start, end, FColor::Red, false, 1, 0, 1);
        }
        PlayFireEffects(TracerEndPoint);
    }
}

void AExpesWeapon::ShotgunFire(class AExpesCharacter* Player)
{
    if (CanFire(Player))
    {
        FireShotgun(Player);
        ConsumeAmmo(Player);
        Player->PlaySoundFireAndForget("Shotgunfire");
        RemainingReloadTime = ReloadTime;

        if (Player->GetAmmo(AmmoType) <= 0)
        {
            return;
        }
    }
    PlayFireEffects(Player);
}

void AExpesWeapon::SuperShotgunFire(class AExpesCharacter* Player)
{
    if (CanFire(Player))
    {
        FireShotgun(Player);
        FireShotgun(Player);
        ConsumeAmmo(Player);
        ConsumeAmmo(Player);
        Player->PlaySoundFireAndForget("SuperShotgunfire");

        RemainingReloadTime = ReloadTime;

        if (Player->GetAmmo(AmmoType) <= 0)
        {
            return;
        }
    }
    PlayFireEffects(Player);
}

void AExpesWeapon::RailgunFire(class AExpesCharacter* Player)
{
    PlayAnimationMontage(Player);
    PlayFireEffects(Player);

    FVector Start = Player->FirstPersonCameraComponent->GetComponentLocation();
    FRotator Rotation = Player->FirstPersonCameraComponent->GetComponentRotation();

    if (CanFire(Player))
    {
        FCollisionQueryParams TraceParams(FName(TEXT("lineTrace")),
            true, // bTraceComplex
            Player); // ignore actor
        TraceParams.bReturnPhysicalMaterial = false;

        FHitResult Hit(ForceInit);
        FRotationMatrix RotMatrix(Rotation);

        FVector End = Player->FirstPersonCameraComponent->GetForwardVector() * Range + Start;
        GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_Camera, TraceParams);

        if (Hit.bBlockingHit)
        {
           // DrawDebugLine(GetWorld(), Start, Hit.Location, FColor::Green, false, 1, 0, 1);
            //DrawDebugSolidBox(GetWorld(), FVector(Hit.Location), FVector(5, 5, 5), FRotationMatrix::MakeFromX(Hit.ImpactNormal).ToQuat(), FColor::Green, false, 1, 0);

            FVector TargetLocation = Start + Player->GetFirstPersonCameraComponent()->GetForwardVector() * Range;

            HandleDamage(Player, Hit);
            PlayImpactEffects(Hit.ImpactPoint);
            TargetLocation = Hit.ImpactPoint;
            PlayFireEffects(TargetLocation);
        }
        else
        {
            FVector TargetLocation = Start + Player->GetFirstPersonCameraComponent()->GetForwardVector() * Range;
            //DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 1, 0, 1);
            PlayFireEffects(TargetLocation);
        }

        ConsumeAmmo(Player);
        Player->PlaySoundFireAndForget("RailgunFire");
        RemainingReloadTime = ReloadTime;

        if (Player->GetAmmo(AmmoType) <= 0)
        {
            return;
        }
    }
}

void AExpesWeapon::RocketLauncherFire(class AExpesCharacter* Player)
{
    PlayAnimationMontage(Player);

    FHitResult HitResult = Player->RayTraceFromCharacterPOV(Range);

    // determine source and target
    UCameraComponent* CameraComponent = Player->GetFirstPersonCameraComponent();
    if (CameraComponent == nullptr)
    {
        return;
    }

    FVector SourceLocation = MuzzleLocation->GetComponentLocation() + CameraComponent->GetForwardVector() * 10.0f;
    FVector TargetLocation;

    // if hit occurs
    if (HitResult.bBlockingHit)
    {
        TargetLocation = HitResult.ImpactPoint;
    }
    else
    {
        TargetLocation = MuzzleLocation->GetComponentLocation() + CameraComponent->GetForwardVector() * Range;
    }

    FVector ProjectileForwardVector = TargetLocation - SourceLocation;
    ProjectileForwardVector.Normalize();

    FMatrix result = FRotationMatrix::MakeFromXZ(ProjectileForwardVector, CameraComponent->GetUpVector());
    FRotator SourceRotation = result.Rotator();

    // spawn and launch a rocket
    UWorld* const World = GetWorld();

    PlayFireEffects(TargetLocation);
    PlayFireEffects(Player);

    if (RocketProjectileClass && World)
    {
        if (!Player || !CameraComponent)
        {
            return;
        }

        if (CanFire(Player))
        {
            FTransform MyTransform(SourceRotation, SourceLocation, FVector(1.0f));
            AExpesRocketProjectile* Rocket = GetWorld()->SpawnActorDeferred<AExpesRocketProjectile>(RocketProjectileClass, MyTransform);
            ProjectileSpeed = 3000.0f;

            // pass controller to rocket as damage instigator
            AController* Controller = Player->GetController();
            AExpesPlayerController* ExpesPlayerController = Cast<AExpesPlayerController>(Controller);
            Rocket->QLSetPlayerController(ExpesPlayerController);
            Rocket->SetDamageMultiplier(1);
            UGameplayStatics::FinishSpawningActor(Rocket, MyTransform);

            // change velocity
            Rocket->GetProjectileMovementComponent()->MaxSpeed = ProjectileSpeed;
            Rocket->GetProjectileMovementComponent()->InitialSpeed = ProjectileSpeed;
            FVector FinalVelocity = ProjectileForwardVector * ProjectileSpeed;
            Rocket->GetProjectileMovementComponent()->Velocity = FinalVelocity;

            ConsumeAmmo(Player);
            Player->PlaySoundFireAndForget("RocketFire");
            RemainingReloadTime = ReloadTime;

            if (Player->GetAmmo(AmmoType) <= 0)
            {
                return;
            }
        }
    }
}

void AExpesWeapon::PlayFireEffects(class AExpesCharacter* Player)
{
    if (Player)
    {
        AController* Controller = Player->GetController();
        AExpesPlayerController* ExpesPlayerController = Cast<AExpesPlayerController>(Controller);

        if (ExpesPlayerController)
        {
            ExpesPlayerController->ClientPlayCameraShake(FireCamShake);
        }
    }
}

void AExpesWeapon::HandleDamage(class AExpesCharacter* Player, FHitResult Hit)
{
    // check the hit actor
    auto* hitActor = Cast<AExpesCharacter>(Hit.GetActor());
    if (!hitActor)
    {
        // do sth
        return;
    }

    // if self-hurt
    if (hitActor == Player)
    {
        // do sth
        return;
    }

    // create a damage event
    const FPointDamageEvent DamageEvent;

    float DamageAmount = hitActor->TakeDamage(Damage, DamageEvent, Player->GetController(), this);
}

void AExpesWeapon::ConsumeAmmo(class AExpesCharacter* player)
{
    player->SetAmmo(AmmoType, player->GetAmmo(AmmoType) - 1);
}

bool AExpesWeapon::CanFire(class AExpesCharacter* player)
{
    if (RemainingReloadTime > 0 || player->GetAmmo(AmmoType) <= 0)
    {
        return false;
    }

    return true;
}

void AExpesWeapon::PlayFireEffects(FVector TraceEnd)
{
    if (MuzzleEffect)
    {
        UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, WeaponMesh, MuzzleSocketName);
    }

    if (TracerEffect)
    {
        FVector WeaponMuzzleLocation = WeaponMesh->GetSocketLocation(MuzzleSocketName);

        UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, WeaponMuzzleLocation);
        if (TracerComp)
        {
            TracerComp->SetVectorParameter(TracerTargetName, TraceEnd);
        }
    }
}

void AExpesWeapon::PlayImpactEffects(FVector ImpactPoint)
{
    FVector WeaponMuzzleLocation = WeaponMesh->GetSocketLocation(MuzzleSocketName);

    FVector ShotDirection = ImpactPoint - WeaponMuzzleLocation;
    ShotDirection.Normalize();

    UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DefaultImpactEffect, ImpactPoint, ShotDirection.Rotation());
}

void AExpesWeapon::PlayAnimationMontage(class AExpesCharacter* Player)
{
    if (Player)
    {
        if (Player->bIsAI)
        {
            USkeletalMeshComponent* ArmMesh = Player->GetThirdPersonMesh();
            if (ArmMesh)
            {
                UAnimInstance* AnimInstance = ArmMesh->GetAnimInstance();
                if (AnimInstance)
                {
                    AnimInstance->Montage_Play(FireAnimation, 1.0f);
                }
            }
        }
        else
        { 
            USkeletalMeshComponent* ArmMesh = Player->GetFirstPersonMesh();
            if (ArmMesh)
            {
                UAnimInstance* AnimInstance = ArmMesh->GetAnimInstance();
                if (AnimInstance)
                {
                    AnimInstance->Montage_Play(FireAnimation, 1.0f);
                }
            }
        }

    }
}

