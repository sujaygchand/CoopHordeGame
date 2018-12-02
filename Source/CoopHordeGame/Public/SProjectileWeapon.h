// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SWeapon.h"
#include "SProjectileWeapon.generated.h"

/**
 * 
 */
UCLASS()
class COOPHORDEGAME_API ASProjectileWeapon : public ASWeapon
{
	GENERATED_BODY()
	
public:
	virtual void Fire() override;
	
protected:
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AActor> ProjectileClass;
};
