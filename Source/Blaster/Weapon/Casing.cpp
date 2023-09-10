// Fill out your copyright notice in the Description page of Project Settings.


#include "Casing.h"

#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"

ACasing::ACasing()
{
	PrimaryActorTick.bCanEverTick = false;

	CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Casing mesh"));
	SetRootComponent(CasingMesh);
	CasingMesh->SetCollisionResponseToChannel(ECC_Camera,ECR_Ignore);
	CasingMesh->SetSimulatePhysics(true);
	CasingMesh->SetEnableGravity(true);
	CasingMesh->SetNotifyRigidBodyCollision(true);
	ShellEjectionImpulse = 6.f;
}

void ACasing::BeginPlay()
{
	Super::BeginPlay();
	CasingMesh->AddImpulse(GetActorForwardVector() * ShellEjectionImpulse);
	CasingMesh->OnComponentHit.AddDynamic(this,&ThisClass::OnHit);
	// Set up the timer to call the Destroy function after TimeToDestroy seconds.
	GetWorldTimerManager().SetTimer(TimerHandle, this, &ThisClass::DestroyShell, TimeToDestroy, false);
}

void ACasing::DestroyShell()
{
	Destroy();
}

void ACasing::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                    FVector NormalImpulse, const FHitResult& Hit)
{
	if(ShellSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ShellSound, GetActorLocation());
	}
}


