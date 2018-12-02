// Fill out your copyright notice in the Description page of Project Settings.

#include "SProjectileWeapon.h"
#include "Kismet/GameplayStatics.h"


void ASProjectileWeapon::Fire() {

	// Trace world, from pawn eyes

	AActor* Owner = GetOwner();
	if (Owner && ProjectileClass) {

		FVector EyeLocation;
		FRotator EyeRotation;

		// The parameters are passed by references, so the values are filled by the owner 
		Owner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		FRotator MuzzleRotation = MeshComp->GetSocketRotation(MuzzleSocketName);

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;


		GetWorld()->SpawnActor<AActor>(ProjectileClass, MuzzleLocation, EyeRotation, SpawnParams);

	}
}


