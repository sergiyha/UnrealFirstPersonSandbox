// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "FPSCharacter.h"


#include "DrawDebugHelpers.h"
#include "FPSProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "Animation/AnimSequence.h"
#include "SpecialMoves/H/WallRunState.h"


AFPSCharacter::AFPSCharacter()
{
	MovementComponent = Cast<UCharacterMovementComponent>(ACharacter::GetMovementComponent());
	MovementComponent->AirControl = 1;
	// Create a CameraComponent	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	CameraComponent->SetupAttachment(GetCapsuleComponent());
	CameraComponent->SetRelativeLocation(FVector(0, 0, BaseEyeHeight)); // Position the camera
	CameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1PComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh"));
	Mesh1PComponent->SetupAttachment(CameraComponent);
	Mesh1PComponent->CastShadow = false;
	Mesh1PComponent->SetRelativeRotation(FRotator(2.0f, -15.0f, 5.0f));
	Mesh1PComponent->SetRelativeLocation(FVector(0, 0, -160.0f));

	// Create a gun mesh component
	GunMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	GunMeshComponent->CastShadow = false;
	GunMeshComponent->SetupAttachment(Mesh1PComponent, "GripPoint");
}


void AFPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AFPSCharacter::Jump);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AFPSCharacter::Fire);

	PlayerInputComponent->BindAxis("MoveForward", this, &AFPSCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AFPSCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	PlayerInputComponent->BindKey(EKeys::LeftShift, IE_Pressed, this, &AFPSCharacter::OnDashStart);
}

FVector DashVector;
float DashTime = 0.1;
float TimeRemain = DashTime;

void AFPSCharacter::OnDashStart()
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	auto W = PlayerController->IsInputKeyDown(EKeys::W);
	auto A = PlayerController->IsInputKeyDown(EKeys::A);
	auto D = PlayerController->IsInputKeyDown(EKeys::D);
	auto S = PlayerController->IsInputKeyDown(EKeys::S);

	FVector dashDirection;
	FVector fVector = GetActorForwardVector();
	FVector rVector = GetActorRightVector();

	if (W) { dashDirection = fVector; }
	if (A) { dashDirection = rVector * -1; }
	if (D) { dashDirection = rVector; }
	if (S) { dashDirection = fVector * -1; }

	UE_LOG(LogTemp, Error, TEXT("Dash pressed"));

	MovementComponent->GravityScale = 0.0f;
	MovementComponent->Velocity = dashDirection * 4000;
	MovementComponent->BrakingDecelerationWalking = 0;
	MovementComponent->GroundFriction = 0;
	TimeRemain = DashTime;
	isDashing = true;
}


//every tick
void AFPSCharacter::DashPhysics(float deltatime)
{
	if (isDashing)
	{
		TimeRemain -= deltatime;
		if (TimeRemain <= 0)
		{
			//finish dash
			MovementComponent->Velocity = FVector::ZeroVector;
			if (!isWallRunning)
				MovementComponent->GravityScale = 1;
			MovementComponent->GroundFriction = 8;
			MovementComponent->BrakingDecelerationWalking = 2048;
			isDashing = false;
		}
	}
}


void AFPSCharacter::Jump()
{
	if (isWallRunning)
	{
		FVector launchVector = lastWallNormal;
		MovementComponent->Velocity += (launchVector * wallOffBouncing + GetActorUpVector() * wallOffBouncing / 2) *
			wallRunJumpForce;
	}
	else
	{
		Super::Jump();
	}
}

void AFPSCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	WallRun();
	DashPhysics(DeltaTime);
}


void AFPSCharacter::Fire()
{
	// try and fire a projectile
	if (ProjectileClass)
	{
		FVector MuzzleLocation = GunMeshComponent->GetSocketLocation("Muzzle");
		FRotator MuzzleRotation = GunMeshComponent->GetSocketRotation("Muzzle");

		//Set Spawn Collision Handling Override
		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

		// spawn the projectile at the muzzle
		GetWorld()->SpawnActor<AFPSProjectile>(ProjectileClass, MuzzleLocation, MuzzleRotation, ActorSpawnParams);
	}

	// try and play the sound if specified
	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// try and play a firing animation if specified
	if (FireAnimation)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1PComponent->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->PlaySlotAnimationAsDynamicMontage(FireAnimation, "Arms", 0.0f);
		}
	}
}


void AFPSCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}


void AFPSCharacter::MoveRight(float Value)
{
	if (Value != 0.0f && !isWallRunning)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}


void AFPSCharacter::WallRun()
{
	FHitResult hitResult;

	const bool walkingNow = MovementComponent->IsWalking();
	const bool isForwardButtonPressed = InputComponent->GetAxisValue("MoveForward") > 0;

	const bool isTraceTheWall = WallRunRaycastCheck(&hitResult);


	//UE_LOG(LogTemp, Error, TEXT("1:%s 2:%s 3:%s"), walkingNow ? TEXT("true") : TEXT("false"),
	//	isForwardButtonPressed ? TEXT("true") : TEXT("false"), isTraceTheWall ? TEXT("true") : TEXT("false"));

	if (!walkingNow && isForwardButtonPressed && isTraceTheWall)
	{
		if (isWallRunning == false)
		{
			FString s = hitResult.Actor.Get()->GetName();
			UE_LOG(LogTemp, Error, TEXT("%s"), *s);

			FVector velocity = MovementComponent->Velocity;
			MovementComponent->Velocity = FVector(velocity.X, velocity.Y, 0);
			MovementComponent->GravityScale = 0.1f;

			MovementComponent->MaxWalkSpeed = wallRunSpeed;
		}

		lastWallNormal = hitResult.Normal;
		isWallRunning = true;
	}
	else
	{
		if (isWallRunning)
		{
			UE_LOG(LogTemp, Error, TEXT("WallRunFinished"));
			MovementComponent->MaxWalkSpeed = 600;

			MovementComponent->GravityScale = 1;
			isWallRunning = false;
		}
	}
}


bool AFPSCharacter::WallRunRaycastCheck(FHitResult* hitResult)
{
	FVector rayOrigin = GetActorLocation();

	FVector fVec = GetActorForwardVector();
	FVector rVector = GetActorRightVector();

	FVector lVector = rVector * -1;
	//UE_LOG(LogTemp, Error, TEXT("lVect, %f %f %f"), rVector.X, rVector.Y, rVector.Z);
	FVector frVector = fVec + rVector;
	FVector flVector = fVec + lVector;

	FVector vectorsToRay[] = {fVec, rVector, lVector, frVector, flVector};


	bool isAnyTrace = false;
	TArray<FHitResult> hr_vectors;
	for (FVector vector : vectorsToRay)
	{
		FVector rayEnd = (vector * wallDistanceToRun) + rayOrigin;
		FHitResult hr;

		bool bIsHit = GetWorld()->LineTraceSingleByChannel(
			hr, // FHitResult object that will be populated with hit info
			rayOrigin, // starting position
			rayEnd, // end position
			ECC_GameTraceChannel2 // collision channel - 2rd custom one
		);

		if (!isAnyTrace && bIsHit)
		{
			isAnyTrace = true;
		}

		if (bIsHit)
		{
			hr_vectors.Add(hr);
			DrawDebugLine(GetWorld(), rayOrigin, rayEnd, FColor::Green, false);
		}
		else
		{
			DrawDebugLine(GetWorld(), rayOrigin, rayEnd, FColor::Red, false);
		}
	}


	if (hr_vectors.Num() > 0)
	{
		hr_vectors.Sort([rayOrigin](const FHitResult& LHS, const FHitResult& RHS)
		{
			return (LHS.ImpactPoint - rayOrigin).Size() < (RHS.ImpactPoint - rayOrigin).Size();
		});

		*hitResult = hr_vectors[0];
	}
	hr_vectors.Reset();


	return isAnyTrace;
}
