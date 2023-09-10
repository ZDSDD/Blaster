// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Casing.generated.h"

UCLASS()
class BLASTER_API ACasing : public AActor
{
	GENERATED_BODY()
	
public:	
	ACasing();
	void DestroyShell();
	
protected:
	virtual void BeginPlay() override;


private:

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* CasingMesh;

	UPROPERTY(EditAnywhere)
	float ShellEjectionImpulse{};

	UPROPERTY(EditAnywhere)
	class USoundCue* ShellSound;

	UPROPERTY()
	FTimerHandle TimerHandle;

	UPROPERTY(EditAnywhere)
	float TimeToDestroy{5.f};
private:
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
