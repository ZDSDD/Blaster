// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSessionSettings.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"

/*	.h Generated files needs to be at the bottom of headers declaration!	*/
#include "MultiplayerSessionsSubsystem.generated.h"


/*
 * Declaring our own custom delegates for the Menu class to bind callbacks to
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnCreateSessionComplete, bool, bWassSuccessful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnFindSessionsComplete,
	const TArray<FOnlineSessionSearchResult> & SessionResult, bool bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_OneParam(FMultiplayerOnJoinSessionComplete, EOnJoinSessionCompleteResult::Type Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnDestroySessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnStartSessionComplete, bool, bWasSuccessful);


/**
 *
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMultiplayerSessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UMultiplayerSessionsSubsystem();


	/*
	 *	To handle session functionality. The Menu class will call these
	 */

	void CreateSession(int32 NumPublicConnections, FString MatchType);
	void FindSession(int32 MaxSearchResults);
	void JoinSession(const FOnlineSessionSearchResult& SessionResult);
	void DestroySession();
	void StartSession();

	/*
	 * Our own custom delegates for the Menu class to bind callbacks to
	 */
	
	FMultiplayerOnCreateSessionComplete MultiplayerOnCreateSessionComplete;
	FMultiplayerOnFindSessionsComplete MultiplayerOnFindSessionsComplete;
	FMultiplayerOnJoinSessionComplete MultiplayerOnJoinSessionComplete;
	FMultiplayerOnStartSessionComplete MultiplayerOnStartSessionComplete;
	FMultiplayerOnDestroySessionComplete MultiplayerOnDestroySessionComplete;

protected:
	/*
	 *	Internal callbacks for the delegates we'll add to the [Online Session Interface] delegate list
	 *	These don't need to be called outside this class.
	 */

	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);

private:
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;
	
	/*
	 *	Add to the Online Session Interface delegate list.
	 *	We'll bring our MultiplayerSessionSubsystem internal callbacks to these.
	 */
	
	//	CREATE
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	//	FIND
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FDelegateHandle FindSessionCompleteDelegateHandle;
	//	JOIN
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	//	DESTROY
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FDelegateHandle DestroySessionCompleteDelegateHandle;
	//	START
	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
	FDelegateHandle StartSessionCompleteDelegateHandle;
private:
	bool bCreateSessionOnDestroy{false};
	int32 LastNumPublicConnections;
	FString LastMatchType;
};
