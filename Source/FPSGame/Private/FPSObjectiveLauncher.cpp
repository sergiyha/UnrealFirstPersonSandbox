// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSObjectiveLauncher.h"
#include "AudioDevice.h"
#include "FPSCharacter.h"
#include "Components/BoxComponent.h"

// Sets default values
AFPSObjectiveLauncher::AFPSObjectiveLauncher()
{
	OverlapComp = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapComp"));
	OverlapComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OverlapComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	OverlapComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	OverlapComp->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
	OverlapComp->SetBoxExtent(FVector(200.0f));
	OverlapComp->SetHiddenInGame(false);
	OverlapComp->OnComponentBeginOverlap.AddDynamic(this, &AFPSObjectiveLauncher::HandleOverlap);

	LaunchStrength = 1500;
	LaunchYRotation = 35;
}

void AFPSObjectiveLauncher::HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                          UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                          const FHitResult& SweepResult)
{
	FRotator startDirection = GetActorRotation();
	startDirection.Pitch += LaunchYRotation;
	FVector launchDirection = startDirection.Vector() * LaunchStrength;

	AFPSCharacter* chrctr = Cast<AFPSCharacter>(OtherActor);
	if (chrctr)
	{
		chrctr->LaunchCharacter(launchDirection, true, true);
		UE_LOG(LogTemp, Warning, TEXT("Overlapped with Launcher zone! PAWN OBJECT"));
	}
	else if (OtherComp && OtherComp->IsSimulatingPhysics())
	{
		OtherComp->AddImpulse(launchDirection, NAME_None, true);
		UE_LOG(LogTemp, Warning, TEXT("Overlapped with Launcher zone! PHYSICS OBJECT"));
	}
}

void AFPSObjectiveLauncher::BeginPlay()
{
	Super::BeginPlay();
}

void AFPSObjectiveLauncher::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
