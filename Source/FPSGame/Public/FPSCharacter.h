// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/Actor.h"
#include "FPSCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class AFPSProjectile;
class USoundBase;
class UAnimSequence;


UCLASS()
class AFPSCharacter : public ACharacter
{
	GENERATED_BODY()

protected:

	/** Pawn mesh: 1st person view  */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Mesh")
	USkeletalMeshComponent* Mesh1PComponent;

	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	USkeletalMeshComponent* GunMeshComponent;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* CameraComponent;

public:
	AFPSCharacter();

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category="Projectile")
	TSubclassOf<AFPSProjectile> ProjectileClass;

	/** Sound to play each time we fire */
	UPROPERTY(EditDefaultsOnly, Category="Gameplay")
	USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
	UAnimSequence* FireAnimation;

	UPROPERTY(BlueprintReadOnly, Category="Gameplay")
	bool bIsCarryingObjective;

	UPROPERTY(EditDefaultsOnly, Category="WallRun")
	float wallDistanceToRun = 100;

	UPROPERTY(EditDefaultsOnly, Category="WallRun")
	float wallRunSpeed = 900;

	UPROPERTY(EditDefaultsOnly, Category="WallRun")
	float wallRunJumpForce = 300;
	UPROPERTY(EditDefaultsOnly, Category="WallRun")
	float wallOffBouncing = 3;

	UPROPERTY(BlueprintReadOnly, Category="WallRun")
	bool isWallRunning = false;

	UPROPERTY(BlueprintReadOnly, Category="WallRun")
	bool isDashing = false;
private:
	FVector lastWallNormal;


protected:

	/** Fires a projectile. */
	void Fire();

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles strafing movement, left and right */
	void MoveRight(float Val);
	void DashPhysics(float deltatime);

	void OnDashStart();
	void WallRun();
	bool WallRunRaycastCheck(FHitResult* hitResult);

	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	

	UCharacterMovementComponent* MovementComponent;

public:
	/** Returns Mesh1P subobject **/
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1PComponent; }

	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return CameraComponent; }

	virtual void Jump() override;
	virtual void Tick(float DeltaTime) override;
};
