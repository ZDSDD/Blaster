// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Blaster/Character/BlasterCharacter.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	this->WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	this->WeaponMesh->SetupAttachment(RootComponent);
	SetRootComponent(WeaponMesh);

	this->WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block);
	this->WeaponMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	this->WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	this->AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	this->AreaSphere->SetupAttachment(RootComponent);
	this->AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	this->AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	this->PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	this->PickupWidget->SetupAttachment(RootComponent);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("PLAYER HAS AUTHORITY"));
		this->AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		this->AreaSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		this->AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereOverlap);
		this->AreaSphere->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnSphereEndOverlap);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("PLAYER HAS !NOT! AUTHORITY"));
	}
	if (PickupWidget)
	{
		this->PickupWidget->SetVisibility(false);
	}
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                              const FHitResult& SweepResult)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);

	if (BlasterCharacter && PickupWidget)
	{
		BlasterCharacter->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);

	if (BlasterCharacter && PickupWidget)
	{
		BlasterCharacter->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWeapon::ShowPickupWidget(const bool bShowWidget)
{
	if (this->PickupWidget)
	{
		this->PickupWidget->SetVisibility(bShowWidget);
	}
}
