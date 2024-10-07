// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperZDCharacter.h"
#include "EFaction.h"
#include "CharacterBase.generated.h"

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE(FDefeatedDelegate);

/**
 * 
 */
UCLASS()
class SIMPLEPLATFORMER_API ACharacterBase : public APaperZDCharacter
{
	GENERATED_BODY()

public:
	ACharacterBase();

	UPROPERTY(EditAnywhere, Category="Combat")
	EFaction Faction;

protected:

	FDefeatedDelegate Defeated;
	
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	
	/* ~Begin */
	/* Parameters and functions for the flashing effect when the character takes a hit*/
	// Flashes the sprite by the amount specified
	UFUNCTION()
	void FlashSprite(float FlashAmount, FColor FlashColor);

	// Flashes the sprite then turns it off
	UFUNCTION()
	void PlayFlashEffect(float FlashRate, float FlashAmount, FColor FlashColor);
	/* ~End */

private:
	UPROPERTY(VisibleAnywhere, Category="Components")
	class UVitality* Vitality;

	
	/* ~Begin */
	/* Parameters and functions for the flashing effect when the character takes a hit*/
	
	UPROPERTY()
	UMaterialInstanceDynamic* MaterialInstance;
	
	// How much should the character flash when takes damage
	UPROPERTY(EditAnywhere, Category="Sprite")
	float FlashMultiplier = 1.f;

	// After how many seconds should the effect wear off
	UPROPERTY(EditAnywhere, Category="Sprite")
	float FlashEffectRate = 0.1f;
	

	/* ~End */
};
