// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthPickup.h"
#include "Expes/ExpesCharacter.h"
#include "Kismet/GameplayStatics.h"

AHealthPickup::AHealthPickup()
{

}

void AHealthPickup::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor->IsA(AExpesCharacter::StaticClass()))
    {
        AExpesCharacter* player = Cast<AExpesCharacter>(OtherActor);
        if (player->bIsAI == false)
        {
            player->AddHealth(HealthAmount);
            UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());
            Destroy();
        }
        else
        {
            return;
        }
    }
}
