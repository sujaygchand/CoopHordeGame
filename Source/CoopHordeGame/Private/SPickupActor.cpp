// Fill out your copyright notice in the Description page of Project Settings.

#include "SPickupActor.h"
#include "Components/SphereComponent.h"
#include "Components/DecalComponent.h"
#include "Engine/World.h"
#include "SPowerupActor.h"
#include "TimerManager.h"
#include "SCharacter.h"

// Sets default values
ASPickupActor::ASPickupActor()
{

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(75.0f);
	RootComponent = SphereComp;

	DecalComp = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComp"));
	DecalComp->SetRelativeRotation(FRotator(90, 0, 0));
	DecalComp->DecalSize = FVector(64, 75, 75);
	DecalComp->SetupAttachment(RootComponent);

	CooldownDuration = 10.0f;
	
	SetReplicates(true);
}

// Called when the game starts or when spawned
void ASPickupActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (Role == ROLE_Authority) {
		Respawn();
	}
}

void ASPickupActor::Respawn()
{
	if (PowerUpClass == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("Powered up class is nullptr in %s. Please update your BP"), *GetName())
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	PoweredupInstance = GetWorld()->SpawnActor<ASPowerupActor>(PowerUpClass, GetTransform(), SpawnParams);
}

void ASPickupActor::NotifyActorBeginOverlap(AActor * OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (Cast<ASCharacter>(OtherActor)) {

		if (PoweredupInstance && Role == ROLE_Authority) {
			PoweredupInstance->ActivatePowerup(OtherActor);
			PoweredupInstance = nullptr;

			// Set timer to respawn
			GetWorldTimerManager().SetTimer(TimerHandle_Respawn, this, &ASPickupActor::Respawn, CooldownDuration);
			

		}
	}
}


