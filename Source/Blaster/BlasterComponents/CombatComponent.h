// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "CombatComponent.generated.h"

#define TRACE_LENGTH 80'000.f;
struct FHUDPackage;
class AWeapon;
class ABlasterCharacter;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

public:
	friend class ABlasterCharacter;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	void EquipWeapon(AWeapon* WeaponToEquip);

protected:
	virtual void BeginPlay() override;

protected:
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon() const;

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

protected:
	void SetAiming(bool bAiming);

	void FireButtonPressed(bool Pressed);

	void TraceUnderCrosshair(FHitResult& TraceHitResult);

	void CalculateCrosshairSpread(float DeltaTime);
	void SetHUDCrosshairs(float DeltaTime);

private:
	ABlasterCharacter* Character;
	class ABlaster_PlayerController* Controller;
	class ABlasterHUD* HUD;

private:
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(Replicated)
	bool bIsAiming;

private:
	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	/*
	 * Hud And Crosshair
	 */
	FHUDPackage HUDPackage{};
	float CrosshairVelocityFactor{0};
	float CrosshairInAirFactor{0};
	float CrosshairAimFactor{0};
	float CrosshairShootingFactor{0};
	float CrosshairOnEnemyFactor{0};
	void SetCrosshairFactors(float DeltaTime);
	FVector HitTarget{};

	/*
	 * Aiming and FOV
	 */

	//Field of view when not aiming, set to the camera's base FOV in begin play
	float DefaultFOV;

	UPROPERTY(EditAnywhere, Category="Combat")
	float ZoomedFOV{30.f};

	float CurrentFOV;

	UPROPERTY(EditAnywhere, Category="Combat")
	float ZoomInterpSpeed{20.f};

	bool bEnemyUnderCrosshair;

	void InterpFOV(float DeltaTime);
	

private:
	bool bFireButtonPressed{};
};
