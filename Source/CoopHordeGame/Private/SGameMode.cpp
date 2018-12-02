// Fill out your copyright notice in the Description page of Project Settings.

#include "SGameMode.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "SHealthComponent.h"
#include "EnvironmentQuery/EQSTestingPawn.h"
#include "Kismet/GameplayStatics.h"
#include "STrackerBot.h"

/**
*  The Game Mode cpp class
*/
ASGameMode::ASGameMode() {
	TimeBetweenWaves = 2.0f;
	WaveCount = 3;
	bPreapreWaveActive = true;

	// Tick variables
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.0f;

}

/**
 * Starts the spawn Wave
 */
void ASGameMode::StartWave()
{
	// Add to wave count
	WaveCount++;

	// Bots spawned depends on wave
	NumOfBotsToSpawn = WaveCount;

	// Starts the spawn timer
	GetWorldTimerManager().SetTimer(TimerHandle_BotSpawner, this, &ASGameMode::SpawnBotTimerElapsed, 1.0f, true, 0.0f);
}

/**
 * Ends the spawn Wave and clears timer
 */
void ASGameMode::EndWave()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);
	bPreapreWaveActive = true;
}

/**
 * Spawning Bot timer
 */
void ASGameMode::SpawnBotTimerElapsed()
{

	NumOfBotsToSpawn--;

	SpawnNewBot();

	if (NumOfBotsToSpawn <= 0) {
		EndWave();
	}
}

/**
 * Initiates the next wave
 */
void ASGameMode::PrepareForNextWave()
{
	if (bPreapreWaveActive) {
		bPreapreWaveActive = false;
		GetWorldTimerManager().SetTimer(TimerHandle_NextWaveStart, this, &ASGameMode::StartWave, TimeBetweenWaves, false);
	}
}

/**
 * Checks on the Wave status
 */
void ASGameMode::CheckWaveState()
{

	bool bIsPreparingForWave = GetWorldTimerManager().IsTimerActive(TimerHandle_NextWaveStart);
	
	// Stops check, if wave is in preparation 
	if (NumOfBotsToSpawn > 0 || bIsPreparingForWave) {
		return;
	}

	bool bIsAnyBotAlive = false;

	/*
	TArray<AActor*> TempActors;

	UGameplayStatics::GetAllActorsOfClass(this, ASTrackerBot::StaticClass(), TempActors);

	if (TempActors.Num() <= 0) {
		PrepareForNextWave();
	}
	*/

	// Goes thru every Pawn in the level and checks if any non-player controlled pawns are alive
	// When all AI is dead the next wave starts
	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It) {
		APawn* TempPawn = It->Get();

		// Skips current iteration
		if (TempPawn == nullptr || TempPawn->IsPlayerControlled()) {

			continue;
		}

		// Finds if current bot has health component
		USHealthComponent* HealthComp = Cast<USHealthComponent>(TempPawn->GetComponentByClass(USHealthComponent::StaticClass()));

		// Breaks if still alive
		if (HealthComp && HealthComp->GetHealth() > 0.0f) {

			bIsAnyBotAlive = true;
			break;
		}

		// No bots are alive, so next wave starts
		if (!bIsAnyBotAlive) {
			UE_LOG(LogTemp, Warning, TEXT("Pawn with healyj comp is %s"), *TempPawn->GetName())
				PrepareForNextWave();
		}
	}
}

/**
 * On Start Play
*/
void ASGameMode::StartPlay()
{
	Super::StartPlay();

	PrepareForNextWave();
}

/**
 * On Tick
*/
void ASGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	CheckWaveState();
}
