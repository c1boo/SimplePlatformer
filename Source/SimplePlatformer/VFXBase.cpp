// Fill out your copyright notice in the Description page of Project Settings.


#include "VFXBase.h"

#include "PaperFlipbookComponent.h"

// Sets default values
AVFXBase::AVFXBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	
	PaperFlipbookComponent = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("Flipbook"));
	PaperFlipbookComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PaperFlipbookComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AVFXBase::BeginPlay()
{
	Super::BeginPlay();

	PaperFlipbookComponent->OnFinishedPlaying.AddDynamic(this, &AVFXBase::OnFinishedPlaying);
	PaperFlipbookComponent->SetLooping(false);
}

// Called every frame
void AVFXBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AVFXBase::OnFinishedPlaying()
{
	Destroy();
}

