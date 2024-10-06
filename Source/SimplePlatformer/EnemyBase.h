// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h"
#include "EnemyBase.generated.h"

/**
 * 
 */
UCLASS()
class SIMPLEPLATFORMER_API AEnemyBase : public ACharacterBase
{
	GENERATED_BODY()

public:
	AEnemyBase();

private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AVFXBase> DeathVFX;
	
	virtual void Destroyed() override;
};
