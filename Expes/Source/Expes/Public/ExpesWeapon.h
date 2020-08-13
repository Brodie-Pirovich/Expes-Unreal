// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TimelineComponent.h"
#include "ExpesWeapon.generated.h"

class AExpesCharacter;
class AExpesProjectile;
class AExpesRocketProjectile;
class UAnimSequence;

UENUM(BlueprintType)
enum class EAmmoType : uint8
{
	ENone       UMETA(DisplayName = "None"),
	EShell      UMETA(DisplayName = "Shells"),
	EBullet       UMETA(DisplayName = "Bullets"),
	ECell       UMETA(DisplayName = "Cells"),
	ERocket       UMETA(DisplayName = "Rocket"),
	ESlug      UMETA(DisplayName = "Slug")
};

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	ENone       UMETA(DisplayName = "None"),
	EShotgun      UMETA(DisplayName = "Shotgun"),
	ESShotgun       UMETA(DisplayName = "Supershotgun"),
	ECell       UMETA(DisplayName = "Firegun"),
	ERocket       UMETA(DisplayName = "RocketLauncher"),
	ERail      UMETA(DisplayName = "Railgun")
};

UCLASS()
class EXPES_API AExpesWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AExpesWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void Fire(class AExpesCharacter* Player);

	virtual void FireSpread(FRotator Rotation, FVector MuzzleLocation);

	virtual void FireShotgun(class AExpesCharacter* Player);

	virtual void EndFire();

	virtual void ShotgunFire(class AExpesCharacter* Player);

	virtual void SuperShotgunFire(class AExpesCharacter* Player);

	virtual void RailgunFire(class AExpesCharacter* Player);

	virtual void RocketLauncherFire(class AExpesCharacter* Player);

	virtual void PlayFireEffects(class AExpesCharacter* Player);

protected:
	/** Returns whether or not the weapon is able to fire */
	bool CanFire(class AExpesCharacter* Player);

	/** Decrements weapon ammo */
	void ConsumeAmmo(class AExpesCharacter* Player);

	UPROPERTY(EditDefaultsOnly, Category = Weapon)
	TSubclassOf<UCameraShake> FireCamShake;

public:
	/** The damage per round */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Ammo)
	int32 Damage;

	/** The range of the projectiles, if applicable */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Ammo)
	int32 Range;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Ammo)
	int32 shotsPerSpread;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon)
	int32 maxSpread;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon)
	int32 maxSpreadHorizontal;

	/** The time between shots */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Weapon)
	float ReloadTime;

	/** The type of ammo the weapon consumes */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Weapon)
	EAmmoType AmmoType;

	/** The type of ammo the weapon consumes */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Weapon)
	EWeaponType WeaponType;

	/** The type of ammo the weapon consumes */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Weapon)
	int SlotID;

	/** The skeletal mesh representing the weapon */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* WeaponMesh;

	/** Location on gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USceneComponent* MuzzleLocation;

	/** The sound to play when the weapon fires */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Audio)
	class USoundBase* FireSound;

	/** The projectile class of the weapon, if applicable */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Ammo)
	TSubclassOf<AExpesProjectile> Projectile;

	/** The default amount of ammo the weapon holds */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Ammo)
	int32 DefaultAmmo;

	/** Plays the weapon fire sound */
	UFUNCTION(NetMulticast, Unreliable, WithValidation)
	void ServerPlayFireSound();
	void ServerPlayFireSound_Implementation();
	bool ServerPlayFireSound_Validate();

protected:
	/** The time until the weapon can fire again */
	float RemainingReloadTime;

	UPROPERTY(EditDefaultsOnly, Category = "C++Property")
	TSubclassOf<AExpesRocketProjectile> RocketProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++Property")
	float ProjectileSpeed;
};
