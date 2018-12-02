// Fill out your copyright notice in the Description page of Project Settings.

#include "SExplosiveBarrel.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "SHealthComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ASExplosiveBarrel::ASExplosiveBarrel()
{
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetSimulatePhysics(true);
	MeshComp->SetCollisionObjectType(ECC_PhysicsBody);
	RootComponent = MeshComp;

	RFComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("RFComp"));
	RFComp->SetupAttachment(RootComponent);
	RFComp->Radius = 250;
	RFComp->bImpulseVelChange = true;
	RFComp->bAutoActivate = false;
	RFComp->bIgnoreOwningActor = true;
	

	BarrelHealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("BarrelHealthComp"));
	BarrelHealthComp->OnHealthChanged.AddDynamic(this, &ASExplosiveBarrel::OnHealthChanged);

	ExplosionImpluse = 400;
	bExploded = false;

	SetReplicates(true);
	SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void ASExplosiveBarrel::BeginPlay()
{
	Super::BeginPlay();
	
	
}

void ASExplosiveBarrel::OnHealthChanged(USHealthComponent * HealthComp, float Health, float HealthDelta, const UDamageType * DamageType, AController * InstigatedBy, AActor * DamageCauser)
{
	if (Health <= 0.0f && !bExploded) {
		bExploded = true;

		OnRep_Exploded();

		// Boost the barrel upwards
		FVector BoostIntensity = FVector::UpVector * ExplosionImpluse;
		MeshComp->AddImpulse(BoostIntensity, NAME_None, true);

		RFComp->Activate(true);
		RFComp->FireImpulse();
	}
}

void ASExplosiveBarrel::OnRep_Exploded()
{
	// Play VFX and set material
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionParticle, GetActorLocation());
	MeshComp->SetMaterial(0, ExplodedMaterial);
}

void ASExplosiveBarrel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {

	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASExplosiveBarrel, bExploded);
}

