// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/TimelineComponent.h"
#include "ExpesMovementComponent.h"
#include "ExpesMovementStyle.h"
#include "ExpesWeapon.h"
#include "ExpesCharacter.generated.h"

class UInputComponent;
class UExpesMovementParameter;
class AExpesPlayerController;
class ExpesMovementStyle;
class UCameraComponent;
class AExpesWeapon;

UCLASS(config=Game)
class AExpesCharacter : public ACharacter
{
	GENERATED_BODY()

public:
    // Sets default values for this character's properties
    AExpesCharacter(const class FObjectInitializer& ObjectInitializer);

    // Called every frame
    // virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    FHitResult RayTraceFromCharacterPOV(float rayTraceRange = 1e5f);

    // Returns FirstPersonMesh subobject
    UFUNCTION(BlueprintCallable, Category = "C++Function")
    USkeletalMeshComponent* GetFirstPersonMesh();

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    USkeletalMeshComponent* GetThirdPersonMesh();

    // Returns FirstPersonCameraComponent subobject
    UFUNCTION(BlueprintCallable, Category = "C++Function")
    UCameraComponent* GetFirstPersonCameraComponent() const;

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    void SetHealthArmorBarVisible(bool bFlag);

    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

    UFUNCTION()
    void TakeDamageQuakeStyle(float ActualDamage);

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    void UpdateHealth();

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    void UpdateArmor();

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    AExpesPlayerController* GetQLPlayerController();

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    void SetCurrentWeapon(const FName& QLName);

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    void SetCurrentMovementStyle(EQLMovementStyle MyStyle);

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    void Die();

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    void OnDie();

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    void OnRespawnNewCharacter();

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    bool IsAlive();

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    float GetHealth() const;

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    void AddHealth(float Increment);

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    float GetMaxHealth() const;

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    float GetArmor() const;

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    void AddArmor(float Increment);

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    float GetMaxArmor() const;

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    void SetDamageMultiplier(const float Value);

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    void SetProtectionMultiplier(const float Value);

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    void StartGlow();

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    void StopGlow();

    //------------------------------------------------------------
    // Enable or disable weapon switch, fire and alt fire
    //------------------------------------------------------------
    UFUNCTION(BlueprintCallable, Category = "C++Function")
    void SetWeaponEnabled(const bool bFlag);

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    virtual UAnimSequence* PlayAnimationSequence(const FName& AnimationSequenceName);

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    bool HasWeapon(const FName& WeaponName);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++Property")
    float Health;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++Property")
    float MaxHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++Property")
    float Armor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++Property")
    float MaxArmor;

    //------------------------------------------------------------
    // Differentiate human player from AI bots
    //------------------------------------------------------------
    UFUNCTION(BlueprintCallable, Category = "C++Function")
    bool GetIsBot();

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    void SetIsBot(bool bFlag);

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    void SetMaxWalkSpeed(const float MaxWalkSpeed);

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    void ResetMaxWalkSpeed();

    // Fires a projectile.
    UFUNCTION(BlueprintCallable, Category = "C++Function")
    void OnFire();

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    void OnFireRelease();

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    void OnAltFire();

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    void OnAltFireRelease();

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    bool QLGetVisibility();

    //------------------------------------------------------------
    // Enable or disable the visibility of the first person mesh,
    // third person mesh, and weapon mesh
    //------------------------------------------------------------
    UFUNCTION(BlueprintCallable, Category = "C++Function")
    void QLSetVisibility(const bool bFlag);

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    bool QLGetVulnerability();

    //------------------------------------------------------------
    // Enable or disable the visibility of the first person mesh,
    // third person mesh, and weapon mesh
    //------------------------------------------------------------
    UFUNCTION(BlueprintCallable, Category = "C++Function")
    void QLSetVulnerability(const bool bFlag);

    virtual void Jump() override;

    virtual void StopJumping() override;

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    bool IsJumpButtonDown();

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    void EquipAll();

    virtual void FellOutOfWorld(const UDamageType& dmgType) override;

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    void OnDebug();

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    void PlaySoundFireAndForget(const FName& SoundName);

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    void PlaySound(const FName& SoundName);

    virtual void AddControllerYawInput(float Val) override;

    virtual void AddControllerPitchInput(float Val) override;

    // First person camera
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++Property", meta = (AllowPrivateAccess = "true"))
    UCameraComponent* FirstPersonCameraComponent;

protected:

    // Pawn mesh : 1st person view(arms; seen only by self)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++Property")
    USkeletalMeshComponent* FirstPersonMesh;

    UPROPERTY()
    TWeakObjectPtr<UMaterialInstanceDynamic> DynamicMaterialFirstPersonMesh;

    // Pawn mesh: 1st person view (arms; seen only by self)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++Property")
    USkeletalMeshComponent* ThirdPersonMesh;

    UPROPERTY()
    TWeakObjectPtr<UMaterialInstanceDynamic> DynamicMaterialThirdPersonMesh;

    // Base turn rate, in deg/sec. Other scaling may affect final turn rate.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++Property")
    float BaseTurnRate;

    // Base look up/down rate, in deg/sec. Other scaling may affect final rate.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++Property")
    float BaseLookUpRate;

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    virtual void PostInitializeComponents() override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Handles moving forward/backward
    void MoveForward(float Val);

    // Handles strafing movement, left and right
    void MoveRight(float Val);

    // Called via input to turn at a given rate.
    // This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
    void TurnAtRate(float Rate);

    // Called via input to turn look up/down at a given rate.
    // This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
    void LookUpAtRate(float Rate);

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    void SwitchToRocketLauncher();

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    void SwitchToLightningGun();

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    void SwitchToRailGun();

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    void SwitchToPortalGun();

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    void SwitchToGrenadeLauncher();

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    void SwitchToNailGun();

    // Fires a projectile.
    UFUNCTION(BlueprintCallable, Category = "C++Function")
    void OnUseAbility();

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    void StopSound();

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    void RespawnCharacterRandomly();
protected:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++Property")
    float ProtectionMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++Property")
    TMap<FName, UAnimMontage*> AnimationMontageList;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++Property")
    TMap<FName, UAnimSequence*> AnimationSequenceList;

    FTimerHandle DieTimerHandle;

    FTimerHandle RespawnTimerHandle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++Property")
    UAudioComponent* SoundComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++Property")
    TMap<FName, USoundBase*> SoundList;

    UPROPERTY()
    bool bCanFireAndAltFire;

    UPROPERTY()
    bool bCanSwitchWeapon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++Property")
    bool bQLIsVisible;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++Property")
    bool bQLIsVulnerable;

    // monitor jump status for animation purpose
    UPROPERTY()
    bool bJumpButtonDown;

    UPROPERTY()
    bool bQLIsBot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++Property")
    float DurationAfterDeathBeforeDestroyed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++Property")
    float DurationAfterDeathBeforeRespawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++Property")
    TSubclassOf<UExpesMovementParameter> MovementParameterQuakeClass;

    UPROPERTY()
    UExpesMovementParameter* MovementParameterQuake;

    ///////////////
    // Weapon Stuff
    public:
        /** Returns the ammo count of the specified ammo type */
        UFUNCTION(BlueprintCallable, Category = Ammo)
        int32 GetAmmo(EAmmoType ammoType);

        /** Returns the ammo count of the currently equipped weapon */
        UFUNCTION(BlueprintCallable, Category = Ammo)
        int32 GetCurrentWeaponAmmo();

        /** Returns the maximum ammo of the currently equipped weapon */
        UFUNCTION(BlueprintCallable, Category = Ammo)
        int32 GetCurrentWeaponMaxAmmo();

        /** Sets the ammo count of the specified ammo type to the specified value, clamping between 0 and max */
        void SetAmmo(EAmmoType AmmoType, int32 Value);

        /** Handles overlapping with a WeaponPickup class */
        bool PickupWeapon(TSubclassOf<AExpesWeapon> WeaponClass);

        /** Handles overlapping with an ArmourPickup class */
        //bool PickupArmour(int32 Armour, EArmourType ArmourType);

        /** The starting inventory of the player */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = Inventory)
        TArray<TSubclassOf<class AExpesWeapon>> DefaultInventory;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Inventory)
        /** The index of the default weapon. Should be 1 for the Shotgun */
        int32 DefaultWeaponIndex;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = Ammo)
        /** The number of Shells the player has */
        int32 Shells;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Ammo)
        /** The maximum number of Shells the player can carry */
        int32 MaxShells;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = Ammo)
        /** The number of Nails the player has */
        int32 Bullets;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Ammo)
        /** The maximum number of Nails the player can carry */
        int32 MaxBullets;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = Ammo)
        /** The number of Cells the player has */
        int32 Cells;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = Ammo)
        /** The maximum number of Cells the player can carry */
        int32 MaxCells;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = Ammo)
        /** The number of Cells the player has */
        int32 Rockets;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = Ammo)
        /** The maximum number of Cells the player can carry */
        int32 MaxRockets;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = Ammo)
        /** The number of Slugs the player has */
         int32 Slugs;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = Ammo)
        /** The maximum number of Slugs the player can carry */
        int32 MaxSlugs;

        /** The current inventory of the player */
        UPROPERTY(VisibleAnywhere, Replicated, Category = Inventory)
        TArray<AExpesWeapon*> WeaponInventory;

        private:
            /** The currently equipped weapon */
            UPROPERTY(Replicated)
            class AExpesWeapon* CurrentWeapon;

            /** The index of the currently equipped weapon */
            int32 WeaponIndex;

            bool WasFiring = false;

            /** Creates an instance of the specified weapon class and adds it to the inventory */
            void AddWeapon(TSubclassOf<AExpesWeapon> WeaponClass);

    protected:
        /** Creates the default inventory */
        void CreateInventory();
        UFUNCTION(Server, Reliable, WithValidation)
        void ServerCreateInventory();
        void ServerCreateInventory_Implementation();
        bool ServerCreateInventory_Validate();

        /** Switches to the next weapon in the inventory */
        void NextWeapon();
        UFUNCTION(Server, Reliable, WithValidation)
        void ServerNextWeapon();
        void ServerNextWeapon_Implementation();
        bool ServerNextWeapon_Validate();

        /** Switches to the previous weapon in the inventory */
        void PrevWeapon();
        UFUNCTION(Server, Reliable, WithValidation)
        void ServerPrevWeapon();
        void ServerPrevWeapon_Implementation();
        bool ServerPrevWeapon_Validate();

        void FireHeld(float Val);
        UFUNCTION(Server, Reliable, WithValidation)
        void ServerFireHeld(float Val);
        void ServerFireHeld_Implementation(float Val);
        bool ServerFireHeld_Validate(float Val);

        /** Returns the maximum ammo capacity of the specified ammo type */
        int32 GetMaxAmmo(EAmmoType AmmoType);

        void UpdateCurrentWeapon();

    public:
        /** Returns FirstPersonCameraComponent subobject **/
        FORCEINLINE class UCameraComponent* GetFPCameraComponent() const { return FirstPersonCameraComponent; }

        /** Returns Mesh1P subobject **/
        FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return FirstPersonMesh; }
};

