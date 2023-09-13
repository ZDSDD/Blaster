// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/BlasterCharacter.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapon/Weapon.h"

void UBlasterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
}

void UBlasterAnimInstance::UpdateCombatVariables()
{
	bWeaponEquipped = BlasterCharacter->IsWeaponEquipped();

	EquippedWeapon = BlasterCharacter->GetEquippedWeapon();

	bIsAiming = BlasterCharacter->IsAiming();
}

void UBlasterAnimInstance::UpdateMovementVariables()
{
	FVector Velocity = BlasterCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();

	bIsAccelerating = BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;

	bIsCrouched = BlasterCharacter->bIsCrouched;
}

void UBlasterAnimInstance::UpdateVariables()
{
	UpdateMovementVariables();
	UpdateCombatVariables();
	TurningInPlace = BlasterCharacter->GetTurningInPlace();
	bRotateRootBone = BlasterCharacter->ShouldRotateRootBone();
}

void UBlasterAnimInstance::UpdateAimOffsetYawAndPitch(float DeltaSeconds)
{
	//Offset Yaw for Strafing
	{
		FRotator AimRotation{BlasterCharacter->GetBaseAimRotation()};
		FRotator MovementRotation{UKismetMathLibrary::MakeRotFromX(BlasterCharacter->GetVelocity())};
		FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
		DeltaRotator = FMath::RInterpTo(DeltaRotator, DeltaRot, DeltaSeconds, 15.f);
	}
	YawOffset = DeltaRotator.Yaw;

	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = BlasterCharacter->GetActorRotation();

	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaSeconds;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaSeconds, 6.f);

	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	AO_Yaw = BlasterCharacter->GetAO_Yaw();
	AO_Pitch = BlasterCharacter->GetAO_Pitch();
}

void UBlasterAnimInstance::UpdateHandPlacement(float DeltaSeconds)
{
	if (!bWeaponEquipped || !EquippedWeapon || !EquippedWeapon->GetWeaponMesh() || !BlasterCharacter->GetMesh())
		return;

	LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), RTS_World);
	FVector OutPosition;
	FRotator OutRotation;
	BlasterCharacter->GetMesh()->TransformToBoneSpace(FName("Hand_R"), LeftHandTransform.GetLocation(),
	                                                  FRotator::ZeroRotator, OutPosition, OutRotation);
	LeftHandTransform.SetLocation(OutPosition);
	LeftHandTransform.SetRotation(FQuat(OutRotation));

	/*Setup the right hand*/

	if (BlasterCharacter->IsLocallyControlled())
	{
		bLocallyControlled = true;
		FTransform RightHandTransform = BlasterCharacter->GetMesh()->GetSocketTransform(FName("Hand_R"), RTS_World);
		FRotator LookAtoRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(),
		                                                                  RightHandTransform.GetLocation() + (
			                                                                  RightHandTransform.GetLocation() -
			                                                                  BlasterCharacter->GetHitTarget()));
		RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtoRotation, DeltaSeconds, 30.f);
	}
	else
	{
		// Corrects the position of clients weapon
		AO_Pitch += 30;
		AO_Yaw += 15;
	}
}

void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (!BlasterCharacter)
	{
		BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
	}
	if (!BlasterCharacter) return;

	UpdateVariables();

	UpdateAimOffsetYawAndPitch(DeltaSeconds);

	UpdateHandPlacement(DeltaSeconds);
}
