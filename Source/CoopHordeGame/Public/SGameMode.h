// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SGameMode.generated.h"

/**
 *  The Game Mode header class
 */
UCLASS()
class COOPHORDEGAME_API ASGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
/* Public Functions */
public:

	ASGameMode();

	virtual void StartPlay() override;

	virtual void Tick(float DeltaSeconds) override;
	
/* Protected Variables */
protected:

	FTimerHandle TimerHandle_BotSpawner;

	FTimerHandle TimerHandle_NextWaveStart;

	// Bots to spawn in current array
	UPROPERTY(BlueprintReadOnly, Category = "GameMode")
	int32 NumOfBotsToSpawn;

	UPROPERTY(BlueprintReadOnly, Category = "GameMode")
	int32 WaveCount;

	UPROPERTY(EditDefaultsOnly, Category = "GameMode")
	float TimeBetweenWaves; 

	bool bPreapreWaveActive;

	/* Protected functions */
protected:

	// Hook for BP to spawn a single bot
	UFUNCTION(BlueprintImplementableEvent, Category = "GameMode")
	void SpawnNewBot();

	// Start Spawning Bots
	void StartWave();

	// Stop spawning Bots
	void EndWave();

	// Spawns bots on timer 
	void SpawnBotTimerElapsed();

	// Set Timer for next wave
	void PrepareForNextWave();

	// Checks wave state
	void CheckWaveState();

};
