// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SPowerupActor.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;
class URotatingMovementComponent;

UCLASS()
class COOPHORDEGAME_API ASPowerupActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASPowerupActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	UPointLightComponent* PointLightComp;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	URotatingMovementComponent* RotationComp;

	// Reference to the static mesh material
	UMaterialInstanceDynamic* MaterialInst;

	/* Time betweeen powerup ticks */
	UPROPERTY(EditDefaultsOnly, Category = "Powerups")
	float PowerupInterval;

	/* Total times we apply the power up effect */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Powerups")
	int32 TotalNumOfTicks;

	FTimerHandle TimerHandle_PowerupTick;

	// Total number of ticks processed
	UPROPERTY( BlueprintReadOnly, Category = "Powerups")
	int32 TicksProcessed;

	UPROPERTY(ReplicatedUsing=OnRep_PowerupActive)
	bool bIsPowerupActive;

	UFUNCTION()
	void OnRep_PowerupActive();

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
	void OnPowerupStateChanged(bool bNewIsActive);

public:	
	
	void ActivatePowerup(AActor* CurrentActor);

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
	void OnActivated(AActor* CurrentActor);

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
	void OnPowerupTicked();

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
	void OnExpired();


	//UFUNCTION()
	void OnTickPowerup();

};
