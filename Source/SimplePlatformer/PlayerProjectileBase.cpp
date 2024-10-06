// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerProjectileBase.h"

#include "PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"


void APlayerProjectileBase::BeginPlay()
{
	Super::BeginPlay();
	
	PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
}

void APlayerProjectileBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (PlayerController)
	{
		FVector2d ScreenLoc;
		int32 ViewPortX, ViewPortY;
		PlayerController->ProjectWorldLocationToScreen(GetActorLocation(), ScreenLoc);
		PlayerController->GetViewportSize(ViewPortX, ViewPortY);
		if (ScreenLoc.X <= 0 || ScreenLoc.X > ViewPortX)
		{
			Destroy();
		}
	}
}

void APlayerProjectileBase::Destroyed()
{
	Super::Destroyed();
	
	PlayerCharacter = Cast<APlayerCharacter>(GetOwner());
	if (PlayerCharacter) {
		PlayerCharacter->ReplenishEnergyCharge(GetClass());
	}
}
