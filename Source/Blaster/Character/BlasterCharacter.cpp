// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "GameFramework/CharacterMovementComponent.h"

ABlasterCharacter::ABlasterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	//Create Spring Arm and Camera
	this->CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("Camera Boom"));
	this->FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Follow Camera"));

	//Setting of the Boom
	this->CameraBoom->SetupAttachment(GetMesh());
	this->CameraBoom->TargetArmLength = 600.f;
	this->CameraBoom->bUsePawnControlRotation = true;
	this->CameraBoom->SetRelativeLocation(FVector(0, 0, 161));

	//Setting of the Camera
	this->FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	this->FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
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
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Input mapping is null"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Input System is NULL!!!\n Couldn't map the input"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Couldn't get LOCAL PLAYER"));
	}
}


void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	auto velocity = GetVelocity();

	auto ActorLocation = this->GetActorLocation();
	auto Velocity = GetVelocity();
	
	/*
	DrawDebugString(this->GetWorld(), ActorLocation, velocity.ToString(), nullptr, FColor::Red,0,true);
	DrawDebugDirectionalArrow(this->GetWorld(),ActorLocation,Velocity+ActorLocation,100,FColor::Emerald,false,0,1,10);
	*/
	
}


void
ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
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
	}
}

void ABlasterCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
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
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}
