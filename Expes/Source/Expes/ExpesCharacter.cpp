// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "ExpesCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Components/WidgetComponent.h"
#include "ExpesPlayerController.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/AudioComponent.h"
#include "NavigationSystem.h"
#include "ExpesMovementComponent.h"
#include "ExpesMovementParameter.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimMontage.h"

//////////////////////////////////////////////////////////////////////////
// AExpesCharacter

//------------------------------------------------------------
// Sets default values
//------------------------------------------------------------
AExpesCharacter::AExpesCharacter(const class FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer.SetDefaultSubobjectClass<UExpesMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	Health = 100.0f;
	MaxHealth = 200.0f;
	Armor = 100.0f;
	MaxArmor = 150.0f;
	ProtectionMultiplier = 1.0f;

	bCanFireAndAltFire = true;
	bCanSwitchWeapon = true;

	// Set size for collision capsule
	// original value: 55.f, 96.0f
	GetCapsuleComponent()->InitCapsuleSize(30.0f, 85.0f);

	// set our turn rates for input
	BaseTurnRate = 40.0f;
	BaseLookUpRate = 40.0f;

	// Create a CameraComponent
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCameraComponent"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	//FirstPersonCameraComponent->RelativeLocation = FVector(-39.56f, 1.75f, 64.f); // Position the camera
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.0f, 1.75f, 64.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true; // critical! If false, mouse would not change pitch!
	FirstPersonCameraComponent->SetFieldOfView(100.0f);

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonMesh"));
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->SetupAttachment(FirstPersonCameraComponent);
	FirstPersonMesh->SetCollisionProfileName(TEXT("NoCollision"));
	FirstPersonMesh->CastShadow = false;
	FirstPersonMesh->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	FirstPersonMesh->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

	// third person
	ThirdPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ThirdPersonMesh"));
	ThirdPersonMesh->SetupAttachment(GetCapsuleComponent());
	ThirdPersonMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ThirdPersonMesh->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
	ThirdPersonMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Block);
	ThirdPersonMesh->bOwnerNoSee = true;
	ThirdPersonMesh->CastShadow = true;
	ThirdPersonMesh->bCastDynamicShadow = true;
	ThirdPersonMesh->bRenderCustomDepth = true;
	ThirdPersonMesh->CustomDepthStencilValue = 1;

	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bQLIsVisible = true;
	bQLIsVulnerable = true;

	bJumpButtonDown = false;

	bQLIsBot = false;

	// capsule
	// in order to achieve good hitbox, let ray-trace occur to the third person skeletal mesh rather than the capsule
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCapsuleComponent()->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	DurationAfterDeathBeforeDestroyed = 3.0f;
	DurationAfterDeathBeforeRespawn = 2.5f;

	MovementParameterQuakeClass = UExpesMovementParameter::StaticClass();

	// Initialize the weapon slots
	WeaponInventory.Init(NULL, 10);
}

void AExpesCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AExpesCharacter, DefaultInventory);
	DOREPLIFETIME(AExpesCharacter, CurrentWeapon);
	DOREPLIFETIME(AExpesCharacter, WeaponInventory);
	DOREPLIFETIME(AExpesCharacter, Shells);
	DOREPLIFETIME(AExpesCharacter, Bullets);
}

void AExpesCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	CreateInventory();

	UpdateHealth();
	UpdateArmor();
}

//------------------------------------------------------------
//------------------------------------------------------------
void AExpesCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (FirstPersonMesh)
	{
		UMaterialInterface* BasicMaterial = FirstPersonMesh->GetMaterial(0);
		DynamicMaterialFirstPersonMesh = FirstPersonMesh->CreateAndSetMaterialInstanceDynamicFromMaterial(0, BasicMaterial);
		FirstPersonMesh->SetMaterial(0, DynamicMaterialFirstPersonMesh.Get());
	}

	if (ThirdPersonMesh)
	{
		UMaterialInterface* BasicMaterial = ThirdPersonMesh->GetMaterial(0);
		DynamicMaterialThirdPersonMesh = ThirdPersonMesh->CreateAndSetMaterialInstanceDynamicFromMaterial(0, BasicMaterial);
		ThirdPersonMesh->SetMaterial(0, DynamicMaterialThirdPersonMesh.Get());
	}

	QLSetVisibility(bQLIsVisible);
	QLSetVulnerability(bQLIsVulnerable);

	// movement data
	// If the movement data is instantiated using
	// NewObject<UQLMovementParameterQuake>(this, MovementParameterQuakeClass->GetFName()),
	// at runtime the default movement data will always be used.
	// To let the movement data be conveniently editable via blueprint subclass,
	// the function call must follow this form:
	MovementParameterQuake = NewObject<UExpesMovementParameter>(this,
	MovementParameterQuakeClass->GetFName(),
	EObjectFlags::RF_NoFlags,
	MovementParameterQuakeClass.GetDefaultObject());

	if (MovementParameterQuake)
	{
		UPawnMovementComponent* MyMovementComp = GetMovementComponent();
		UExpesMovementComponent* MyMovementCompQuake = Cast<UExpesMovementComponent>(MyMovementComp);
		if (MyMovementCompQuake)
		{
			MyMovementCompQuake->SetMovementParameter(MovementParameterQuake);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AExpesCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("NextWeapon", IE_Pressed, this, &AExpesCharacter::NextWeapon);
	PlayerInputComponent->BindAction("PrevWeapon", IE_Pressed, this, &AExpesCharacter::PrevWeapon);

	InputComponent->BindAxis("FireHeld", this, &AExpesCharacter::FireHeld);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &AExpesCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AExpesCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	PlayerInputComponent->BindAxis("Turn", this, &AExpesCharacter::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &AExpesCharacter::AddControllerPitchInput);
}

//------------------------------------------------------------
//------------------------------------------------------------
void AExpesCharacter::Jump()
{
	Super::Jump();

	bJumpButtonDown = true;
}

//------------------------------------------------------------
//------------------------------------------------------------
void AExpesCharacter::StopJumping()
{
	Super::StopJumping();

	bJumpButtonDown = false;
}

//------------------------------------------------------------
//------------------------------------------------------------
bool AExpesCharacter::IsJumpButtonDown()
{
	return bJumpButtonDown;
}

//------------------------------------------------------------
//------------------------------------------------------------
void AExpesCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

//------------------------------------------------------------
//------------------------------------------------------------
void AExpesCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

//------------------------------------------------------------
//------------------------------------------------------------
float AExpesCharacter::GetHealth() const
{
	return Health;
}

//------------------------------------------------------------
//------------------------------------------------------------
void AExpesCharacter::AddHealth(float Increment)
{
	float Temp = Health + Increment;

	if (Temp >= MaxHealth)
	{
		Health = MaxHealth;
	}
	else
	{
		Health = Temp;
	}

	UpdateHealth();
}

//------------------------------------------------------------
//------------------------------------------------------------
float AExpesCharacter::GetMaxHealth() const
{
	return MaxHealth;
}

//------------------------------------------------------------
//------------------------------------------------------------
float AExpesCharacter::GetArmor() const
{
	return Armor;
}

//------------------------------------------------------------
//------------------------------------------------------------
void AExpesCharacter::AddArmor(float Increment)
{
	float Temp = Armor + Increment;

	if (Temp >= MaxArmor)
	{
		Armor = MaxArmor;
	}
	else
	{
		Armor = Temp;
	}

	UpdateArmor();
}

//------------------------------------------------------------
//------------------------------------------------------------
float AExpesCharacter::GetMaxArmor() const
{
	return MaxArmor;
}

//------------------------------------------------------------
// Returns FirstPersonMesh subobject
//------------------------------------------------------------
USkeletalMeshComponent* AExpesCharacter::GetFirstPersonMesh()
{
    return FirstPersonMesh;
}

//------------------------------------------------------------
//------------------------------------------------------------
USkeletalMeshComponent* AExpesCharacter::GetThirdPersonMesh()
{
    return ThirdPersonMesh;
}

//------------------------------------------------------------
// Returns FirstPersonCameraComponent subobject
//------------------------------------------------------------
UCameraComponent* AExpesCharacter::GetFirstPersonCameraComponent() const
{
    return FirstPersonCameraComponent;
}

//------------------------------------------------------------
//------------------------------------------------------------
FHitResult AExpesCharacter::RayTraceFromCharacterPOV(float rayTraceRange)
{
    FCollisionQueryParams params(FName(TEXT("lineTrace")),
        true, // bTraceComplex
        this); // ignore actor
    params.bReturnPhysicalMaterial = false;

    FVector start = FirstPersonCameraComponent->GetComponentLocation();
    FVector end = FirstPersonCameraComponent->GetForwardVector() * rayTraceRange + start;

    FHitResult hitResult(ForceInit);
    // only hit the object that reponds to ray-trace, i.e. ECollisionChannel::ECC_Camera is set to ECollisionResponse::ECR_Block
    GetWorld()->LineTraceSingleByChannel(hitResult, start, end, ECollisionChannel::ECC_Camera, params);

    // useful properties
    // hitResult.bBlockingHit  // did ray hit something
    // hitResult.GetActor();   // the hit actor if there is one
    // hitResult.ImpactPoint;  // FVector
    // hitResult.ImpactNormal; // FVector

    // for debugging purpose
    //DrawDebugLine(GetWorld(), start, hitResult.ImpactPoint, FColor(255, 0, 0), true, -1, 0, 10);

    return hitResult;
}

//------------------------------------------------------------
//------------------------------------------------------------
float AExpesCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    return DamageAmount;
}

//------------------------------------------------------------
//------------------------------------------------------------
void AExpesCharacter::TakeDamageQuakeStyle(float ActualDamage)
{
    if (ActualDamage > 0.0f)
    {
        constexpr float HealthAbsorbingFraction = 1.0f / 3.0f;
        constexpr float ArmorAbsorbingFraction = 1.0f - HealthAbsorbingFraction;
        float HealthDamage = HealthAbsorbingFraction * ActualDamage;
        float ArmorDamage = ArmorAbsorbingFraction * ActualDamage;

        // calculate armor
        float RemainingAmor = Armor - ArmorDamage;
        if (RemainingAmor < 0.0f)
        {
            HealthDamage -= RemainingAmor;
            Armor = 0.0f;
        }
        else
        {
            Armor = RemainingAmor;
        }

        // calculate health
        float RemainingHealth = Health - HealthDamage;
        if (RemainingHealth < 0.0f)
        {
            Health = 0.0f;
        }
        else
        {
            Health = RemainingHealth;
        }
    }
}

//------------------------------------------------------------
//------------------------------------------------------------
void AExpesCharacter::SetHealthArmorBarVisible(bool bFlag)
{
	//TODO
}

//------------------------------------------------------------
//------------------------------------------------------------
void AExpesCharacter::UpdateHealth()
{
	//TODO Update heath UI
}

//------------------------------------------------------------
//------------------------------------------------------------
void AExpesCharacter::UpdateArmor()
{
	//TODO Update armor UI
}

//------------------------------------------------------------
//------------------------------------------------------------
AExpesPlayerController* AExpesCharacter::GetQLPlayerController()
{
    AController* MyController = GetController();
    if (!MyController)
    {
        return nullptr;
    }

    AExpesPlayerController* MyQLPlayerController = Cast<AExpesPlayerController>(MyController);
    if (!MyQLPlayerController)
    {
        return nullptr;
    }

    return MyQLPlayerController;
}

//------------------------------------------------------------
//------------------------------------------------------------
void AExpesCharacter::OnDie()
{
    Destroy();
}

//------------------------------------------------------------
//------------------------------------------------------------
void AExpesCharacter::OnRespawnNewCharacter()
{
}

//------------------------------------------------------------
//------------------------------------------------------------
UAnimSequence* AExpesCharacter::PlayAnimationSequence(const FName& AnimationSequenceName)
{

    return nullptr;
}

//------------------------------------------------------------
//------------------------------------------------------------
void AExpesCharacter::SetDamageMultiplier(const float Value)
{
}

//------------------------------------------------------------
//------------------------------------------------------------
void AExpesCharacter::SetProtectionMultiplier(const float Value)
{
    ProtectionMultiplier = Value;
}

//------------------------------------------------------------
//------------------------------------------------------------
void AExpesCharacter::StartGlow()
{

}

//------------------------------------------------------------
//------------------------------------------------------------
void AExpesCharacter::StopGlow()
{

}

//------------------------------------------------------------
//------------------------------------------------------------
bool AExpesCharacter::IsAlive()
{
    if (Health > 0.0f)
    {
        return true;
    }
    else
    {
        return false;
    }
}

//------------------------------------------------------------
//------------------------------------------------------------
void AExpesCharacter::PlaySoundFireAndForget(const FName& SoundName)
{
    USoundBase** Result = SoundList.Find(SoundName);
    if (Result)
    {
        USoundBase* Sound = *Result;
        if (Sound)
        {
            UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, GetActorLocation());
        }
    }
}

//------------------------------------------------------------
//------------------------------------------------------------
void AExpesCharacter::PlaySound(const FName& SoundName)
{
    USoundBase** Result = SoundList.Find(SoundName);
    if (Result)
    {
        USoundBase* Sound = *Result;
        if (Sound)
        {
            SoundComponent->SetSound(Sound);
            SoundComponent->Play(0.0f);

            //// sound played using this function is fire and forget and does not travel with the actor
            UGameplayStatics::PlaySoundAtLocation(this, Sound, GetActorLocation());
        }
    }
}

//------------------------------------------------------------
//------------------------------------------------------------
void AExpesCharacter::StopSound()
{
    if (SoundComponent)
    {
        if (SoundComponent->IsPlaying())
        {
            SoundComponent->Stop();
        }
    }
}

//------------------------------------------------------------
//------------------------------------------------------------
void AExpesCharacter::SetWeaponEnabled(const bool bFlag)
{
    bCanFireAndAltFire = bFlag;
    bCanSwitchWeapon = bFlag;
}

//------------------------------------------------------------
//------------------------------------------------------------
bool AExpesCharacter::GetIsBot()
{
    return bQLIsBot;
}

//------------------------------------------------------------
//-----------------------------------------------------------
void AExpesCharacter::SetIsBot(bool bFlag)
{
    bQLIsBot = bFlag;
}

//------------------------------------------------------------
//------------------------------------------------------------
void AExpesCharacter::SetMaxWalkSpeed(const float MaxWalkSpeed)
{
    auto* MyCharacterMovement = GetCharacterMovement();
    if (MyCharacterMovement)
    {
        MyCharacterMovement->MaxWalkSpeed = MaxWalkSpeed;
    }
}

//------------------------------------------------------------
//------------------------------------------------------------
void AExpesCharacter::ResetMaxWalkSpeed()
{
    auto* MyCharacterMovement = GetCharacterMovement();
    if (MyCharacterMovement)
    {
        MyCharacterMovement->MaxWalkSpeed = 600.0f;
    }
}

//------------------------------------------------------------
//------------------------------------------------------------
bool AExpesCharacter::QLGetVisibility()
{
    return bQLIsVisible;
}

//------------------------------------------------------------
//------------------------------------------------------------
void AExpesCharacter::QLSetVisibility(const bool bFlag)
{
    if (FirstPersonMesh)
    {
        FirstPersonMesh->SetVisibility(bFlag);
    }

    if (ThirdPersonMesh)
    {
        ThirdPersonMesh->SetVisibility(bFlag);
    }

    bQLIsVisible = bFlag;
}

//------------------------------------------------------------
//------------------------------------------------------------
bool AExpesCharacter::QLGetVulnerability()
{
    return bQLIsVulnerable;
}

//------------------------------------------------------------
//------------------------------------------------------------
void AExpesCharacter::QLSetVulnerability(const bool bFlag)
{
    bQLIsVulnerable = bFlag;
}

//------------------------------------------------------------
//------------------------------------------------------------
void AExpesCharacter::EquipAll()
{
    if (bQLIsBot)
    {
        SetCurrentMovementStyle(EQLMovementStyle::Default);
    }
    else
    {
        SetCurrentMovementStyle(EQLMovementStyle::QuakeVallina);
    }
}

//------------------------------------------------------------
//------------------------------------------------------------
void AExpesCharacter::FellOutOfWorld(const UDamageType& dmgType)
{
    OnDie();
}

//------------------------------------------------------------
//------------------------------------------------------------
void AExpesCharacter::AddControllerYawInput(float Val)
{
    Super::AddControllerYawInput(0.6f * Val);
}

//------------------------------------------------------------
//------------------------------------------------------------
void AExpesCharacter::AddControllerPitchInput(float Val)
{
    Super::AddControllerPitchInput(0.6f * Val);
}

//------------------------------------------------------------
//------------------------------------------------------------
void AExpesCharacter::SetCurrentMovementStyle(EQLMovementStyle MyStyle)
{
    UCharacterMovementComponent* CharacterMovementComponent = GetCharacterMovement();
    UExpesMovementComponent* MoveComponentQuake = Cast<UExpesMovementComponent>(CharacterMovementComponent);
    if (MoveComponentQuake)
    {
        MoveComponentQuake->SetMovementStyle(MyStyle);
    }
}

void AExpesCharacter::CreateInventory()
{
	UE_LOG(LogTemp, Warning, TEXT(""));
	if (Role != ROLE_Authority)
	{
		ServerCreateInventory();
	}
	else
	{
		for (auto& curWeapon : DefaultInventory)
		{
			if (curWeapon)
			{
				// TODO: Find out why the second player is creating a null weapon
				AddWeapon(curWeapon);
			}
		}

		UpdateCurrentWeapon();
	}
}

void AExpesCharacter::AddWeapon(TSubclassOf<AExpesWeapon> WeaponClass)
{
	AExpesWeapon* weapon = GetWorld()->SpawnActor<AExpesWeapon>(WeaponClass);
	if (weapon)
	{
		//WeaponInventory.Emplace(weapon);
		WeaponInventory[weapon->SlotID] = weapon;
		weapon->SetActorHiddenInGame(true);
		SetAmmo(weapon->AmmoType, GetAmmo(weapon->AmmoType) + weapon->DefaultAmmo);
	}
}

void AExpesCharacter::ServerCreateInventory_Implementation()
{
	CreateInventory();
}

bool AExpesCharacter::ServerCreateInventory_Validate()
{
	return true;
}

void AExpesCharacter::NextWeapon()
{
	if (Role != ROLE_Authority)
	{
		ServerNextWeapon();
	}
	else
	{
		for (int32 Index = 0; Index != WeaponInventory.Num(); ++Index)
		{
			if (WeaponInventory[Index] != NULL && WeaponInventory[Index]->SlotID > WeaponIndex)
			{
				WeaponIndex = WeaponInventory[Index]->SlotID;
				UpdateCurrentWeapon();
				return;
			}
			else
			{
				WeaponIndex = WeaponIndex;
			}
		}
		//WeaponIndex++;

		/*if (WeaponInventory[WeaponIndex] == NULL)
		{
			WeaponIndex--;
		}

		if (WeaponIndex >= 9)
		{
			WeaponIndex = WeaponIndex;
		}

		UpdateCurrentWeapon();*/
	}
}

void AExpesCharacter::ServerNextWeapon_Implementation()
{
	NextWeapon();
}

bool AExpesCharacter::ServerNextWeapon_Validate()
{
	return true;
}

void AExpesCharacter::PrevWeapon()
{
	if (Role != ROLE_Authority)
	{
		ServerPrevWeapon();
	}
	else
	{
		for (int32 Index = WeaponInventory.Num() -1; Index >= 0; --Index)
		{
			if (WeaponInventory[Index] != NULL && WeaponInventory[Index]->SlotID < WeaponIndex)
			{
				WeaponIndex = WeaponInventory[Index]->SlotID;
				UpdateCurrentWeapon();
				return;
			}
			else
			{
				WeaponIndex = WeaponIndex;
			}
		}
		/*WeaponIndex--;
		if (WeaponIndex < 0
			|| WeaponIndex >= 255)
			WeaponIndex = 0;

		UpdateCurrentWeapon();*/
	}
}

void AExpesCharacter::ServerPrevWeapon_Implementation()
{
	PrevWeapon();
}

bool AExpesCharacter::ServerPrevWeapon_Validate()
{
	return true;
}

int32 AExpesCharacter::GetAmmo(EAmmoType ammoType)
{
	switch (ammoType)
	{
	case EAmmoType::ENone:
		return 1;
		break;
	case EAmmoType::EShell:
		return Shells;
		break;
	case EAmmoType::EBullet:
		return Bullets; 
		break;
	case EAmmoType::ECell:
		return Cells;
		break;
	case EAmmoType::ERocket:
		return Rockets;
		break;
	case EAmmoType::ESlug:
		return Slugs;
		break;
	default:
		return 0;
		break;
	}
}

int32 AExpesCharacter::GetCurrentWeaponAmmo()
{
	if (CurrentWeapon != nullptr)
	{
		return GetAmmo(CurrentWeapon->AmmoType);
	}

	return 0;
}

int32 AExpesCharacter::GetCurrentWeaponMaxAmmo()
{
	if (CurrentWeapon != nullptr)
	{
		return GetMaxAmmo(CurrentWeapon->AmmoType);
	}

	return 0;
}

void AExpesCharacter::SetAmmo(EAmmoType ammoType, int32 value)
{
	switch (ammoType)
	{
	case EAmmoType::EShell:
		Shells = value;
		Shells = FMath::Clamp(Shells, 0, MaxShells);
		break;
	case EAmmoType::EBullet:
		Bullets = value;
		Bullets = FMath::Clamp(Bullets, 0, MaxBullets);
		break;
	case EAmmoType::ECell:
		Cells = value;
		Cells = FMath::Clamp(Cells, 0, MaxCells);
		break;
	case EAmmoType::ERocket:
		Rockets = value;
		Rockets = FMath::Clamp(Rockets, 0, Rockets);
		break;
	case EAmmoType::ESlug:
		Slugs = value;
		Slugs = FMath::Clamp(Slugs, 0, Slugs);
		break;
	}
}

void AExpesCharacter::FireHeld(float Val)
{
	if (Val > 0)
	{
		WasFiring = true;
		if (Role == ROLE_Authority)
		{
			CurrentWeapon->Fire(this);
		}
		else
		{
			ServerFireHeld(Val);
		}
	}
	else
	{
		if (WasFiring)
		{
			CurrentWeapon->EndFire();
			WasFiring = false;
		}
	}

	// Get the animation object for the arms mesh
	/*UAnimInstance* AnimInstance = FirstPersonMesh->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Play(FireAnimation, 1.0f);
	}*/
}

void AExpesCharacter::ServerFireHeld_Implementation(float Val)
{
	FireHeld(Val);
}

bool AExpesCharacter::ServerFireHeld_Validate(float Val)
{
	return true;
}

bool AExpesCharacter::PickupWeapon(TSubclassOf<AExpesWeapon> WeaponClass)
{
	bool weaponExists = false;
	for (auto& weapon : WeaponInventory)
	{
		if (weapon && weapon->GetClass() == WeaponClass)
		{
			weaponExists = true;
			break;
		}
	}

	if (weaponExists)
	{
		EAmmoType weaponAmmoType = WeaponClass.GetDefaultObject()->AmmoType;
		if (GetAmmo(weaponAmmoType) == GetMaxAmmo(weaponAmmoType))
		{
			// We already have the weapon and full ammo, so we shouldn't pick it up
			return false;
		}
		else
		{
			SetAmmo(weaponAmmoType, GetAmmo(weaponAmmoType) + WeaponClass.GetDefaultObject()->DefaultAmmo);
			return true;
		}
	}

	// If the weapon doesn't exist, create it
	AddWeapon(WeaponClass);
	return true;
}

int32 AExpesCharacter::GetMaxAmmo(EAmmoType ammoType)
{
	switch (ammoType)
	{
	case EAmmoType::EShell:
		return MaxShells;
	case EAmmoType::EBullet:
		return MaxBullets;
	case EAmmoType::ECell:
		return MaxCells;
	case EAmmoType::ERocket:
		return MaxRockets;
	case EAmmoType::ESlug:
		return MaxSlugs;
	default:
		return 0;
	}
}

void AExpesCharacter::UpdateCurrentWeapon()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->DetachFromActor(FDetachmentTransformRules(EDetachmentRule::KeepWorld, false));
		CurrentWeapon->SetActorHiddenInGame(true);
	}
	CurrentWeapon = WeaponInventory[WeaponIndex];
	CurrentWeapon->SetActorHiddenInGame(false);
	CurrentWeapon->AttachToComponent(FirstPersonMesh, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
}
