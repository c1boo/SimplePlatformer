// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Vitality.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SIMPLEPLATFORMER_API UVitality : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UVitality();

	void ReceiveDamage(float DamageAmount);

	bool GetIsDefeated() const {return bIsDefeated;}
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY(EditAnywhere, Category="Combat")
	float MaxHealth = 10.f;

	UPROPERTY(VisibleAnywhere, Category="Combat")
	float Health;

	UPROPERTY(VisibleAnywhere, Category="Combat")
	bool bIsDefeated = false;

	
};
