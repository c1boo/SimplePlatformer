// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EFaction.h"
#include "GameFramework/Actor.h"
#include "ProjectileBase.generated.h"

class AVFXBase;
class UPaperFlipbookComponent;
class USphereComponent;
class UProjectileMovementComponent;

UCLASS()
class SIMPLEPLATFORMER_API AProjectileBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectileBase();

	UPROPERTY(EditAnywhere, Category="Combat")
	EFaction Faction;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	void PlayHitVfx() const;

private:
	UPROPERTY(VisibleAnywhere, Category="Components")
	USphereComponent* Sphere;

	UPROPERTY(VisibleAnywhere, Category="Components")
	UPaperFlipbookComponent* PaperFlipBook;
	
	UPROPERTY(VisibleAnywhere, Category="Components")
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditAnywhere, Category="Sprite")
	TSubclassOf<AVFXBase> HitVFX;

	UPROPERTY(EditAnywhere, Category="Variables")
	float Damage = 1.f;
	
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bSweep, const FHitResult& SweepResult);
};
