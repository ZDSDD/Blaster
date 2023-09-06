#pragma once

UENUM(BlueprintType)
enum class ETurningInPlace :uint8
{
	//Enum - Turning in place
	ETIP_Left UMETA(DisplayName = "Turning Left"),
	ETIP_Right UMETA(DisplayName = "Turning Right"),
	ETIP_NotTurning UMETA(DisplayName = "Not turning"),
	
	ETIP_MAX UMETA(DisplayName = "Default Max"),

};
