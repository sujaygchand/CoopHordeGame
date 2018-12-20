// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "STrackerBot.generated.h"

class UStaticMeshComponent;
class USHealthComponent;
class USphereComponent;
class UAudioComponent;

/**
 * Tracker Bot AI header class
*/
UCLASS()
class COOPHORDEGAME_API ASTrackerBot : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASTrackerBot();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	USHealthComponent* BotHealthComp;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	USphereComponent* PlayerSenseSphere;

	UPROPERTY(VisibleDefaultsOnly, Category = "Sounds")
	UAudioComponent* AudioComponent;

	// Dynamic material to pluse on damage
	UMaterialInstanceDynamic* MaterialInst;

	// Explosion Effect
	UPROPERTY(EditDefaultsOnly, Category = "Components")
	UParticleSystem* ExplosionEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundBase* SelfDestructTimerSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundBase* SelfDestructSound;

	//Returns the next point to travel
	FVector GetNextPathPoint();

	// Next path point
	FVector NextPathPoint;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float MovementForce;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float RequiredDistanceToTarget;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	bool bUseVelocityChange;

	bool bExploded;

	bool bStartedSelfDestruction;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float ExplosionRadius;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float ExplosionDamage;
	
	FTimerHandle TimerHandel_SelfDamage;

	FTimerHandle TimerHandel_BoostPowerLevel;

	FTimerHandle TimerHandel_RefreshPath;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float SelfDamageInterval;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TrackerBot")
	int PowerLevel;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TrackerBot")
	int MaxPowerLevel;

	UFUNCTION()
	void HandleTakeDamage(USHealthComponent* HealthComp, float Health, float HealthDelta, 
	const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	void SelfDestruct();

	void DamageSelf();

	void Patrol();

	void PlayRollSound();

	void BoostPowerLevel();

	void RefreshPath();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	
	void SetPowerLevel(int PowerLevel);
};
