// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h"
#include "PlayerCharacter.generated.h"

class AProjectileBase;
class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class UAsyncRootMovement;
struct FInputActionInstance;

/**
 * 
 */
UCLASS()
class SIMPLEPLATFORMER_API APlayerCharacter : public ACharacterBase
{
	GENERATED_BODY()

public:
	APlayerCharacter();

	/* Getters
	 * ~ BEGIN */
	UFUNCTION(BlueprintPure)
	bool GetIsShooting() const {return bIsShooting;}

	UFUNCTION(BlueprintPure)
	bool GetIsStunned() const {return bIsStunned;}

	UFUNCTION(BlueprintPure)
	bool GetIsWallSliding() const {return bIsWallSliding;}

	UFUNCTION(BlueprintPure)
	bool GetIsSliding() const {return bIsSliding;}

	/* ~ END */
	
	void ReplenishEnergyCharge(UClass* ProjectileClass);
	
protected:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	virtual void OnWalkingOffLedge_Implementation(const FVector& PreviousFloorImpactNormal, const FVector& PreviousFloorContactNormal, const FVector& PreviousLocation, float TimeDelta) override;

	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnDefeated();
	
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	
private:
	UPROPERTY(VisibleAnywhere, Category="Components")
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, Category="Components")
	UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, Category="Components")
	USceneComponent* ProjectileSpawnPoint;

	UPROPERTY(VisibleAnywhere, Category="Components")
	USceneComponent* ProjectileWallSlideSpawnPoint;

	UPROPERTY()
	UCharacterMovementComponent* MovementComponent;
	
	UPROPERTY(EditAnywhere, Category="Input")
	UInputMappingContext* DefaultMappingContext;
	
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* ShootAction;

	UPROPERTY(EditAnywhere, Category="Combat")
	TSubclassOf<AProjectileBase> BaseProjectileClass;

	UPROPERTY(EditAnywhere, Category="Combat")
	TSubclassOf<AProjectileBase> PartialChargedProjectileClass;

	UPROPERTY(EditAnywhere, Category="Combat")
	TSubclassOf<AProjectileBase> FullChargedProjectileClass;

	UPROPERTY(EditAnywhere, Category="Combat")
	float RespawnTimer = 2.f;
	
	UPROPERTY(EditAnywhere, Category="Combat")
	float PartialChargeThreshold = 0.8f;

	UPROPERTY(EditAnywhere, Category="Combat")
	float FullChargeThreshold = 1.8f;

	UPROPERTY(EditAnywhere, Category="Combat")
	float ChargeFlashInterval = 0.2f;
	
	float ChargeFlashCounter;
	
	UPROPERTY(EditAnywhere, Category="Combat")
	TMap<TSubclassOf<AProjectileBase>, int32> ProjectileToEnergyCost;
	
	UPROPERTY(VisibleAnywhere, Category="Combat")
	int32 MaxEnergyCharges = 3.f;
	
	UPROPERTY(VisibleAnywhere, Category="Combat")
	int32 EnergyCharges;

	// Damage taken when overlapping another enemy
	UPROPERTY(EditAnywhere, Category="Combat")
	float OverlapDamage = 1.f;

	// How many seconds should the player stay invincible
	UPROPERTY(EditAnywhere, Category="Combat")
	float InvincibilityTime = 1.5f;

	// At what force should the knockback horizontal be applied
	UPROPERTY(EditAnywhere, Category="Combat")
	float KnockbackPowerHorizontal = 1200.f;

	// At what force should the knockback vertical be applied
	UPROPERTY(EditAnywhere, Category="Combat")
	float KnockbackPowerVertical = 800.f;

	// How much (in seconds) should the player stay stunned after taking damage
	UPROPERTY(EditAnywhere, Category="Combat")
	float StunDuration = 0.4;

	// How many units (cm) should the player start wall sliding
	UPROPERTY(EditAnywhere, Category="Combat")
	float WallSlideDistance = 40.f;

	UPROPERTY(EditAnywhere, Category="Combat")
	float WallSlideDropSpeed = -100.f;

	UPROPERTY(EditAnywhere, Category="Combat")
	float WallSlideXPower = 1000.f;

	UPROPERTY(EditAnywhere, Category="Combat")
	float WallSlideZPower = 1200.f;

	UPROPERTY(EditAnywhere, Category="Combat")
	float GroundSlidePower = 1000.f;

	UPROPERTY(EditAnywhere, Category="Combat")
	float GroundSlideTime = 0.3f;
	
	UPROPERTY(VisibleAnywhere, Category="Combat")
	bool bIsWallSliding = false;
	
	float LateralFriction = 50.f;
	
	// In what rate should sprite flicker
	UPROPERTY(EditAnywhere, Category="Sprite")
	float SpriteFlickerRate = 0.12f;

	UPROPERTY()
	UAsyncRootMovement* SlideAction;

	UPROPERTY()
	class APlatformerGameMode* GameMode;
	
	bool bIsSpriteVisible = true;
	bool bIsShooting = false;
	bool bIsStunned = false;
	bool bGotHitDuringCharged = false;
	bool bIsSliding = false;
	bool bIsFacingDirection;
	
	FTimerHandle Timer;
	FTimerHandle StunTimer;
	FTimerHandle FlickerTimer;
	FTimerHandle WallJumpTimer;
	
	void Move(const FInputActionInstance& Instance);

	bool TrySlide();
	void StartJump();
	void StopJump();
	void WallJump();
	bool CanWallSlide();
	void Slide();
	
	void Shoot(const FInputActionInstance& Instance);
	void ChargeFlashEffect(const FInputActionInstance& Instance);
	void SpawnProjectile(UClass* ProjectileClass);
	
	void JumpToStunAnimation();
	void KnockBack(AActor* DamageCauser);
	
	/* Functions used by timers
	 * ~ Begin */

	// Toggles the bIsShooting to true and then toggles it back to false after 0.4 seconds for animation purposes
	UFUNCTION()
	void ToggleIsShooting();

	UFUNCTION()
	void ResetLateralFriction();

	/**
	 * @brief Triggers the invincibility of the player after taken damage or some other cause
	 */UFUNCTION()
	void TriggerInvincibility();

	UFUNCTION()
	void ToggleSpriteFlicker();

	UFUNCTION()
	void ToggleIsStunned();

	UFUNCTION()
	void ToggleIsSliding();

	UFUNCTION()
	void DestroyAndRespawnCharacter();
	/* ~End */
};
