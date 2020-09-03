// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseWeaponPickup.h"
#include "Expes/ExpesCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"

ABaseWeaponPickup::ABaseWeaponPickup()
{
    WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
    WeaponMesh->SetupAttachment(BoxCollision);
}

void ABaseWeaponPickup::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor->IsA(AExpesCharacter::StaticClass()))
    {
        AExpesCharacter* player = Cast<AExpesCharacter>(OtherActor);
        if (player->bIsAI == false)
        {
            if (player->PickupWeapon(Weapon))
            {
                if (PickupSound)
                {
                    UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());
                }

                Destroy();
            }
        }
        else
        {
            return;
        }
    }
}