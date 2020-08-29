// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RailBeam.generated.h"

class UParticleSystemComponent;

UCLASS()
class EXPES_API ARailBeam : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARailBeam();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++Property")
	UParticleSystemComponent* BeamComponent;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called every frame
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "C++Function")
	UParticleSystemComponent* GetBeamComponent();
};
