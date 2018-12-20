// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnHealthChangedSignature, USHealthComponent*, HealthComp, float, Health, float, HealthDelta, const class UDamageType*, DamageType, class AController*, InstigatedBy, AActor*, DamageCauser);

class ASCharacter;

UCLASS( ClassGroup=(COOP), meta=(BlueprintSpawnableComponent) )
class COOPHORDEGAME_API USHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USHealthComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	bool bIsDead;

	bool bIsDeadDoOnce;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bIsSmartOwner;

	ASCharacter* SmartOwner;

	UPROPERTY(ReplicatedUsing=OnRep_Health, BlueprintReadOnly)
	float Health;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float DefaultHealth;

	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);
	
	UFUNCTION()
	void OnRep_Health(float OldHealth);

public:
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHealthChangedSignature OnHealthChanged;

	UFUNCTION(BlueprintCallable)
	void Heal(float HealAmount);

	float GetHealth() const;

	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "HealthComponent")
	uint8 TeamNumber;

	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "HealthComponent")
	static bool IsFriendly(AActor* Owner ,AActor* Attacker);

	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "HealthComponent")
	static bool IsAllied(AActor* ActorA, AActor* ActorB);
};
