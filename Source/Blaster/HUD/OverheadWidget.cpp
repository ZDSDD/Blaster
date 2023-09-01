// Fill out your copyright notice in the Description page of Project Settings.


#include "OverheadWidget.h"

#include "Components/TextBlock.h"

void UOverheadWidget::SetDisplayText(FString TextToDisplay)
{
	if (DisplayText)
	{
		DisplayText->SetText(FText::FromString(TextToDisplay));
	}
}

void UOverheadWidget::ShowPlayerNetRole(APawn* InPawn)
{
	ENetRole RemoteRole = InPawn->GetRemoteRole();
	FString Role;
	switch (RemoteRole)
	{
	case ROLE_Authority:
		Role = "Authority";
		break;
	case ROLE_None:
		Role = "None";
		break;
	case ROLE_SimulatedProxy:
		Role = "SimulatedProxy";
		break;
	case ROLE_AutonomousProxy:
		Role = "AutonomousProxy";
		break;
	case ROLE_MAX:
		Role = "MAX";
		break;
	default: ;
	}
	FString RemoteRoleString = FString::Printf(TEXT("Remote Role: %s"), *Role);
	SetDisplayText(RemoteRoleString);
}

void UOverheadWidget::NativeDestruct()
{
	Super::NativeDestruct();
	RemoveFromParent();
}
