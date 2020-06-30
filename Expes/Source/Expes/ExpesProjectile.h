// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <vector>
#include "ExpesProjectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class AExpesCharacter;
class AExpesPlayerController;

UCLASS(config=Game)
class AExpesProjectile : public AActor
{
	GENERATED_BODY()

public:
	AExpesProjectile();

    //------------------------------------------------------------
    // Called every frame
    //------------------------------------------------------------
    virtual void Tick(float DeltaTime) override;

    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    UProjectileMovementComponent* GetProjectileMovementComponent();

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    void QLSetPlayerController(AExpesPlayerController* PlayerControllerExt);

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    void SetDamageMultiplier(const float Value);

    UFUNCTION(BlueprintCallable, Category = "C++Function")
    void PlaySoundFireAndForget(const FName& SoundName);

protected:
    //------------------------------------------------------------
    // Called when the game starts or when spawned
    //------------------------------------------------------------
    virtual void BeginPlay() override;

    UFUNCTION()
    virtual void OnBeginOverlapForComponent(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    virtual void HandleDirectHit(AActor* OtherActor, bool& bSelfDirectHit, bool& bDirectHit);

    UFUNCTION()
    virtual void HandleSplashHit(AActor* OtherActor, bool bDirectHit);

    //------------------------------------------------------------
    // Given input damage, reduce it and return the result
    //------------------------------------------------------------
    UFUNCTION(BlueprintCallable, Category = "C++Function")
    float ReduceSelfDamage(const float InDamage);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++Property")
    UProjectileMovementComponent* ProjectileMovementComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++Property")
    USphereComponent* RootSphereComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++Property")
    UStaticMeshComponent* StaticMeshComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++Property")
    float ProjectileLifeSpan;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++Property")
    float BlastRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++Property")
    float BlastSpeedChange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++Property")
    float BlastSpeedChangeSelfDamageScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++Property")
    float BasicDamage;

    UPROPERTY()
    float BasicDamageAdjusted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++Property")
    UParticleSystem* ExplosionParticleSystem;

    UPROPERTY()
    TWeakObjectPtr<AExpesPlayerController> PlayerController;

    UPROPERTY()
    float DamageMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++Property")
    TMap<FName, USoundBase*> SoundList;

    //UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++Property")
    //USoundAttenuation* SoundAttenuation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++Property")
    float ExplosionParticleSystemScale;

    // temporary container to hold victims of splash damage
    std::vector<TWeakObjectPtr<AActor>> SplashDamageVictimList;

    bool bIsSelfDirectHit;
    bool bIsDirectHit;
};

