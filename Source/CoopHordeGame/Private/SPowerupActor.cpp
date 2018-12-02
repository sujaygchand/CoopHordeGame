// Fill out your copyright notice in the Description page of Project Settings.

#include "SPowerupActor.h"
#include "TimerManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "ConstructorHelpers.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "Net/UnrealNetwork.h"

//TODO: 8:00
// Sets default values
ASPowerupActor::ASPowerupActor()
{
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> LightFunctionMaterial(TEXT("/Game/Powerups/M_PowerupLightFunction"));

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = MeshComp;

	PointLightComp = CreateDefaultSubobject<UPointLightComponent>(TEXT("PointLightComp"));
	PointLightComp->AttenuationRadius = 200.0f;
	PointLightComp->CastShadows = false;
	PointLightComp->SetLightFunctionMaterial(LightFunctionMaterial.Object);
	PointLightComp->SetupAttachment(RootComponent);

	RotationComp = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotationComp"));

	PowerupInterval = 0.0f;
	TotalNumOfTicks = 0;

	bIsPowerupActive = false;

	SetReplicates(true);

}

// Called when the game starts or when spawned
void ASPowerupActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (Role == ROLE_Authority) {
		if (MeshComp) {
			MaterialInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));

			PointLightComp->SetLightColor(MaterialInst->K2_GetVectorParameterValue("Colour"));
		}
	}
}


void ASPowerupActor::OnRep_PowerupActive()
{
	OnPowerupStateChanged(bIsPowerupActive);
}

void ASPowerupActor::OnTickPowerup()
{
	TicksProcessed++;

	OnPowerupTicked();

	if (TicksProcessed >= TotalNumOfTicks) {
		OnExpired();

		bIsPowerupActive = true;
		OnRep_PowerupActive();

		// Delete timer
		GetWorldTimerManager().ClearTimer(TimerHandle_PowerupTick);
		TicksProcessed = 0;
	}
}

void ASPowerupActor::ActivatePowerup(AActor* CurrentActor)
{
	OnActivated(CurrentActor);

	bIsPowerupActive = true;
	OnRep_PowerupActive();

	if (PowerupInterval > 0.0f) {
		GetWorldTimerManager().SetTimer(TimerHandle_PowerupTick, this, &ASPowerupActor::OnTickPowerup, PowerupInterval, true);
		
	} {
		OnTickPowerup();
	}
}

void ASPowerupActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {

	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASPowerupActor, bIsPowerupActive);
}
