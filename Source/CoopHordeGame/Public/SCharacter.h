// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UChildActorComponent;
class ASWeapon;
class USHealthComponent;


/**
* The player character header class
*/
UCLASS()
class COOPHORDEGAME_API ASCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float value);

	void MoveRight(float value);

	void BeginCrouch();

	void EndCrouch();

	void BeginZoom();

	void EndZoom();

	void ChangeWeapon(float value);

	void StartFire();

	void StopFire();

	void ReloadWeapon();

	UFUNCTION()
	void OnHealthChanged(USHealthComponent* HealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	// Camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* CameraComp;

	// Spring arm
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* SpringArmComp;

	// Gun Slots
	UPROPERTY(AdvancedDisplay, EditAnywhere, BlueprintReadWrite, Category = "Components")
	UChildActorComponent* GunSlot;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	USHealthComponent* PlayerHealthComp;

	// Current Weapon
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category = "Weapon")
	ASWeapon* CurrentWeapon;

	// List of Weapons
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	TArray<TSubclassOf<ASWeapon>> Weapons;

	bool bCanZoom;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite)
	bool bDied;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	float ZoomedFOV;

	/* Default FOV */
	float DefaultFOV;

	UPROPERTY(EditDefaultsOnly, Category = "Player", meta = (ClampMin = 0.1f, ClampMax = 100.0f))
	float ZoomInterpSpeed;

	/* Current Weapon Index */
	int32 CurrentWeaponIndex = 0;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual FVector GetPawnViewLocation() const override;

	// Checks if character is crouching 
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bIsCrouching = false;
	
	void CheckCurrentWeapon();
};
