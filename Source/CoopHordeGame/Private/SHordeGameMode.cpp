// Fill out your copyright notice in the Description page of Project Settings.

#include "SHordeGameMode.h"
#include "SHealthComponent.h"
#include "TimerManager.h"
#include "SGameState.h"
#include "SPlayerState.h"



ASHordeGameMode::ASHordeGameMode()
{
	TimeBetweenWaves = 2.0f;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.0f;

	GameStateClass = ASGameState::StaticClass();
	PlayerStateClass = ASPlayerState::StaticClass();
}

void ASHordeGameMode::StartPlay()
{
	Super::StartPlay();

	PrepareForNextWave();
}

void ASHordeGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	CheckWaveState();
	CheckAnyPlayerAlive();
}

void ASHordeGameMode::StartWave()
{
	WaveCount++;

	NumOfBotsToSpawn = WaveCount * 2;

	GetWorldTimerManager().SetTimer(TimerHandle_BotSpawner, this, &ASHordeGameMode::SpawnBotTimerElapsed, 1.0f, true, 0.0f);

	SetWaveState(EWaveState::WaveInProgress);
}

void ASHordeGameMode::EndWave()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);

	SetWaveState(EWaveState::WaitingToComplete);
}

void ASHordeGameMode::SpawnBotTimerElapsed()
{
	SpawnNewBot();

	NumOfBotsToSpawn--;

	if (NumOfBotsToSpawn <= 0) {
		EndWave();
	}
}

void ASHordeGameMode::PrepareForNextWave()
{
	GetWorldTimerManager().SetTimer(TimerHandle_NextWaveStart, this, &ASHordeGameMode::StartWave, TimeBetweenWaves, false);

	SetWaveState(EWaveState::WaitingToStart);

	RespawnPlayer();
}

void ASHordeGameMode::CheckWaveState()
{
	bool bIsPreparingForWave = GetWorldTimerManager().IsTimerActive(TimerHandle_NextWaveStart);

	if (NumOfBotsToSpawn > 0 || bIsPreparingForWave) {
		return;
	}

	bool bIsAnyBotAlive = false;

	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It) {
		APawn* TempPawn = It->Get();

		if (TempPawn == nullptr || TempPawn->IsPlayerControlled()) {
			continue;
		}

		USHealthComponent* HealthComp = Cast<USHealthComponent>(TempPawn->GetComponentByClass(USHealthComponent::StaticClass()));
		if (HealthComp && HealthComp->GetHealth() > 0.0f) {
			bIsAnyBotAlive = true;
			break;
		}
	}

	if (!bIsAnyBotAlive) {
		SetWaveState(EWaveState::WaveComplete);

		PrepareForNextWave();
	}
}

void ASHordeGameMode::CheckAnyPlayerAlive()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It) {

		APlayerController* PlayerController = It->Get();

		if (PlayerController && PlayerController->GetPawn()) {
			APawn* MyPawn = PlayerController->GetPawn();
			USHealthComponent* HealthComp = Cast<USHealthComponent>(MyPawn->GetComponentByClass(USHealthComponent::StaticClass()));

			if (ensure(HealthComp) && HealthComp->GetHealth() > 0.0f) {
				// A player is still alive
				return;
			}
		}
	}

	// No player alive
	GameOver();
}

void ASHordeGameMode::GameOver()
{
	EndWave();

	// @TODO: Finish up the match, present 'game over' to players.
	SetWaveState(EWaveState::GameOver);

	UE_LOG(LogTemp, Log, TEXT("GAME OVER! Players Died"));
}

void ASHordeGameMode::SetWaveState(EWaveState NewState)
{
	ASGameState* GameState = GetGameState<ASGameState>();
	
	if (ensureAlways(GameState)) {
		GameState->SetWaveState(NewState);
	}
}

void ASHordeGameMode::RespawnPlayer()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It) {
		
		APlayerController* PlayerController = It->Get();
		if (PlayerController && PlayerController->GetPawn() == nullptr) {
			RestartPlayer(PlayerController);
		}
	}
}

