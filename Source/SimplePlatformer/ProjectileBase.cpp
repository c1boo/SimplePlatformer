// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBase.h"

#include "CharacterBase.h"
#include "Components/SphereComponent.h"
#include "PaperFlipbookComponent.h"
#include "VFXBase.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AProjectileBase::AProjectileBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	Sphere->SetCollisionProfileName(FName(TEXT("Projectile")), true);
	RootComponent = Sphere;

	PaperFlipBook = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("PaperFlipbook"));
	PaperFlipBook->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PaperFlipBook->SetupAttachment(Sphere);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement"));
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->InitialSpeed = 1000.f;
	ProjectileMovement->MaxSpeed = 1000.f;
}

// Called when the game starts or when spawned
void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();

	Sphere->OnComponentHit.AddDynamic(this, &AProjectileBase::OnHit);
	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AProjectileBase::OnOverlap);
}

void AProjectileBase::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent,
                            FVector NormalImpulse, const FHitResult& Hit)
{
	PlayHitVfx();
	Destroy();
}

void AProjectileBase::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bSweep, const FHitResult& SweepResult)
{
	ACharacterBase* OverlappedActor = Cast<ACharacterBase>(OtherActor);
	if (OverlappedActor && OverlappedActor->Faction != this->Faction)
	{
		UGameplayStatics::ApplyDamage(OverlappedActor, Damage, nullptr, this, UDamageType::StaticClass());
		PlayHitVfx();
		Destroy();
	}
}

void AProjectileBase::PlayHitVfx() const
{
	if (HitVFX && GetWorld()->IsGameWorld())
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		GetWorld()->SpawnActor<AVFXBase>(HitVFX, GetActorLocation(), GetActorRotation(), SpawnParameters);
	}
}

