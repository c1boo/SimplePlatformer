// Fill out your copyright notice in the Description page of Project Settings.


#include "PlatformerGameMode.h"

#include "Kismet/GameplayStatics.h"

void APlatformerGameMode::BeginPlay()
{
	Super::BeginPlay();

	UGameplayStatics::GetPlayerCameraManager(this, 0)->StartCameraFade(
    			1, 0.f,
    			1.f, FColor::Black,
    			false, true);
}

void APlatformerGameMode::RespawnPlayer()
{
	UGameplayStatics::GetPlayerController(this, 0)->RestartLevel();
}
