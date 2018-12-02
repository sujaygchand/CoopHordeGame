// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWeapon.generated.h"

class USkeletalMeshComponent;
class UDamageType;
class UParticleSystem;
class UCameraShake;

// Contains information of a single hit scan trace
USTRUCT()
struct FHitScanTrace {
	GENERATED_BODY()

public:

	UPROPERTY()
	FHitResult Hit;

	UPROPERTY()
	TEnumAsByte<EPhysicalSurface> SurfaceType;

	UPROPERTY()
	FVector_NetQuantize TraceEnd;

	UPROPERTY()
	FVector_NetQuantize TraceEndPoint;

	UPROPERTY()
	FVector_NetQuantize EyeLocation;

};

UCLASS()
class COOPHORDEGAME_API ASWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASWeapon();

	// Fire gun stop function
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void StartFire();

	// Fire gun stop function
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void StopFire();

	// Reload gun function
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void ReloadAmmo();
	
protected:

	virtual void BeginPlay() override;

	/** Gun Mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* MeshComp;

	// Detects if object can take damage
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<UDamageType> DamageType;

	// Camera Shake effect
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<UCameraShake> FireCamShake;

	// The fire effect on muzzle
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* MuzzleEffect;

	// On impact effect
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* DefaultImpactEffect;

	// On impact effect
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* FleshImpactEffect;

	// Bullet trail effect
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* BulletTrailEffect;

	// Muzzle Socket Name
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName MuzzleSocketName;

	// Tracer target name
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName TracerTargetName;

	FTimerHandle TimerHandle_TimeBetweenShots;

	// Current ammo
	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	int32 CurrentAmmo;

	// Ammo size
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	int32 AmmoSize;

	// Damage
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float BaseDamage;

	float LastFireTime;
	
	// RPM
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	float FireRate;

	float TimeBetweenShots;

	// Bullet spread
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	float BulletSpread;

	UPROPERTY(ReplicatedUsing=OnRep_HitScanTrace)
	FHitScanTrace HitScanTrace;
	
	// Fire gun function
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void Fire();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire();

	UFUNCTION()
	void OnRep_HitScanTrace();
	
	void ApplyDamage(EPhysicalSurface SurfaceType, AActor* HitActor, FVector ShotDirection, FHitResult Hit, AActor* Owner);

	void CheckSurfaceType(EPhysicalSurface SurfaceType, FHitResult Hit, FVector TrailEndPoint);

	void PlayFireEffect(FVector EyeLocation, FVector TraceEnd, FVector TrailEndPoint);
	
};
