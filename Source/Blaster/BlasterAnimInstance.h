// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "BlasterAnimInstance.generated.h"
/**
 * 
 */
UCLASS()
class BLASTER_API UBlasterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeInitializeAnimation() override;
	void UpdateCombatVariables();
	void UpdateMovementVariables();
	void UpdateVariables();
	void UpdateAimOffsetYawAndPitch(float DeltaSeconds);
	void UpdateHandPlacement(float DeltaSeconds);
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	class ABlasterCharacter* BlasterCharacter;
	
private:

	/*
	 *	MOVEMENT RELATED VARIABLES
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Character movement", meta = (AllowPrivateAccess = "true"))
	float Speed;

	UPROPERTY(BlueprintReadOnly, Category = "Character movement", meta = (AllowPrivateAccess = "true"))
	bool bIsInAir;

	UPROPERTY(BlueprintReadOnly, Category = "Character movement", meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating;

	UPROPERTY(BlueprintReadOnly, Category = "Character movement", meta = (AllowPrivateAccess = "true"))
	bool bIsCrouched;
	
	UPROPERTY(BlueprintReadOnly, Category = "Character movement", meta = (AllowPrivateAccess = "true"))
	float Lean;
	
private:

	/*
	 * COMBAT RELATED VARIABLES
	 */
	
	UPROPERTY()
	class AWeapon * EquippedWeapon;
	
	UPROPERTY(BlueprintReadOnly, Category = "Character combat", meta = (AllowPrivateAccess = "true"))
	bool bWeaponEquipped;
	
	UPROPERTY(BlueprintReadOnly, Category = "Character combat", meta = (AllowPrivateAccess = "true"))
	bool bIsAiming;

	UPROPERTY(BlueprintReadOnly, Category = "Character combat", meta = (AllowPrivateAccess = "true"))
	float YawOffset;
	
	UPROPERTY(BlueprintReadOnly, Category = "Character combat", meta = (AllowPrivateAccess = "true"))
	float AO_Yaw;

	UPROPERTY(BlueprintReadOnly, Category = "Character combat", meta = (AllowPrivateAccess = "true"))
	float AO_Pitch;
	
private:
	UPROPERTY(BlueprintReadOnly, Category = "Character position", meta = (AllowPrivateAccess = "true"))
	FTransform LeftHandTransform;

	UPROPERTY(BlueprintReadOnly, Category = "Character position", meta = (AllowPrivateAccess = "true"))
	ETurningInPlace TurningInPlace;
	
	UPROPERTY(BlueprintReadOnly, Category = "Character position", meta = (AllowPrivateAccess = "true"))
	FRotator RightHandRotation;
	
	UPROPERTY(BlueprintReadOnly, Category = "Character position", meta = (AllowPrivateAccess = "true"))
	bool bLocallyController;

private:
	FRotator CharacterRotationLastFrame;
	FRotator CharacterRotation;
	FRotator DeltaRotator;
		
};
