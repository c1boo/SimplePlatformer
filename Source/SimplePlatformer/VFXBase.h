// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VFXBase.generated.h"

class UPaperFlipbookComponent;

UCLASS()
class SIMPLEPLATFORMER_API AVFXBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVFXBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(VisibleAnywhere, Category="Components")
	UPaperFlipbookComponent* PaperFlipbookComponent;

	UFUNCTION()
	void OnFinishedPlaying();
};
