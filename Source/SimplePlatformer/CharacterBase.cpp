// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterBase.h"

#include "PaperFlipbookComponent.h"
#include "Vitality.h"
#include "GameFramework/CharacterMovementComponent.h"

ACharacterBase::ACharacterBase()
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	MovementComponent->SetPlaneConstraintEnabled(true);
	MovementComponent->SetPlaneConstraintNormal(FVector(0.f, 1.f, 0.f));
	MovementComponent->bUseFlatBaseForFloorChecks = true;
	MovementComponent->GravityScale = 5.5f;
	MovementComponent->JumpZVelocity = 600.f;
	MovementComponent->AirControl = 0.8f;

	Vitality = CreateDefaultSubobject<UVitality>(TEXT("Vitality Component"));
	
}

void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();

	MaterialInstance = GetSprite()->CreateDynamicMaterialInstance(0);
}

float ACharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
                                 AActor* DamageCauser)
{
	float AppliedDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	Vitality->ReceiveDamage(DamageAmount);
	if (Vitality->GetIsDefeated())
	{
		Destroy();
	}
	else
	{
		PlayFlashEffect(FlashEffectRate, FlashMultiplier, FColor::FromHex(TEXT("FFD41CFF")));
	}
	
	return AppliedDamage;
}

void ACharacterBase::FlashSprite(float FlashAmount, FColor FlashColor)
{
	MaterialInstance->SetScalarParameterValue(TEXT("FlashMultiplier"), FlashAmount);
	MaterialInstance->SetVectorParameterValue(TEXT("FlashColor"), FlashColor);
}

void ACharacterBase::PlayFlashEffect(float FlashRate, float FlashAmount, FColor FlashColor)
{
	if (!MaterialInstance) return;
	
	FTimerHandle Timer;
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUFunction(this, FName("FlashSprite"), 0.f, FlashColor);
	
	FlashSprite(FlashAmount, FlashColor);
	GetWorld()->GetTimerManager().SetTimer(Timer, TimerDelegate, FlashRate, false);
}
