// Fill out your copyright notice in the Description page of Project Settings.

#include "Public/AI/STrackerBot.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem/Public/NavigationSystem.h"
#include "NavigationSystem/Public/NavigationPath.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"
#include "SHealthComponent.h"
#include "SCharacter.h"
#include "Components/AudioComponent.h"

/**
* Tracker Bot AI cpp class
*/

// Sets default values
ASTrackerBot::ASTrackerBot()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Mesh Comp setup
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCanEverAffectNavigation(false);
	MeshComp->SetSimulatePhysics(true);
	RootComponent = MeshComp;

	// Bot Health setup
	BotHealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("BotHealthComp"));
	BotHealthComp->OnHealthChanged.AddDynamic(this, &ASTrackerBot::HandleTakeDamage);

	// Sphere collsion for player sensing setup
	PlayerSenseSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PlayerSenseSphere"));
	PlayerSenseSphere->SetSphereRadius(200);
	PlayerSenseSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PlayerSenseSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	PlayerSenseSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	PlayerSenseSphere->SetupAttachment(RootComponent);

	// Audio Component setup
	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	AudioComponent->SetupAttachment(RootComponent);

	bUseVelocityChange = true;
	bExploded = false;
	bStartedSelfDestruction = false;
	MovementForce = 400;
	RequiredDistanceToTarget = 150;
	
	ExplosionDamage = 40;
	ExplosionRadius = 200;
	PowerLevel = 0;
	MaxPowerLevel = 4;
	
	SelfDamageInterval = 0.25f;
}

/**
* On Begin Play
*/
void ASTrackerBot::BeginPlay()
{
	Super::BeginPlay();

	if (Role == ROLE_Authority) {
		// Initial move to
		NextPathPoint = GetNextPathPoint();
	}

	// Create material instance
	if (MaterialInst == nullptr) {
		MaterialInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}

	GetWorldTimerManager().SetTimer(TimerHandel_BoostPowerLevel, this, &ASTrackerBot::BoostPowerLevel, 1.0f, true, 0.0f);
}

/**
 * Finds returns the next patrol point
 *
 * @return Patrol Point Location
 */
FVector ASTrackerBot::GetNextPathPoint()
{
	ACharacter* PlayerPawn = UGameplayStatics::GetPlayerCharacter(this, 0);

	if (PlayerPawn == nullptr) {
		return GetActorLocation();
	}
	
	// Sets point on navigation volume
	UNavigationPath* NavPath = UNavigationSystemV1::FindPathToActorSynchronously(this, GetActorLocation(), PlayerPawn);

	if (NavPath && NavPath->PathPoints.Num() > 1) {
		return NavPath->PathPoints[1];
	}

	// Failed to find path
	return GetActorLocation();
}

/**
* When bot is damaged 
*
* @param HealthComp
* @param Health
* @param HealthDelta
* @param DamageType
* @param InstigatedBy
* @param DamageCauser
*/
void ASTrackerBot::HandleTakeDamage(USHealthComponent * HealthComp, float Health, float HealthDelta, const UDamageType * DamageType, AController * InstigatedBy, AActor * DamageCauser)
{

	// Pulse material on damage
	if (MaterialInst) {
		MaterialInst->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);
	}

	//UE_LOG(LogTemp, Warning, TEXT("Health %s of %s"), *FString::SanitizeFloat(Health), *GetName())

		// Explode on Hitpoints = 0
		if (Health <= 0 && !bExploded) {
			SelfDestruct();
		}
}

/*
 * Self Destruct function
 */
void ASTrackerBot::SelfDestruct()
{
	bExploded = true;

	// Emitter and sound
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
	UGameplayStatics::SpawnSoundAttached(SelfDestructSound, RootComponent);

	MeshComp->SetVisibility(false, true);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Server side death function
	if (Role == ROLE_Authority) {
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);

		// Set Explosion damage on power level
		float NewExplosionDamage = ExplosionDamage + (PowerLevel * ExplosionDamage);

		UGameplayStatics::ApplyRadialDamage(this, NewExplosionDamage, GetActorLocation(), ExplosionRadius, nullptr, IgnoredActors, this, GetInstigatorController(), true);

		DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Emerald, false, 2.0f, 0, 1.0f);

		SetLifeSpan(2.0f);

		//Destroy();
	}
}

/*
 * Applies damage to self when self destructing 
 */
void ASTrackerBot::DamageSelf()
{
	UGameplayStatics::ApplyDamage(this, 20, GetInstigatorController(), this, nullptr);
}

/*
 * Patrol function
 */
void ASTrackerBot::Patrol()
{
	// Patrol route

	float DistanceToTarget = (GetActorLocation() - NextPathPoint).Size();

	if (DistanceToTarget > RequiredDistanceToTarget) {
		// Keep moving towrad target
		FVector ForceDirection = NextPathPoint - GetActorLocation();
		ForceDirection.Normalize();
		ForceDirection *= MovementForce;

		MeshComp->AddForce(ForceDirection, NAME_None, bUseVelocityChange);

		DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + ForceDirection, 32, FColor::Yellow, false, 2.0f, 1.0f, 1.0f);
	}
	else {
		NextPathPoint = GetNextPathPoint();
	}

	DrawDebugSphere(GetWorld(), NextPathPoint, 20, 12, FColor::Yellow, false, 2.0f, 1.0f, 1.0f);
}

/*
 * Rool sound effect
 */
void ASTrackerBot::PlayRollSound()
{
	float Speed = GetVelocity().Size();

	Speed = FMath::GetMappedRangeValueClamped(FVector2D(10.0f, 1000.0f), FVector2D(0.1f, 2.5f), Speed);

	AudioComponent->SetVolumeMultiplier(Speed);
}

void ASTrackerBot::BoostPowerLevel()
{
	TArray<AActor*> TempTrackerBots;

	PlayerSenseSphere->GetOverlappingActors(TempTrackerBots, TSubclassOf<ASTrackerBot>());

	int NumOfBots = TempTrackerBots.Num() - 1;
	
	SetPowerLevel(NumOfBots);

	if (MaterialInst) {
		float PowerLevelAlpha = (float)PowerLevel/(float)MaxPowerLevel;
		MaterialInst->SetScalarParameterValue("PowerLevelAlpha", PowerLevelAlpha);
	}

}

// Called every frame
void ASTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Role == ROLE_Authority && !bExploded) {
		Patrol();

		PlayRollSound();
	}
}


void ASTrackerBot::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (!bStartedSelfDestruction && !bExploded) {

		ASCharacter* PlayerPawn = Cast<ASCharacter>(OtherActor);

		if (PlayerPawn) {
			// We overlapped with a player
			if (Role == ROLE_Authority) {
				// Start self destuction sequence
				GetWorldTimerManager().SetTimer(TimerHandel_SelfDamage, this, &ASTrackerBot::DamageSelf, SelfDamageInterval, true, 0.0f);
			}

			bStartedSelfDestruction = true;

			UGameplayStatics::SpawnSoundAttached(SelfDestructTimerSound,RootComponent);
		}
	}
}

void ASTrackerBot::SetPowerLevel(int PowerLevel)
{
	this->PowerLevel = FMath::Clamp(PowerLevel, 0, MaxPowerLevel);
}




