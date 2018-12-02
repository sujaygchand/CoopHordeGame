// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SPickupActor.generated.h"

class USphereComponent;
class ASPowerupActor;

UCLASS()
class COOPHORDEGAME_API ASPickupActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASPickupActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Sphere Comp
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* SphereComp;

	// Decal
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UDecalComponent* DecalComp;

	// Power up child actor
	UPROPERTY(EditInstanceOnly, Category = "PowerupActor")
	TSubclassOf<ASPowerupActor> PowerUpClass;

	UPROPERTY(BlueprintReadOnly, Category = "PowerupActor")
	ASPowerupActor* PoweredupInstance;

	void Respawn();

	UPROPERTY(EditDefaultsOnly, Category = "PickupActor")
	float CooldownDuration;

	FTimerHandle TimerHandle_Respawn;

public:	
	
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
};
