// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyBase.h"

#include "VFXBase.h"
#include "Kismet/GameplayStatics.h"

AEnemyBase::AEnemyBase()
{
	Faction = EFaction::Enemy;
}

void AEnemyBase::Destroyed()
{
	Super::Destroyed();

	if (DeathVFX && GetWorld()->IsGameWorld())
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		GetWorld()->SpawnActor<AVFXBase>(DeathVFX, GetActorLocation(), GetActorRotation(), Params);
	}
}
