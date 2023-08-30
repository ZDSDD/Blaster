// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"

#include "MultiplayerSessionsSubsystem.h"
#include "Components/Button.h"
#include "OnlineSubsystem.h"
#include "String/Join.h"
#include "Windows/AllowWindowsPlatformTypes.h"


void UMenu::MenuSetup(const int32 NumberOfPublicConnections, const FString TypeOfMatch, FString LobbyPath)
{
	PathToLobby = LobbyPath.Append(FString(TEXT("?listen")));
	this->NumPublicConnections = NumberOfPublicConnections;
	this->MatchType = TypeOfMatch;
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	SetIsFocusable(true);

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController{World->GetFirstPlayerController()};
		if (PlayerController)
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		this->MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}

	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
		MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &ThisClass::OnFindSession);
		MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
		MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.
		                              AddDynamic(this, &ThisClass::OnDestroySession);
	}
}

bool UMenu::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}
	if (HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &ThisClass::HostButtonClicked);
	}
	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &ThisClass::JoinButtonClicked);
	}
	return true;
}

void UMenu::NativeDestruct()
{
	MenuTearDown();
	Super::NativeDestruct();
}

void UMenu::OnCreateSession(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		if (UWorld* World{GetWorld()})
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					10, 50, FColor::Silver, this->PathToLobby);
			}
			World->ServerTravel(this->PathToLobby);
		}
	}
	else
	{
		HostButton->SetIsEnabled(true);
	}
}

void UMenu::HostButtonClicked()
{
	HostButton->SetIsEnabled(false);
	if (this->MultiplayerSessionsSubsystem)
	{
		this->MultiplayerSessionsSubsystem->CreateSession(this->NumPublicConnections, this->MatchType);
	}
}

void UMenu::JoinButtonClicked()
{
	this->JoinButton->SetIsEnabled(false);
	if (this->MultiplayerSessionsSubsystem)
	{
		this->MultiplayerSessionsSubsystem->FindSession(10'000);
	}
}

void UMenu::MenuTearDown()
{
	RemoveFromParent();
	UWorld* World{GetWorld()};
	if (World)
	{
		APlayerController* PlayerController{World->GetFirstPlayerController()};
		if (PlayerController)
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
}

void UMenu::OnFindSession(const TArray<FOnlineSessionSearchResult>& SessionResult, bool bWasSuccessful)
{
	if (MultiplayerSessionsSubsystem == nullptr)
	{
		return;
	}
	for (const auto& Result : SessionResult)
	{
		FString SettingsValue;
		Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);
		if (SettingsValue == this->MatchType)
		{
			MultiplayerSessionsSubsystem->JoinSession(Result);
			return;
		}
	}
	if(!bWasSuccessful || SessionResult.Num() == 0)
	{
		this->JoinButton->SetIsEnabled(true);
	}
}

void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			FString Address;
			SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);

			APlayerController* PlayerController{GetGameInstance()->GetFirstLocalPlayerController()};
			if (PlayerController)
			{
				PlayerController->ClientTravel(Address, TRAVEL_Absolute);
			}
		}
	}
	if(Result != EOnJoinSessionCompleteResult::Success)
	{
	this->JoinButton->SetIsEnabled(true);
	}
}

void UMenu::OnDestroySession(bool bWasSuccessful)
{
}

void UMenu::OnStartSession(bool bWasSuccessful)
{
}
