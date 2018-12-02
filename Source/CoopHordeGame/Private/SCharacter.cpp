// Fill out your copyright notice in the Description page of Project Settings.

#include "SCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Components/ChildActorComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "SWeapon.h"
#include "CoopHordeGame.h"
#include "Runtime/Core/Public/Misc/OutputDeviceNull.h"
#include "SHealthComponent.h"
#include "Net/UnrealNetwork.h"

/**
* The player character header class
*/

// Sets default values
ASCharacter::ASCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Spring Arm setup
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->SetupAttachment(RootComponent);


	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

	// Health Comp setup
	PlayerHealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("PlayerHealthComp"));

	// Camera Comp setup
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	// Gun Slot setup
	GunSlot = CreateDefaultSubobject<UChildActorComponent>(TEXT("GunSlot"));
	GunSlot->CreateChildActor();
	GunSlot->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, "WeaponSocket");
	GunSlot->SetChildActorClass(ASWeapon::StaticClass());

	ZoomedFOV = 65.0f;
	ZoomInterpSpeed = 20.0f;

	bDied = false;

}

/**
 * On Begin Play
*/
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();

	DefaultFOV = CameraComp->FieldOfView;

	CheckCurrentWeapon();

	//Binds the link
	PlayerHealthComp->OnHealthChanged.AddDynamic(this, &ASCharacter::OnHealthChanged);

}

/**
 * Player input setup
*/
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Axis Setup
	PlayerInputComponent->BindAxis("MoveForward", this, &ASCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASCharacter::MoveRight);
	PlayerInputComponent->BindAxis("LookUp", this, &ASCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &ASCharacter::AddControllerYawInput);
	PlayerInputComponent->BindAxis("ChangeWeapon", this, &ASCharacter::ChangeWeapon);

	// Button Setup
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASCharacter::BeginCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASCharacter::EndCrouch);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Zoom", IE_Pressed, this, &ASCharacter::BeginZoom);
	PlayerInputComponent->BindAction("Zoom", IE_Released, this, &ASCharacter::EndZoom);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ASCharacter::StopFire);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ASCharacter::ReloadWeapon);

}

/**
* Moves Forward
*/
void ASCharacter::MoveForward(float value)
{
	AddMovementInput(GetActorForwardVector() *value);
}

/**
* Moves Right
*/
void ASCharacter::MoveRight(float value)
{
	AddMovementInput(GetActorRightVector() *value);
}

/**
 * Start Crouch
*/
void ASCharacter::BeginCrouch()
{
	Crouch();
	//UE_LOG(LogTemp, Warning, TEXT("Crouched"))
	bIsCrouching = true;
}

/**
 * End Crouch
*/
void ASCharacter::EndCrouch()
{
	UnCrouch();
	//UE_LOG(LogTemp, Warning, TEXT("UNCrouched"))
	bIsCrouching = false;
}

/**
* Start Zoom
*/
void ASCharacter::BeginZoom()
{
	bCanZoom = true;
}

/**
* End Zoom
*/
void ASCharacter::EndZoom()
{
	bCanZoom = false;
}

/**
* Weapon swap function
*/
void ASCharacter::ChangeWeapon(float value)
{
	// Does nothing, if player has only 1 weapon
	if (Weapons.Num() <= 1) {
		return;
	}

	if (value != 0) {
		CurrentWeaponIndex += value;
		if (CurrentWeaponIndex >= Weapons.Num()) {
			CurrentWeaponIndex = 0;
		}
		else if (CurrentWeaponIndex < 0) {
			CurrentWeaponIndex = Weapons.Num() - 1;
		}

		GunSlot->SetChildActorClass(Weapons[CurrentWeaponIndex]);
		CheckCurrentWeapon();
	}
}

/*
 * Fires automatic gun 
 */
void ASCharacter::StartFire()
{
	if (CurrentWeapon) {
		CurrentWeapon->StartFire();
	}
}

/*
 * Stops gun fire
 */
void ASCharacter::StopFire()
{
	if (CurrentWeapon) {
		CurrentWeapon->StopFire();
	}
}

/*
 * Reload gun
 */
void ASCharacter::ReloadWeapon()
{
	if (CurrentWeapon) {
		CurrentWeapon->ReloadAmmo();
	}
}

/*
 * Triggers deaths when health is zero
 */
void ASCharacter::OnHealthChanged(USHealthComponent* HealthComp, float Health, float HealthDelta,
	const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Health <= 0.0f && !bDied) {
		// Die!
		bDied = true;

		GetMovementComponent()->StopMovementImmediately();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		DetachFromControllerPendingDestroy();
		SetLifeSpan(10.0f);
	}
}

/*
 * On Tick
 */
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float CurrentFOV = bCanZoom ? ZoomedFOV : DefaultFOV;

	float NewFOV = FMath::FInterpTo(CameraComp->FieldOfView, CurrentFOV, DeltaTime, ZoomInterpSpeed);

	CameraComp->SetFieldOfView(NewFOV);

}


/*
 * Gets the player view for firing
 */
FVector ASCharacter::GetPawnViewLocation() const
{
	if (CameraComp) {
		return CameraComp->GetComponentLocation();
	}

	return Super::GetPawnViewLocation();
}

/*
* Checks the current weapon
*/
void ASCharacter::CheckCurrentWeapon()
{
	CurrentWeapon = Cast<ASWeapon>(GunSlot->GetChildActor()) ? Cast<ASWeapon>(GunSlot->GetChildActor()) : nullptr;

	if (CurrentWeapon) {
		CurrentWeapon->SetOwner(this);
	}
}

/*
* Variable replication over the network
*/
void ASCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {

	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASCharacter, CurrentWeapon);
	DOREPLIFETIME(ASCharacter, bDied);
}