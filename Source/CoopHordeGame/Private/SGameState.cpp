// Fill out your copyright notice in the Description page of Project Settings.

#include "SGameState.h"
#include "Net/UnrealNetwork.h"


void ASGameState::OnRep_WaveState(EWaveState CurrentState)
{
	WaveStateChanged(WaveState, CurrentState);
}

void ASGameState::SetWaveState(EWaveState NewState)
{
	if (Role = ROLE_Authority) {

		EWaveState CurrentState = WaveState;

		WaveState = NewState;

		// Call on server
		OnRep_WaveState(CurrentState);
	}
}

EWaveState ASGameState::GetWaveState()
{
	return WaveState;
}

void ASGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const {

	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASGameState, WaveState);
}

