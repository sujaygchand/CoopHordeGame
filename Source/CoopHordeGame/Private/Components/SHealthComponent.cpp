// Fill out your copyright notice in the Description page of Project Settings.

#include "SHealthComponent.h"
#include "Net/UnrealNetwork.h"
#include "SHordeGameMode.h"
#include "SCharacter.h"

// Sets default values for this component's properties
USHealthComponent::USHealthComponent()
{
	DefaultHealth = 100;
	bIsDead = false;
	bIsDeadDoOnce = true;

	TeamNumber = 255;

	SetIsReplicated(true);
}


// Called when the game starts
void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// Only hook if we are the server
	if (GetOwnerRole() == ROLE_Authority) {
		
		AActor* Owner = GetOwner();
		if (Owner) {
			Owner->OnTakeAnyDamage.AddDynamic(this, &USHealthComponent::HandleTakeAnyDamage);
		}
	}

	Health = DefaultHealth;

	//SmartOwner = bIsSmartOwner ? Cast<ASCharacter>(GetOwner()) : nullptr;
	
}

void USHealthComponent::HandleTakeAnyDamage(AActor * DamagedActor, float Damage, const UDamageType * DamageType, AController * InstigatedBy, AActor * DamageCauser)
{
	/*/
	if (bIsSmartOwner) {
		if (SmartOwner) {
			if (IsFriendly(SmartOwner, InstigatedBy->GetPawn())) {
				return;
			}
		}
	} */
		
	if (Damage <= 0.0f || bIsDead) {
		return;
	}

	if (DamageCauser != DamagedActor && IsAllied(DamagedActor, DamageCauser)) {
		return;
	}

	Health = FMath::Clamp(Health - Damage, 0.0f, DefaultHealth);

	UE_LOG(LogTemp, Log, TEXT("Health: %s"), *FString::SanitizeFloat(Health))

	bIsDead = Health <= 0.0f;

	OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);

	if (bIsDead && bIsDeadDoOnce) {
		ASHordeGameMode* GameMode = Cast<ASHordeGameMode>(GetWorld()->GetAuthGameMode());

		if (GameMode) {
			bIsDeadDoOnce = false;
			GameMode->OnActorKilled.Broadcast(GetOwner(), DamageCauser, InstigatedBy);
		}
	}

}

void USHealthComponent::OnRep_Health(float OldHealth)
{
	float Damage = Health - OldHealth;

	OnHealthChanged.Broadcast(this, Health, Damage, nullptr, nullptr, nullptr);
}

void USHealthComponent::Heal(float HealAmount)
{
	if (HealAmount <= 0.0f || Health <= 0.0f) {
		return;
	}

	Health = FMath::Clamp(Health + HealAmount, 0.0f, DefaultHealth);

	UE_LOG(LogTemp, Log, TEXT("Health: %s (+%s)"), *FString::SanitizeFloat(Health), *FString::SanitizeFloat(HealAmount))

		OnHealthChanged.Broadcast(this, Health, -HealAmount, nullptr, nullptr, nullptr);
}

float USHealthComponent::GetHealth() const
{
	return Health;
}

// If the owner has allies, stop friendly fire
bool USHealthComponent::IsFriendly(AActor* Owner, AActor* Attacker)
{
	if (Owner && Attacker) {

		ASCharacter* TempOwner = Cast<ASCharacter>(Owner);
		ASCharacter* AttackingPawn = Cast<ASCharacter>(Attacker);

		if (AttackingPawn && TempOwner) {

			for (TSubclassOf<APawn> TempPawn : TempOwner->Allies) {

				//UE_LOG(LogTemp, Warning, TEXT("Friend: %s"), *TempPawn->GetName())

				//UE_LOG(LogTemp, Warning, TEXT("Attacker: %s"), *AttackingPawnClass->GetName())

				if (TempPawn == AttackingPawn->GetClass()) {
					return true;
				}
			}
		}

		return false;
	}
	return false;
}

bool USHealthComponent::IsAllied(AActor* ActorA, AActor* ActorB)
{
	//Assume friendly
	if (ActorA == nullptr || ActorB == nullptr) {
		return true;
	}

	USHealthComponent* HealthCompA = Cast<USHealthComponent>(ActorA->GetComponentByClass(USHealthComponent::StaticClass()));
	USHealthComponent* HealthCompB = Cast<USHealthComponent>(ActorB->GetComponentByClass(USHealthComponent::StaticClass()));

	//Assume not friendly
	if (HealthCompA == nullptr || HealthCompB == nullptr) {
		return true;
	}

	return HealthCompA->TeamNumber == HealthCompB->TeamNumber;
}

void USHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {

	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USHealthComponent, Health);
}
