// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SGameState.generated.h"

UENUM(BlueprintType)
enum class EWaveState : uint8 {
	WaitingToStart,

	WaveInProgress,

	// Stops spawning bots and waiting for players to complete
	WaitingToComplete,

	WaveComplete,

	GameOver
};


/**
 * 
 */
UCLASS()
class COOPHORDEGAME_API ASGameState : public AGameStateBase
{
	GENERATED_BODY()
	
protected:
	
	UPROPERTY(BlueprintReadOnly, Category = "GameState", ReplicatedUsing = OnRep_WaveState)
	EWaveState WaveState;

	UFUNCTION(BlueprintImplementableEvent, Category = "GameState")
	void WaveStateChanged(EWaveState NewState, EWaveState CurrentState);

	UFUNCTION()
	void OnRep_WaveState(EWaveState CurrentState);

public:

	void SetWaveState(EWaveState NewState);

	EWaveState GetWaveState();

};
