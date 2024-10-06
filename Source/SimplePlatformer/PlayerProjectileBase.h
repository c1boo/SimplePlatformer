// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectileBase.h"
#include "PlayerProjectileBase.generated.h"

/**
 * 
 */
UCLASS()
class SIMPLEPLATFORMER_API APlayerProjectileBase : public AProjectileBase
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	
	virtual void Tick(float DeltaSeconds) override;

	virtual void Destroyed() override;

private:
	
	UPROPERTY()
	class APlayerCharacter* PlayerCharacter;

	UPROPERTY()
	APlayerController* PlayerController;
	
};
