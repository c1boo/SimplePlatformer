// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"

#include "EnemyBase.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "PaperFlipbookComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"
#include "PaperZDAnimationComponent.h"
#include "PaperZDAnimInstance.h"
#include "PlayerProjectileBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AsyncRootMovement.h"
#include "PlatformerGameMode.h"

APlayerCharacter::APlayerCharacter()
{
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetRelativeRotation(FRotator(-15.f, -90.f, 0.f));
	SpringArm->TargetArmLength = 500.f;
	SpringArm->bDoCollisionTest = false;
	SpringArm->SetAbsolute(false, true, false);
	SpringArm->SetupAttachment(RootComponent);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	ProjectileSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Projectile Spawn Point"));
	ProjectileSpawnPoint->SetupAttachment(RootComponent);

	ProjectileWallSlideSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Projectile Wall Slide Spawn Point"));
	ProjectileWallSlideSpawnPoint->SetupAttachment(RootComponent);
	
	JumpMaxHoldTime = 0.4;
	MovementComponent = GetCharacterMovement();
	if (MovementComponent)
	{
		MovementComponent->AirControl = 1.f;
		MovementComponent->FallingLateralFriction = 50.f;	
	}
	
	Faction = EFaction::Player;
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Access the player controller to get the input subsystem
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &APlayerCharacter::OnOverlap);
	EnergyCharges = MaxEnergyCharges;

	if (MovementComponent)
	{
		LateralFriction = MovementComponent->FallingLateralFriction;
	}

	GameMode = Cast<APlatformerGameMode>(GetWorld()->GetAuthGameMode());
	Defeated.BindDynamic(this, &APlayerCharacter::OnDefeated);
}

void APlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	bIsWallSliding = CanWallSlide();

	if (bIsWallSliding)
	{
		FVector NewVelocity = MovementComponent->Velocity;
		NewVelocity.Z = FMath::Max(NewVelocity.Z, WallSlideDropSpeed);
		MovementComponent->Velocity = NewVelocity;
	}
	
	if (bIsSliding && !bIsFacingDirection)
	{
		ToggleIsSliding();
	}
}

void APlayerCharacter::OnWalkingOffLedge_Implementation(const FVector& PreviousFloorImpactNormal,
	const FVector& PreviousFloorContactNormal, const FVector& PreviousLocation, float TimeDelta)
{
	Super::OnWalkingOffLedge_Implementation(PreviousFloorImpactNormal, PreviousFloorContactNormal, PreviousLocation,
	                                        TimeDelta);

	ToggleIsSliding();
}

void APlayerCharacter::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                 UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bSweep, const FHitResult& SweepResult)
{
	if (Cast<AEnemyBase>(OtherActor) && OtherComponent->IsA(UCapsuleComponent::StaticClass()))
	{
		UGameplayStatics::ApplyDamage(this, OverlapDamage, nullptr, OtherActor, UDamageType::StaticClass());
	}
}

void APlayerCharacter::OnDefeated()
{
	if (GameMode)
	{
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SpringArm->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		DisableInput(UGameplayStatics::GetPlayerController(this, 0));
		UGameplayStatics::GetPlayerCameraManager(this, 0)->StartCameraFade(
			0.f, 1.f,
			RespawnTimer, FColor::Black,
			false, true);
		GetWorldTimerManager().SetTimer(Timer, this, &APlayerCharacter::DestroyAndRespawnCharacter, RespawnTimer);
	}
	else
	{
		UGameplayStatics::GetPlayerController(this, 0)->RestartLevel();
	}
}

float APlayerCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
								   AActor* DamageCauser)
{
	const float AppliedDamage =  Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	bGotHitDuringCharged = true;
	ToggleIsSliding();
	TriggerInvincibility();
	JumpToStunAnimation();
	KnockBack(DamageCauser);
	
	return AppliedDamage;
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (Input)
	{
		Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
		
		Input->BindAction(JumpAction, ETriggerEvent::Started, this, &APlayerCharacter::StartJump);
		Input->BindAction(JumpAction, ETriggerEvent::Canceled, this, &APlayerCharacter::StopJump);
		Input->BindAction(JumpAction, ETriggerEvent::Completed, this, &APlayerCharacter::StopJump);
		
		Input->BindAction(ShootAction, ETriggerEvent::Completed, this, &APlayerCharacter::Shoot);
		Input->BindAction(ShootAction, ETriggerEvent::Ongoing, this, &APlayerCharacter::ChargeFlashEffect);
	}
}

void APlayerCharacter::Move(const FInputActionInstance& Instance)
{
	if (bIsStunned) return;

	float Value = Instance.GetValue().Get<FVector2D>().X;
	bIsFacingDirection = Value * GetActorForwardVector().X > 0.f;
	
	AddMovementInput(FVector(1.f, 0.f, 0.f), Value);

	// Rotate to the moving direction
	if (Value > 0 && !bIsSliding)
	{
		Controller->SetControlRotation(FRotator::ZeroRotator);
	}
	else if (Value < 0 && !bIsSliding)
	{
		Controller->SetControlRotation(FRotator(0.f, 180.f, 0.f));
	}
}

bool APlayerCharacter::TrySlide()
{
	//TODO: Implement try slide
	return false;
}

void APlayerCharacter::StartJump()
{
	if (bIsStunned) return;

	if (bIsWallSliding)
	{
		WallJump();	
	}
	else if (MovementComponent->IsMovingOnGround())
	{
		if (TrySlide())
		{
			Slide();
		}
		else
		{
			Jump();
			if (SlideAction) ToggleIsSliding();	
		}
	}
}

void APlayerCharacter::StopJump()
{
	StopJumping();
}

void APlayerCharacter::WallJump()
{
	MovementComponent->FallingLateralFriction = 0.f;
	FVector LaunchVelocity(0);
	LaunchVelocity.X = -GetActorForwardVector().X * WallSlideXPower;
	LaunchVelocity.Z = WallSlideZPower;
	LaunchCharacter(LaunchVelocity, true, true);
	GetWorldTimerManager().SetTimer(WallJumpTimer, this, &APlayerCharacter::ResetLateralFriction, 0.2f);
}

bool APlayerCharacter::CanWallSlide()
{
	if (MovementComponent->Velocity.Z > -50.f) return false;
	
	FVector StartLoc = GetActorLocation();
	FVector EndLoc = StartLoc + GetActorForwardVector() * WallSlideDistance;
	FHitResult Hit;
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);

	return
			GetWorld()->LineTraceSingleByObjectType(Hit, StartLoc, EndLoc, ObjectQueryParams) && bIsFacingDirection;
}

void APlayerCharacter::Slide()
{
	if (bIsSliding) return;
	
	bIsSliding = true;
	SlideAction = UAsyncRootMovement::AsyncRootMovement(
			GetWorld(),
			MovementComponent,
			GetActorForwardVector(),
			GroundSlidePower,
			GroundSlideTime,
			false,
			nullptr,
			ERootMotionFinishVelocityMode::MaintainLastRootMotionVelocity,
			FVector::ZeroVector,
			0.f,
			true);
	if (SlideAction)
	{
		SlideAction->Activate();
		SlideAction->OnComplete.AddDynamic(this, &APlayerCharacter::ToggleIsSliding);
		SlideAction->OnFail.AddDynamic(this, &APlayerCharacter::ToggleIsSliding);
	}
}

void APlayerCharacter::Shoot(const FInputActionInstance& Instance)
{
	if (EnergyCharges == 0 || bIsStunned || bIsSliding ) return;

	const float ElapsedTime = Instance.GetElapsedTime();
	if (ElapsedTime < PartialChargeThreshold)
	{
		bGotHitDuringCharged = false;
		SpawnProjectile(BaseProjectileClass);
	}
	else if (ElapsedTime < FullChargeThreshold && !bGotHitDuringCharged)
	{
		SpawnProjectile(PartialChargedProjectileClass);
	}
	else if (!bGotHitDuringCharged)
	{
		SpawnProjectile(FullChargedProjectileClass);	
	}
}

void APlayerCharacter::ChargeFlashEffect(const FInputActionInstance& Instance)
{
	if (Instance.GetElapsedTime() < PartialChargeThreshold && !bGotHitDuringCharged)
	{
		ChargeFlashCounter = 0;
		return;
	}

	// TODO: Check 37th lecture again
	ChargeFlashCounter += GetWorld()->GetDeltaSeconds();
	if (ChargeFlashCounter >= ChargeFlashInterval)
	{
		const FColor FlashColor = Instance.GetElapsedTime() < FullChargeThreshold? FColor::White : FColor::Blue;
		ChargeFlashCounter = 0;
		PlayFlashEffect(0.1f, 5.f, FlashColor);	
	}
}

void APlayerCharacter::SpawnProjectile(UClass* ProjectileClass)
{
	if (!ProjectileClass || !ProjectileToEnergyCost.Contains(ProjectileClass)) return;
	// If player doesn't have enough energy charges to spawn this projectile then early return
	if (*ProjectileToEnergyCost.Find(ProjectileClass) > EnergyCharges) return;

	ToggleIsShooting();
	EnergyCharges = FMath::Max(EnergyCharges - *ProjectileToEnergyCost.Find(ProjectileClass), 0);
	
	const FTransform SpawnTransform = bIsWallSliding?
		ProjectileWallSlideSpawnPoint->GetComponentTransform() : ProjectileSpawnPoint->GetComponentTransform();

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	APlayerProjectileBase* Projectile = GetWorld()->SpawnActor<APlayerProjectileBase>(
		ProjectileClass, SpawnTransform.GetLocation(), SpawnTransform.GetRotation().Rotator(), Params);
	if (Projectile)
	{
		Projectile->SetOwner(this);
		Projectile->SetInstigator(this);
		Projectile->Faction = Faction;
	}
	
}

void APlayerCharacter::ToggleIsShooting()
{
	bIsShooting = !bIsShooting;
	if (bIsShooting)
	{
		GetWorld()->GetTimerManager().SetTimer(Timer, this, &APlayerCharacter::ToggleIsShooting, 0.4f, false);
	}
}

void APlayerCharacter::ReplenishEnergyCharge(UClass* ProjectileClass)
{
	EnergyCharges = FMath::Min(
		EnergyCharges + *ProjectileToEnergyCost.Find(ProjectileClass),
		MaxEnergyCharges);
}

void APlayerCharacter::JumpToStunAnimation()
{
	if (UPaperZDAnimationComponent* AnimComponent = GetAnimationComponent())
	{
		if (UPaperZDAnimInstance* Instance = AnimComponent->GetAnimInstance())
		{
			Instance->JumpToNode(TEXT("JumpStun"));
			ToggleIsStunned();
			GetWorld()->GetTimerManager().SetTimer(
				StunTimer, this, &APlayerCharacter::ToggleIsStunned, StunDuration);
		}
	}
}

void APlayerCharacter::TriggerInvincibility()
{
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (!Capsule) return;
	
	if (Capsule->GetCollisionResponseToChannel(ECC_Pawn) == ECR_Overlap)
	{
		Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

		FTimerHandle InvincibilityTimer;
		GetWorld()->GetTimerManager().SetTimer(
			InvincibilityTimer,
			this,
			&APlayerCharacter::TriggerInvincibility,
			InvincibilityTime);
		
		GetWorld()->GetTimerManager().SetTimer(
			FlickerTimer,
			this,
			&APlayerCharacter::ToggleSpriteFlicker,
			SpriteFlickerRate,
			true
			);
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(FlickerTimer);
		GetSprite()->SetVisibility(true, true);
		Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		bGotHitDuringCharged = false;
	}
}

void APlayerCharacter::KnockBack(AActor* DamageCauser)
{
	if (MovementComponent)
	{
		MovementComponent->FallingLateralFriction = 0.f;

		float Direction = (GetActorLocation() - DamageCauser->GetActorLocation()).GetSafeNormal().X;
		Direction = Direction < 0? -1 : 1;

		FVector LaunchDirection;
		LaunchDirection.X = Direction * KnockbackPowerHorizontal;
		LaunchDirection.Y = 0;
		LaunchDirection.Z = KnockbackPowerVertical;
		LaunchCharacter(LaunchDirection, false, true);

		
		FTimerHandle KnockBackTimer;
		GetWorld()->GetTimerManager().SetTimer(
			KnockBackTimer, this, &APlayerCharacter::ResetLateralFriction, InvincibilityTime);
	}
}

void APlayerCharacter::ResetLateralFriction()
{
	if (MovementComponent)
	{
		MovementComponent->FallingLateralFriction = LateralFriction;	
	}
}

void APlayerCharacter::ToggleSpriteFlicker()
{
	bIsSpriteVisible = !bIsSpriteVisible;
	GetSprite()->SetVisibility(bIsSpriteVisible, true);
}

void APlayerCharacter::ToggleIsStunned()
{
	bIsStunned = !bIsStunned;
}

void APlayerCharacter::ToggleIsSliding()
{
	if (SlideAction) SlideAction->Cancel();
	bIsSliding = false;
}

void APlayerCharacter::DestroyAndRespawnCharacter()
{
	GameMode->RespawnPlayer();
}




