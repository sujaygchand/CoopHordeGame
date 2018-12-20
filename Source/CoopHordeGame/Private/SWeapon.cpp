// Fill out your copyright notice in the Description page of Project Settings.

#include "SWeapon.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "CoopHordeGame.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"

static int32 DebugWeaponDrawing = 0;

FAutoConsoleVariableRef CVARDebugWeaponDrawing(
	TEXT("COOP.DebugWeapons"),
	DebugWeaponDrawing,
	TEXT("Draw Debug Lines for weapons"),
	ECVF_Cheat);

// Sets default values
ASWeapon::ASWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName = "Target";
	BaseDamage = 20.0f;
	FireRate = 600;
	BulletSpread = 2.0f;

	SetReplicates(true);

	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
}

void ASWeapon::BeginPlay()
{	
	Super::BeginPlay();

	TimeBetweenShots = 60 / FireRate;
}

void ASWeapon::StartFire()
{
	if (CurrentAmmo > 0) {

		float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);

		GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &ASWeapon::Fire, TimeBetweenShots, true, FirstDelay);
	}
	else {
		StopFire();
		ReloadAmmo();
		return;
	}
}

void ASWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}

// Fires gun
void ASWeapon::Fire()
{
	if (Role < ROLE_Authority) {
		ServerFire();
	}

	CurrentAmmo--;

	CurrentAmmo = FMath::Max( 0, CurrentAmmo);

	// Trace world, from pawn eyes
	
	AActor* Owner = GetOwner();
	if (Owner) {

		FVector EyeLocation;
		FRotator EyeRotation;

		// The parameters are passed by references, so the values are filled by the owner 
		Owner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FVector ShotDirection = EyeRotation.Vector();

		// Bullet spread
		float HalfRad = FMath::DegreesToRadians(BulletSpread);
		ShotDirection = FMath::VRandCone(ShotDirection, HalfRad, HalfRad);

		FVector TraceEnd = EyeLocation + (ShotDirection * 10000);

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(Owner);
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;
		QueryParams.bReturnPhysicalMaterial = true;

		FVector TrailEndPoint = TraceEnd;

		EPhysicalSurface SurfaceType;

		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams)) {

			// Blocking hit, process damage
			AActor* HitActor = Hit.GetActor();
			
			SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

			ApplyDamage(SurfaceType, HitActor, ShotDirection, Hit, Owner);

			CheckSurfaceType(SurfaceType, Hit, TrailEndPoint);

			TrailEndPoint = Hit.ImpactPoint;
		}
		
		PlayFireEffect(EyeLocation, TraceEnd, TrailEndPoint);

		if (Role == ROLE_Authority) {
			HitScanTrace.TraceEndPoint = TrailEndPoint;
			HitScanTrace.TraceEnd = TraceEnd;
			HitScanTrace.EyeLocation = EyeLocation;
			HitScanTrace.SurfaceType = SurfaceType;
			HitScanTrace.Hit = Hit;
		}

		LastFireTime = GetWorld()->TimeSeconds;
	}
}

 

void ASWeapon::ServerFire_Implementation()
{
	Fire();
}

bool ASWeapon::ServerFire_Validate()
{
	return true;
}

// TODO: Fix this function 7:12
//Play cosmetic effect
void ASWeapon::OnRep_HitScanTrace()
{
	PlayFireEffect(HitScanTrace.EyeLocation, HitScanTrace.TraceEnd, HitScanTrace.TraceEndPoint);
	CheckSurfaceType(HitScanTrace.SurfaceType, HitScanTrace.Hit ,HitScanTrace.TraceEndPoint);
}

void ASWeapon::ApplyDamage(EPhysicalSurface SurfaceType, AActor* HitActor, FVector ShotDirection, FHitResult Hit, AActor* Owner)
{

	float ActualDamage = BaseDamage;

	if (SurfaceType == SURFACE_FLESHVULNERABLE) {
		ActualDamage *= 6.0f;
	}

	UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, Owner->GetInstigatorController(), this->GetOwner(), DamageType);

}

void ASWeapon::CheckSurfaceType(EPhysicalSurface SurfaceType, FHitResult Hit, FVector TrailEndPoint)
{
	
	UParticleSystem* SelectedEffect = nullptr;
	switch (SurfaceType) {
	case SURFACE_FLESHDEFAULT:		// FleshDefault
		SelectedEffect = FleshImpactEffect;
		break;
	case SURFACE_FLESHVULNERABLE:		// FleshVulnerable
		SelectedEffect = FleshImpactEffect;
		break;
	default:
		SelectedEffect = DefaultImpactEffect;
		break;
	}

	if (SelectedEffect) {
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
	}

}

void ASWeapon::PlayFireEffect(FVector EyeLocation, FVector TraceEnd, FVector TrailEndPoint)
{
	if (DebugWeaponDrawing > 0) {
		DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::Red, false, 1.0f, 0, 4.0f);
	}

	if (MuzzleEffect) {
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
	}

	if (BulletTrailEffect) {
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

		UParticleSystemComponent* TrailComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BulletTrailEffect, MuzzleLocation);

		if (TrailComp) {
			TrailComp->SetVectorParameter(TracerTargetName, TrailEndPoint);
		}
	}

	APawn* Owner = Cast<APawn>(GetOwner());
	if (Owner) {
		APlayerController* MyController = Cast<APlayerController>(Owner->GetController());
		if (MyController) {
			MyController->ClientPlayCameraShake(FireCamShake);
		}
	}
}


void ASWeapon::ReloadAmmo()
{
	CurrentAmmo = AmmoSize;
}

void ASWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {

	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASWeapon, HitScanTrace, COND_SkipOwner);
	DOREPLIFETIME(ASWeapon, CurrentAmmo);
}


