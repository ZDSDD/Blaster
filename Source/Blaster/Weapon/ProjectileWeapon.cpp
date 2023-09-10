// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"

/*
 * GetOwner() will get the owner of the weapon that is set up in the CombatComponent:
 * void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
 */

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);
	if(!HasAuthority()) return;
	
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	const USkeletalMeshSocket* MuzzleFlashSocket{GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"))};
	if (MuzzleFlashSocket)
	{
		const FTransform SocketTransform{MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh())};
		//From muzzle flash socket to hit location from TraceUnderCross-hairs
		const FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		const FRotator TargetRotation = ToTarget.Rotation();
		if(ProjectileClass && InstigatorPawn)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner();
			SpawnParams.Instigator = InstigatorPawn;
			UWorld* const World {GetWorld()};
			if(World)
			{
				World->SpawnActor<AProjectile>(
					ProjectileClass,
					SocketTransform.GetLocation(),
					TargetRotation,
					SpawnParams
					);
			}
		}
	}
}
