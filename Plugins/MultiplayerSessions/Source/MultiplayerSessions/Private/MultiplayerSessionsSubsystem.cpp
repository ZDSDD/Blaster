// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerSessionsSubsystem.h"

#include "OnlineSubsystem.h"
#include "Online/OnlineSessionNames.h"


//	CONSTRUCTOR
UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem() :
	CreateSessionCompleteDelegate(
		FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
	FindSessionsCompleteDelegate(
		FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete)),
	JoinSessionCompleteDelegate(
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete)),
	DestroySessionCompleteDelegate(
		FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete)),
	StartSessionCompleteDelegate(
		FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete))
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		this->SessionInterface = Subsystem->GetSessionInterface();
	}
}

//	CONSTRUCTOR


void UMultiplayerSessionsSubsystem::CreateSession(int32 NumPublicConnections, FString MatchType)
{
	if (!SessionInterface.IsValid())
	{
		return;
	}
	const auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if (ExistingSession)
	{
		this->bCreateSessionOnDestroy = true;
		this->LastNumPublicConnections = NumPublicConnections;
		this->LastMatchType = MatchType;
		
		DestroySession();
	}

	//	Store the delegate in the FDelegateHandle, so we can later remove it from the delegate list
	this->CreateSessionCompleteDelegateHandle = this->SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
		this->CreateSessionCompleteDelegate);

	this->LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
	this->LastSessionSettings->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL";
	this->LastSessionSettings->NumPublicConnections = NumPublicConnections;
	this->LastSessionSettings->bAllowJoinInProgress = true;
	this->LastSessionSettings->bAllowJoinViaPresence = true;
	this->LastSessionSettings->bShouldAdvertise = true;
	this->LastSessionSettings->bUsesPresence = true;
	this->LastSessionSettings->bUseLobbiesIfAvailable = true;
	this->LastSessionSettings->Set(FName("MatchType"), MatchType,
	                               EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	this->LastSessionSettings->BuildUniqueId = 1;

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();

	//  When session wasn't created successfully
	if (!SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession,
	                                     *LastSessionSettings))
	{
		this->SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);

		//	Broadcast our own custom delegate
		MultiplayerOnCreateSessionComplete.Broadcast(false);
	}
}

void UMultiplayerSessionsSubsystem::FindSession(int32 MaxSearchResults)
{
	//	Return if the session interface is NOT valid
	if (!this->SessionInterface.IsValid())
	{
		return;
	}
	//	Set the find session complete DELEGATE and DELEGATE HANDLE
	this->FindSessionCompleteDelegateHandle = this->SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(
		FindSessionsCompleteDelegate);

	//	CONFIGURE LAST SESSION SEARCH
	this->LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
	this->LastSessionSearch->MaxSearchResults = MaxSearchResults;
	this->LastSessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL";
	this->LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	// FIND SESSION
	const ULocalPlayer* LocalPlayer{GetWorld()->GetFirstLocalPlayerFromController()};

	if (!this->SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(),
	                                          this->LastSessionSearch.ToSharedRef()))
	{
		//	if session was NOT found: CLEAR the delegate handle
		this->SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(this->FindSessionCompleteDelegateHandle);
		//	Broadcast that the session was NOT found
		this->MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
	}
}

void UMultiplayerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult& SessionResult)
{
	if (!SessionInterface.IsValid())
	{
		this->MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		return;
	}
	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
		JoinSessionCompleteDelegate);
	
	const ULocalPlayer* LocalPlayer{GetWorld()->GetFirstLocalPlayerFromController()};
	if(this->SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession,SessionResult))
	{
		this->SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
	}

	
}

void UMultiplayerSessionsSubsystem::DestroySession()
{
	if(!SessionInterface.IsValid())
	{
		MultiplayerOnDestroySessionComplete.Broadcast(false);
		return;
	}
	this->DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);
	if(!SessionInterface->DestroySession(NAME_GameSession))
	{
		this->SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(this->DestroySessionCompleteDelegateHandle);
		MultiplayerOnDestroySessionComplete.Broadcast(false);
		return;
	}
}

void UMultiplayerSessionsSubsystem::StartSession()
{
}

void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface)
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}
	MultiplayerOnCreateSessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (this->SessionInterface)
	{
		this->SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionCompleteDelegateHandle);
	}
	//	If there is NO search results: Broadcast FALSE and return;
	if (LastSessionSearch->SearchResults.Num() <= 0)
	{
		this->MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		return;
	}

	this->MultiplayerOnFindSessionsComplete.Broadcast(LastSessionSearch->SearchResults, bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	//If joined successfully: detach the delegate handle
	if(this->SessionInterface)
	{
		this->SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(this->JoinSessionCompleteDelegateHandle);
	}
	MultiplayerOnJoinSessionComplete.Broadcast(Result);
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if(SessionInterface)
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(this->DestroySessionCompleteDelegateHandle);
	}
	if(bWasSuccessful && bCreateSessionOnDestroy)
	{
		bCreateSessionOnDestroy = false;
		CreateSession(this->LastNumPublicConnections,this->LastMatchType);
	}
	MultiplayerOnDestroySessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
}
