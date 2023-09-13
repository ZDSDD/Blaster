// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/Blaster.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/BlasterAnimInstance.h"

ABlasterCharacter::ABlasterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	//Create Spring Arm and Camera
	this->CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("Camera Boom"));
	this->FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Follow Camera"));

	//Camera boom settings
	this->CameraBoom->SetupAttachment(GetMesh());
	this->CameraBoom->TargetArmLength = 600.f;
	this->CameraBoom->bUsePawnControlRotation = true;
	this->CameraBoom->SetRelativeLocation(FVector(0, 0, 161));

	//Camera settings
	this->FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	this->FollowCamera->bUsePawnControlRotation = false;
	this->bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	//Widget settings
	this->OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Overhead Widget"));
	this->OverheadWidget->SetupAttachment(RootComponent);

	//Combat component
	this->CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	this->CombatComponent->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	GetMesh()->SetCollisionObjectType(ECC_BlasterSM);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	this->TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	this->NetUpdateFrequency = 66.f;
	this->MinNetUpdateFrequency = 33.f;
}

void ABlasterCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();

	SimProxiesTurn();

	TimeSinceLastMovementReplication = 0.f;
}

void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (GetLocalRole() > ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else //
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}
	HideCharacterIfCameraClose();
}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();

	// In your cpp...
	if (APlayerController* PlayerController = Cast<APlayerController>(this->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSystem = ULocalPlayer::GetSubsystem<
			UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (!InputMapping.IsNull())
			{
				InputSystem->AddMappingContext(InputMapping.LoadSynchronous(), 0);
			}
		}
	}
}

/*
 * Hides the meshes if camera is too close to the player.
 * Meshes are invisible only to the locally controlled player. Other client can still see the meshes
 */
void ABlasterCharacter::HideCharacterIfCameraClose() const
{
	if (!IsLocallyControlled() || !FollowCamera || !GetMesh())return;
	//	FollowCamera->GetComponentLocation() - GetActorLocation() gives vector. Size() gives the length of this vector;
	const bool bShouldHideMeshes{(FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold};
	GetMesh()->SetVisibility(!bShouldHideMeshes);
	if (CombatComponent && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponMesh())
	{
		CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = bShouldHideMeshes;
	}
}

void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon) const
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

void ABlasterCharacter::EquipButtonPressed()
{
	if (!this->CombatComponent) return;

	if (this->HasAuthority())
	{
		this->CombatComponent->EquipWeapon(OverlappingWeapon);
	}
	else
	{
		ServerEquipButtonPressed();
	}
}

void ABlasterCharacter::CrouchButtonPresses()
{
	Crouch();
}

void ABlasterCharacter::CrouchButtonReleased()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
}

void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
	if (CombatComponent)
	{
		CombatComponent->EquipWeapon(OverlappingWeapon);
	}
}


void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//	Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		//	Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::Move);
		//	Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::Look);
		//	Equip
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Triggered, this,
		                                   &ABlasterCharacter::EquipButtonPressed);
		//	Crouch
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this,
		                                   &ABlasterCharacter::CrouchButtonPresses);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this,
		                                   &ABlasterCharacter::CrouchButtonReleased);
		//	Aim
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Triggered, this,
		                                   &ABlasterCharacter::AimButtonPressed);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this,
		                                   &ABlasterCharacter::AimButtonReleased);
		//	Shoot
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this,
		                                   &ABlasterCharacter::FireButtonPressed);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Canceled, this,
		                                   &ABlasterCharacter::FireButtonReleased);
	}
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (this->CombatComponent)
	{
		this->CombatComponent->Character = this;
	}
}

void ABlasterCharacter::PlayFireMontage(bool bAiming) const
{
	if (!this->CombatComponent || !this->CombatComponent->EquippedWeapon) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && this->FireMontage)
	{
		AnimInstance->Montage_Play(this->FireMontage);
		FName SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayHitReactMontage()
{
	if (!this->CombatComponent || !this->CombatComponent->EquippedWeapon) return;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && this->HitReactMontage)
	{
		AnimInstance->Montage_Play(this->HitReactMontage);
		FName SectionName = FName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}


void ABlasterCharacter::MulticastHit_Implementation()
{
	PlayHitReactMontage();
}


void ABlasterCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (this->Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = this->Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ABlasterCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (this->Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ABlasterCharacter::AimButtonPressed()
{
	if (this->CombatComponent)
	{
		this->CombatComponent->SetAiming(true);
	}
}

void ABlasterCharacter::AimButtonReleased()
{
	if (this->CombatComponent)
	{
		this->CombatComponent->SetAiming(false);
	}
}

void ABlasterCharacter::CalculateAO_Pitch()
{
	this->AO_Pitch = GetBaseAimRotation().Pitch;
	if (this->AO_Pitch > 90.f && !IsLocallyControlled())
	{
		//Map pitch from [270, 360) to [-90,0)
		const FVector2D InRange(270.f, 360.f);
		const FVector2D OutRange(-90.f, 0.f);
		this->AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, this->AO_Pitch);
	}
}

float ABlasterCharacter::CalculateSpeed() const
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}

void ABlasterCharacter::AimOffset(const float DeltaTime)
{
	if (this->CombatComponent && !this->CombatComponent->EquippedWeapon) return;
	
	const float Speed = CalculateSpeed();
	const bool bIsInAir = GetCharacterMovement()->IsFalling();

	/*
	 * Setting-up the Aim Offset YAW and PITCH
	 */
	if (Speed == 0.f && !bIsInAir) //Standing still, not jumping
	{
		bRotateRootBone = true;
		const FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		const FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(
			CurrentAimRotation, this->StartingAimRotation);
		this->AO_Yaw = DeltaAimRotation.Yaw;
		if (this->TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			this->InterpAO_Yaw = AO_Yaw;
		}
		this->bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}
	if (Speed > 0.f || bIsInAir) //Running or jumping
	{
		bRotateRootBone = false;
		this->StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		this->AO_Yaw = 0.f;
		this->bUseControllerRotationYaw = true;
		this->TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}
	CalculateAO_Pitch();
}

void ABlasterCharacter::SimProxiesTurn()
{
	if (!CombatComponent || !CombatComponent->EquippedWeapon) return;
	
	bRotateRootBone = false;

	if(CalculateSpeed() > 0)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	
	
	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;

	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if (ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}

void ABlasterCharacter::FireButtonPressed()
{
	if (this->CombatComponent)
	{
		this->CombatComponent->FireButtonPressed(true);
	}
}

void ABlasterCharacter::FireButtonReleased()
{
	if (this->CombatComponent)
	{
		this->CombatComponent->FireButtonPressed(false);
		UE_LOG(LogTemp, Warning, TEXT("FireButtonReleased"));
	}
}

FVector ABlasterCharacter::GetHitTarget() const
{
	if (this->CombatComponent)
	{
		return this->CombatComponent->HitTarget;
	}

	return FVector();
}

void ABlasterCharacter::TurnInPlace(const float DeltaTime)
{
	if (this->AO_Yaw > 90.f)
	{
		this->TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)
	{
		this->TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	if (this->TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		this->InterpAO_Yaw = FMath::FInterpTo(this->InterpAO_Yaw, 0.f, DeltaTime, 5.f);
		this->AO_Yaw = this->InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			this->TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			this->StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (this->OverlappingWeapon)
	{
		this->OverlappingWeapon->ShowPickupWidget(false);
	}
	this->OverlappingWeapon = Weapon;

	if (IsLocallyControlled() && this->OverlappingWeapon)
	{
		this->OverlappingWeapon->ShowPickupWidget(true);
	}
}

bool ABlasterCharacter::IsWeaponEquipped() const
{
	return CombatComponent && CombatComponent->EquippedWeapon;
}

bool ABlasterCharacter::IsAiming() const
{
	return CombatComponent && CombatComponent->bIsAiming;
}

AWeapon* ABlasterCharacter::GetEquippedWeapon() const
{
	if (!CombatComponent) return nullptr;

	return CombatComponent->EquippedWeapon;
}


void ABlasterCharacter::DrawDebugVelocityVector() const
{
	auto velocity = GetVelocity();
	auto ActorLocation = this->GetActorLocation();
	auto Velocity = GetVelocity();


	DrawDebugString(GetWorld(), ActorLocation, velocity.ToString(), nullptr, FColor::Red, 0, true);
	DrawDebugDirectionalArrow(GetWorld(), ActorLocation, Velocity + ActorLocation, 100, FColor::Emerald, false, 0,
	                          1, 10);
}
