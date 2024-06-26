// Fill out your copyright notice in the Description page of Project Settings.

// ReSharper disable CppRedundantAccessSpecifier
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "Blaster/Interfaces/InteractWithCrosshairsInterface.h"
#include "InputActionValue.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Camera/CameraComponent.h"
#include "BlasterCharacter.generated.h"

class AWeapon;
class UInputMappingContext;
class UInputAction;

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	ABlasterCharacter();
	virtual void Tick(float DeltaTime) override;
	void FireButtonStarted();
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	void PlayFireMontage(bool bAiming) const;

	void PlayHitReactMontage();
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastHit();

	virtual void OnRep_ReplicatedMovement() override;

protected:
	virtual void BeginPlay() override;

private:
	//	CAMERA

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FollowCamera;

	void HideCharacterIfCameraClose() const;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	float CameraThreshold = 200.f;

private:
	//	HUD

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;

private:
	//	Weapon - WidgetPickup displays only for the player that collides with the weapon
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	AWeapon* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon) const;

private:
	//	COMBAT
	UPROPERTY(VisibleAnywhere)
	class UCombatComponent* CombatComponent;

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

private:
	//	INPUT
	UPROPERTY(EditAnywhere, Category=Input)
	TSoftObjectPtr<UInputMappingContext> InputMapping;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* EquipAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CrouchAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AimAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* FireAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SwitchAutoFireRifle;

protected:
	//	Input response
protected:
	
	void Move(const FInputActionValue& Value);
	
protected:
	
	void Look(const FInputActionValue& Value);
	
protected:
	
	void EquipButtonPressed();
	
protected:
	void CrouchButtonPresses();
	void CrouchButtonReleased();
	
protected:
	//AIM
	
	void AimButtonPressed();
	void AimButtonReleased();
protected:
	//FIRE
	
	void FireButtonPressed();
	void FireButtonReleased();
	bool bAutomaticFire{true};
	void SwitchAutoFireButtonPressed();

protected:
	
	void CalculateAO_Pitch();
	float CalculateSpeed() const;
	void AimOffset(float DeltaTime);
	void SimProxiesTurn();

public: //Setters
	void SetOverlappingWeapon(AWeapon* Weapon);

public: //Getters
	bool IsWeaponEquipped() const;
	bool IsAiming() const;
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	FORCEINLINE auto GetCamera() const { return FollowCamera; }
	AWeapon* GetEquippedWeapon() const;
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FVector GetHitTarget() const;

public:
	void TurnInPlace(float DeltaTime);
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }

protected:
	void DrawDebugVelocityVector() const;

private:
	/* Variables related to rotating the character */
	
	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;
	ETurningInPlace TurningInPlace;
	bool bRotateRootBone;
	UPROPERTY(EditAnywhere)
	float TurnThreshold {1.f};
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;

private:
	//MONTAGES

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* FireMontage;

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* HitReactMontage;
};
