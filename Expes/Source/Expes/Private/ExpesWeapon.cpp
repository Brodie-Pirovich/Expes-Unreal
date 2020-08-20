// Fill out your copyright notice in the Description page of Project Settings.


#include "ExpesWeapon.h"
#include "Kismet/GameplayStatics.h"
#include "Expes/ExpesCharacter.h"
#include "Engine.h"
#include "DrawDebugHelpers.h"
#include "Expes/ExpesProjectile.h"
#include "ExpesRocketProjectile.h"
#include "ExpesPlayerController.h"

// Sets default values
AExpesWeapon::AExpesWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon Mesh"));

	MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	MuzzleLocation->SetupAttachment(WeaponMesh);
	MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

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
    if (CanFire(player))
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
        default:
            return;
            break;
        }
    }
}

void AExpesWeapon::FireSpread(FRotator Rotation, FVector SpawnLocation)
{
    FCollisionQueryParams TraceParams = FCollisionQueryParams(false);
    //TraceParams.bTraceAsyncScene = true;
    TraceParams.bReturnPhysicalMaterial = false;

    FHitResult Hit(ForceInit);
    FRotationMatrix RotMatrix(Rotation);

    for (int32 i = 0; i < shotsPerSpread; i++)
    {
        FVector Spread = FVector(0, FMath::RandRange(-maxSpread, maxSpread), FMath::RandRange(-maxSpread, maxSpread));
        Spread = RotMatrix.TransformVector(Spread);
        auto End = Rotation.Vector() * Range + SpawnLocation + Spread;
        GetWorld()->LineTraceSingleByChannel(Hit, SpawnLocation, End, ECC_Visibility, TraceParams);

        if (Hit.bBlockingHit)
        {
            DrawDebugLine(GetWorld(), SpawnLocation, Hit.Location, FColor::Green, false, 1, 0, 1);
            DrawDebugSolidBox(GetWorld(), FVector(Hit.Location), FVector(5, 5, 5), FRotationMatrix::MakeFromX(Hit.ImpactNormal).ToQuat(), FColor::Green, false, 1, 0);
        }
        else
        {
            DrawDebugLine(GetWorld(), SpawnLocation, End, FColor::Red, false, 1, 0, 1);
        }
    }
}

void AExpesWeapon::FireShotgun(AExpesCharacter* Player)
{
    FCollisionQueryParams TraceParams(FName(TEXT("lineTrace")),
        true, // bTraceComplex
        this); // ignore actor
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
        GetWorld()->LineTraceSingleByChannel(hitResult, start, end, ECC_Visibility, TraceParams);

        if (hitResult.bBlockingHit)
        {
            DrawDebugLine(GetWorld(), start, hitResult.Location, FColor::Green, false, 1, 0, 1);
            DrawDebugSolidBox(GetWorld(), FVector(hitResult.Location), FVector(5, 5, 5), FRotationMatrix::MakeFromX(hitResult.ImpactNormal).ToQuat(), FColor::Green, false, 1, 0);
        }
        else
        {
            DrawDebugLine(GetWorld(), start, end, FColor::Red, false, 1, 0, 1);
        }
    }
}

void AExpesWeapon::EndFire()
{
   
}

void AExpesWeapon::ShotgunFire(class AExpesCharacter* Player)
{
    const FVector SpawnLocation = MuzzleLocation->GetComponentLocation();
    //FRotator Rotation = Player->FirstPersonCameraComponent->GetComponentRotation();

    if (CanFire(Player))
    {
        FireShotgun(Player);
        ConsumeAmmo(Player);
        RemainingReloadTime = ReloadTime;

        if (Player->GetAmmo(AmmoType) <= 0)
        {
            return;
        }
    }
}

void AExpesWeapon::SuperShotgunFire(class AExpesCharacter* Player)
{
    const FVector SpawnLocation = MuzzleLocation->GetComponentLocation();
    FRotator Rotation = Player->FirstPersonCameraComponent->GetComponentRotation();

    if (CanFire(Player))
    {
        FireShotgun(Player);
        FireShotgun(Player);
        ConsumeAmmo(Player);
        ConsumeAmmo(Player);

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
    FVector Start = Player->FirstPersonCameraComponent->GetComponentLocation();
    FRotator Rotation = Player->FirstPersonCameraComponent->GetComponentRotation();

    if (CanFire(Player))
    {
        // create the transient beam actor
        //UParticleSystemComponent* BeamComponentTemp = nullptr;
        //AExpesRailBeam* RailBeamTemp = nullptr;

        /*if (RailBeamClass)
        {
            // AQLRailBeam object is automatically destroyed after the particle effect ends
            // because AQLRailBeam lifespan is specified in its BeginPlay()
            RailBeamTemp = GetWorld()->SpawnActor<AExpesRailBeam>(RailBeamClass, SpawnLocation, FRotator::ZeroRotator);
            if (RailBeamTemp)
            {
                RailBeamTemp->SetActorEnableCollision(false);
                BeamComponentTemp = RailBeamTemp->GetBeamComponent();
                if (BeamComponentTemp)
                {
                    BeamComponentTemp->SetBeamSourcePoint(0, SpawnLocation, 0);
                }
            }
        }*/

        FCollisionQueryParams TraceParams = FCollisionQueryParams(false);
        TraceParams.bReturnPhysicalMaterial = false;

        FHitResult Hit(ForceInit);
        FRotationMatrix RotMatrix(Rotation);

        FVector End = Player->FirstPersonCameraComponent->GetForwardVector() * Range + Start;
        GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, TraceParams);

        if (Hit.bBlockingHit)
        {
            DrawDebugLine(GetWorld(), Start, Hit.Location, FColor::Green, false, 1, 0, 1);
            DrawDebugSolidBox(GetWorld(), FVector(Hit.Location), FVector(5, 5, 5), FRotationMatrix::MakeFromX(Hit.ImpactNormal).ToQuat(), FColor::Green, false, 1, 0);

            FVector TargetLocation = Start + Player->GetFirstPersonCameraComponent()->GetForwardVector() * Range;

            /*if (BeamComponentTemp)
            {
                BeamComponentTemp->SetBeamTargetPoint(0, TargetLocation, 0);
            }*/

        }
        else
        {
            DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 1, 0, 1);
            //BeamComponentTemp->SetBeamTargetPoint(0, Hit.ImpactPoint, 0);
        }

        ConsumeAmmo(Player);
        RemainingReloadTime = ReloadTime;

        if (Player->GetAmmo(AmmoType) <= 0)
        {
            return;
        }
    }
}

void AExpesWeapon::RocketLauncherFire(class AExpesCharacter* Player)
{
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
            //AExpesRocketProjectile* Rocket = GetWorld()->SpawnActor<AExpesRocketProjectile>(RocketProjectileClass, MyTransform);

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
        UE_LOG(LogTemp, Warning, TEXT("Has player"));

        if (ExpesPlayerController)
        {
            ExpesPlayerController->ClientPlayCameraShake(FireCamShake);
        }

        //ServerPlayFireSound();
    }
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

void AExpesWeapon::ServerPlayFireSound_Implementation()
{
    UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
}

bool AExpesWeapon::ServerPlayFireSound_Validate()
{
    return true;
}

