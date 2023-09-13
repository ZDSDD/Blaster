// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/PlayerController/Blaster_PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	BaseWalkSpeed = 600;
	AimWalkSpeed = 450;
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bIsAiming);
}


void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
		if (Character->GetCamera())
		{
			DefaultFOV = Character->GetCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}
	}
}

void UCombatComponent::TickComponent(const float DeltaTime, const ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (Character && Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshair(HitResult);
		HitTarget = HitResult.ImpactPoint;
		SetHUDCrosshairs(DeltaTime);
		InterpFOV(DeltaTime);
	}
}

void UCombatComponent::SetAiming(const bool bAiming)
{
	bIsAiming = bAiming;
	// We can set it here too, so there is no need to wait for the server callback with information about aiming
	ServerSetAiming(bIsAiming);

	if (Character && EquippedWeapon)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::ServerSetAiming_Implementation(const bool bAiming)
{
	bIsAiming = bAiming;
	if (Character && EquippedWeapon)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::OnRep_EquippedWeapon() const
{
	if (EquippedWeapon && Character)
	{
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
}


void UCombatComponent::FireButtonPressed(const bool Pressed)
{
	bFireButtonPressed = Pressed;
	if (bFireButtonPressed && EquippedWeapon)
	{
		CrosshairShootingFactor = 0.75;
		ServerFire(HitTarget);
	}
}

/*
 *	Starts trace from the middle of the camera to the crosshair middle direction
 *	If any other blaster character was hit, changes the crosshair color to RED
 */
void UCombatComponent::TraceUnderCrosshair(FHitResult& TraceHitResult
)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	const FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	if (!UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection))
	{
		return;
	}

	FVector Start = CrosshairWorldPosition;

	// Push the start of the trace (that is the camera) more to the front (approximately to the Character location)
	if (Character)
	{
		const float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
		Start += CrosshairWorldDirection * DistanceToCharacter;
	}

	//End location of the ray trace
	const FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

	//No hit
	if (!GetWorld()->LineTraceSingleByChannel(TraceHitResult, Start, End, ECC_Visibility))
	{
		TraceHitResult.ImpactPoint = End;
	}
	//Traced other blaster character
	if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>() &&
		TraceHitResult.GetActor() != Character)
	{
		bEnemyUnderCrosshair = true;
	}
	else
	{
		bEnemyUnderCrosshair = false;
	}
	HUDPackage.CrosshairColor = bEnemyUnderCrosshair ? FLinearColor::Red : FLinearColor::White;
}

void UCombatComponent::CalculateCrosshairSpread(float DeltaTime)
{
	SetCrosshairFactors(DeltaTime);
	
	HUDPackage.CrosshairSpread =
		0.5f +
		CrosshairVelocityFactor +
		CrosshairInAirFactor -
		CrosshairAimFactor +
		CrosshairShootingFactor -
		CrosshairOnEnemyFactor;
}

void UCombatComponent::SetCrosshairFactors(const float DeltaTime)
{
	//[0,600] -> [0,1]
	const FVector2d WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
	FVector Velocity = Character->GetVelocity();
	Velocity.Z = 0.f;
	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, {0.f, 1.f},
																Velocity.Size());
	CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 5.f);
	
	if (Character->GetCharacterMovement()->IsFalling())
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
	}
	else
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 15.f);
	}

	if (bIsAiming)
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.38f, DeltaTime, 15.f);
	}
	else
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 15.f);
	}
	if(bEnemyUnderCrosshair)
	{
		CrosshairOnEnemyFactor = FMath::FInterpTo(CrosshairOnEnemyFactor,0.15f,DeltaTime,5.f);
	}
	else
	{
		CrosshairOnEnemyFactor = FMath::FInterpTo(CrosshairOnEnemyFactor,0.f,DeltaTime,15.f);

	}
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (!Character || !Character->Controller) return;
	Controller = Controller ? Controller : Cast<ABlaster_PlayerController>(Character->Controller);
	if (!Controller) return;
	HUD = HUD ? HUD : Cast<ABlasterHUD>(Controller->GetHUD());
	if (!HUD) return;

	if (EquippedWeapon)
	{
		HUDPackage.Crosshairscenter = EquippedWeapon->CrosshairCenter;
		HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairLeft;
		HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairRight;
		HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairTop;
		HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairDown;
	}
	else
	{
		HUDPackage.Crosshairscenter = nullptr;
		HUDPackage.CrosshairsLeft = nullptr;
		HUDPackage.CrosshairsRight = nullptr;
		HUDPackage.CrosshairsTop = nullptr;
		HUDPackage.CrosshairsBottom = nullptr;
	}
	CalculateCrosshairSpread(DeltaTime);
	HUD->SetHUDPackage(HUDPackage);
}


void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (!EquippedWeapon) return;
	if (!Character || !Character->GetCamera()) return;

	if (bIsAiming)
	{
		CurrentFOV = FMath::FInterpTo(
			CurrentFOV,
			bEnemyUnderCrosshair ? EquippedWeapon->GetZoomedFOV() - 2 : EquippedWeapon->GetZoomedFOV(),
			DeltaTime,
			EquippedWeapon->GetZoomInterpSpeed());
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	Character->GetCamera()->SetFieldOfView(CurrentFOV);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (!EquippedWeapon) return;

	if (Character)
	{
		Character->PlayFireMontage(bIsAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}


void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (!Character || !WeaponToEquip)
		return;

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}
	EquippedWeapon->SetOwner(Character);
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}
