// Fill out your copyright notice in the Description page of Project Settings.


#include "ExpesAICharacter.h"
#include "ExpesWeapon.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// Sets default values
AExpesAICharacter::AExpesAICharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create a CameraComponent
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCameraComponent"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	//FirstPersonCameraComponent->RelativeLocation = FVector(-39.56f, 1.75f, 64.f); // Position the camera
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.0f, 1.75f, 64.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true; // critical! If false, mouse would not change pitch!
	FirstPersonCameraComponent->SetFieldOfView(100.0f);
}

// Called when the game starts or when spawned
void AExpesAICharacter::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;

	Gun = GetWorld()->SpawnActor<AExpesWeapon>(GunClass);
	Gun->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
	Gun->SetOwner(this);

}

// Called every frame
void AExpesAICharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AExpesAICharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

bool AExpesAICharacter::IsDead() const
{
	return Health <= 0;
}

float AExpesAICharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	float DamageToApply = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	DamageToApply = FMath::Min(Health, DamageToApply);
	Health -= DamageToApply;
	UE_LOG(LogTemp, Warning, TEXT("Health left %f"), Health);

	if (IsDead())
	{
		DetachFromControllerPendingDestroy();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	return DamageToApply;
}

void AExpesAICharacter::MoveForward(float AxisValue)
{
	AddMovementInput(GetActorForwardVector() * AxisValue);
}

void AExpesAICharacter::MoveRight(float AxisValue)
{
	AddMovementInput(GetActorRightVector() * AxisValue);
}

void AExpesAICharacter::LookUpRate(float AxisValue)
{
	AddControllerPitchInput(AxisValue * RotationRate * GetWorld()->GetDeltaSeconds());
}

void AExpesAICharacter::LookRightRate(float AxisValue)
{
	AddControllerYawInput(AxisValue * RotationRate * GetWorld()->GetDeltaSeconds());
}

void AExpesAICharacter::Shoot()
{
	//UGameplayStatics::SpawnEmitterAttached(MuzzleFlash, Gun->WeaponMesh, TEXT("Muzzle"));
	//UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, Hit.Location, ShotDirection.Rotation());


	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(GetOwner());

	FVector start = FirstPersonCameraComponent->GetComponentLocation();
	FVector end = FirstPersonCameraComponent->GetForwardVector() * MaxRange + start;

	FHitResult hitResult(ForceInit);
	// only hit the object that reponds to ray-trace, i.e. ECollisionChannel::ECC_Camera is set to ECollisionResponse::ECR_Block
	GetWorld()->LineTraceSingleByChannel(hitResult, start, end, ECollisionChannel::ECC_Visibility, Params);

	// for debugging purpose
	DrawDebugLine(GetWorld(), start, hitResult.ImpactPoint, FColor(255, 0, 0), true, -1, 0, 10);

	AActor* HitActor = hitResult.GetActor();
	if (HitActor != nullptr)
	{
		FVector ShotDirection = -FirstPersonCameraComponent->GetForwardVector();
		FPointDamageEvent DamageEvent(Damage, hitResult, ShotDirection, nullptr);
		AController* OwnerController = GetController();
		HitActor->TakeDamage(Damage, DamageEvent, OwnerController, this);
	}
}




