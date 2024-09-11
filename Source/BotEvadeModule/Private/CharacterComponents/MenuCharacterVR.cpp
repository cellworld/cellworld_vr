#include "BotEvadeModule/Public/CharacterComponents/MenuCharacterVR.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Online/OnlineSessionNames.h" // required for version > 5.1 for `SEARCH_PRESENCE` macro
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystemUtils.h"

//////////////////////////////////////////////////////////////////////////
// AMenuCharacterVR

AMenuCharacterVR::AMenuCharacterVR():
	CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
	FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete)),
	JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete))
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	// IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	// if (OnlineSubsystem) {
	// 	UE_LOG(LogTemp, Log, TEXT("AMenuCharacterVR::AMenuCharacterVR(): OnlineSubsystem valid!"))
	// 	OnlineSessionInterface = OnlineSubsystem->GetSessionInterface();
	//
	// 	if (GEngine) {
	// 		// OnlineSessionInterface = Online::GetSessionInterface(GEngine->GetWorld()); // v2 - find foru link..
	// 		GEngine->AddOnScreenDebugMessage(
	// 			-1,
	// 			5.f,
	// 			FColor::Blue,
	// 			FString::Printf(TEXT("[AMenuCharacterVR::AMenuCharacterVR] Found subsystem: %s"),
	// 				*OnlineSubsystem->GetSubsystemName().ToString())
	// 		);
	// 	}
	//
	// }
	//
	// if(!OnlineSessionInterface.IsValid()) {
	// 	if (GEngine) {
	// 		// OnlineSessionInterface = Online::GetSessionInterface(GEngine->GetWorld()); // v2 - find foru link..
	// 		GEngine->AddOnScreenDebugMessage(
	// 			-1,
	// 			5.f,
	// 			FColor::Blue,
	// 			FString::Printf(TEXT("[AMenuCharacterVR::AMenuCharacterVR] OnlineSessionInterface: not valid!"))
	// 		);
	// 	}
	// }
}

//////////////////////////////////////////////////////////////////////////
// Input

void AMenuCharacterVR::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) {
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMenuCharacterVR::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMenuCharacterVR::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMenuCharacterVR::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMenuCharacterVR::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AMenuCharacterVR::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AMenuCharacterVR::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AMenuCharacterVR::OnResetVR);
}

void AMenuCharacterVR::CreateGameSession() {
	
	// Called when pressing the 1 key

	UE_LOG(LogTemp, Log, TEXT("[AMenuCharacterVR::CreateGameSession()] Called!"))
	// if (!OnlineSessionInterface.IsValid()) { return; }
	// UE_LOG(LogTemp, Log, TEXT("[AMenuCharacterVR::CreateGameSession()] OnlineSessionInterface is valid!"))
	//
	// // check if a game session already exists, if not null, destroy it
	// FNamedOnlineSession* ExistingSession = OnlineSessionInterface->GetNamedSession(NAME_GameSession); // auto -> classdef
	// if (ExistingSession != nullptr) {
	// 	OnlineSessionInterface->DestroySession(NAME_GameSession);
	// }
	//
	// OnlineSessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);
	//
	// TSharedPtr<FOnlineSessionSettings> SessionSettings = MakeShareable(new FOnlineSessionSettings());
	// SessionSettings->bIsLANMatch = true; // was false 
	// SessionSettings->NumPublicConnections = 4;
	// SessionSettings->bAllowJoinInProgress = true;
	// SessionSettings->bUseLobbiesIfAvailable = true;
	// SessionSettings->bAllowJoinViaPresence = true; // steam
	// SessionSettings->bShouldAdvertise = true;	   // steam
	// SessionSettings->bUsesPresence = true;		   // steam
	// SessionSettings->Set(FName("MatchType"), FString("FreeForAll"),
	// 	EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	//
	// const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	// OnlineSessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *SessionSettings);

	// UE_LOG(LogTemp, Warning, TEXT("[AMenuCharacterVR::CreateGameSession()] Created Session! "))
}

void AMenuCharacterVR::JoinGameSession() {
	UE_LOG(LogTemp, Warning, TEXT("[AMenuCharacterVR::JoinGameSession] Looking for session! "))

	// Find game sessions
	// if (!OnlineSessionInterface.IsValid()) {
	// 	UE_LOG(LogTemp, Warning, TEXT("[AMenuCharacterVR::JoinGameSession] OnlineSessionInterface not valid! "))
	// 	return;
	// }
	//
	// OnlineSessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);
	//
	// SessionSearch = MakeShareable(new FOnlineSessionSearch());
	// SessionSearch->MaxSearchResults = 100;
	// SessionSearch->bIsLanQuery = true; // was false
	// // make sure all sessions we find are using presence 
	// // SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals); 
	//
	// const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	// OnlineSessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef());
	// UE_LOG(LogTemp, Warning, TEXT("[AMenuCharacterVR::JoinGameSession] Reached end! "))
}

void AMenuCharacterVR::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful) {
	if (bWasSuccessful) {
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(
				-1,
				5.f,
				FColor::Blue,
				FString::Printf(TEXT("[AMenuCharacterVR::OnCreateSessionComplete] Created session: %s"), *SessionName.ToString())
			);
		}
		UWorld* World = GetWorld();
		if (World) {
			// '/Game/Test_OnlineSubsystem/L_Game?listen'
			const bool bTravelResult = World->ServerTravel(FString("/Game/Test_OnlineSubsystem/L_Game?listen"));
			if (GEngine) {
				GEngine->AddOnScreenDebugMessage(
					-1,
					5.f,
					FColor::Red,
					FString::Printf(TEXT("[AMenuCharacterVR::OnCreateSessionComplete] Server travel: %s"),
						*FString::FromInt((int)bTravelResult))
				);
			}
		}
	} else {
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(
				-1,
				5.f,
				FColor::Red,
				FString(TEXT("[AMenuCharacterVR::OnCreateSessionComplete] Failed to create session!"))
			);
		}
	}
}

void AMenuCharacterVR::OnFindSessionsComplete(bool bWasSuccessful) {

	UE_LOG(LogTemp, Error, TEXT("[AMenuCharacterVR::OnFindSessionsComplete] OnFindSessionComplete->bWasSuccessful = %i"), bWasSuccessful)
	// if (GEngine) {
	// 	GEngine->AddOnScreenDebugMessage(
	// 		-1,
	// 		5.0f,
	// 		FColor::Cyan,
	// 		FString::Printf(TEXT("OnFindSessionComplete->bWasSuccessful: %i"), bWasSuccessful)
	// 	);
	// }
	//
	// if (!OnlineSessionInterface.IsValid())
	// {
	// 	UE_LOG(LogTemp, Error, TEXT("[AMenuCharacterVR::OnFindSessionsComplete] OnlineSessionInterface not valid!"))
	// 	return;
	// }
	//
	// const int NumSessionsFound = SessionSearch->SearchResults.Num(); 
	// if (GEngine) {
	// 	GEngine->AddOnScreenDebugMessage(
	// 		-1,
	// 		5.0f,
	// 		FColor::Cyan,
	// 		FString::Printf(TEXT("Found Sessions: %i"), NumSessionsFound)
	// 	);
	// }
	//
	// for (auto Result : SessionSearch->SearchResults) {
	// 	FString Id = Result.GetSessionIdStr();
	// 	FString User = Result.Session.OwningUserName;
	// 	FString MatchType;
	// 	Result.Session.SessionSettings.Get(FName("MatchType"), MatchType);
	//
	// 	if (GEngine) {
	// 		GEngine->AddOnScreenDebugMessage(
	// 			-1,
	// 			5.0f,
	// 			FColor::Cyan,
	// 			FString::Printf(TEXT("Id: %s, User: %s"), *Id, *User)
	// 		);
	// 	}
	//
	// 	if (MatchType == FString("FreeForAll")) {
	// 		if (GEngine) {
	// 			GEngine->AddOnScreenDebugMessage(
	// 				-1,
	// 				5.f,
	// 				FColor::Cyan,
	// 				FString::Printf(TEXT("Joining Match Type: %s"), *MatchType)
	// 			);
	// 		}
	//
	// 		OnlineSessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);
	//
	// 		const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	// 		OnlineSessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, Result);
	// 	}
	// }
}

void AMenuCharacterVR::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	// if (!OnlineSessionInterface.IsValid()) {
	// 	return;
	// }
	// FString Address;
	// if (OnlineSessionInterface->GetResolvedConnectString(NAME_GameSession, Address)) {
	// 	if (GEngine)
	// 	{
	// 		GEngine->AddOnScreenDebugMessage(
	// 			-1,
	// 			5.f,
	// 			FColor::Yellow,
	// 			FString::Printf(TEXT("Connect string: %s"), *Address)
	// 		);
	// 	}
	//
	// 	APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
	// 	if (PlayerController) {
	// 		PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
	// 	}
	// }
}

void AMenuCharacterVR::OnResetVR()
{
	// If MenuSystem is added to a project via 'Add Feature' in the Unreal Editor the dependency on HeadMountedDisplay in MenuSystem.Build.cs is not automatically propagated
	// and a linker error will result.
	// You will need to either:
	//		Add "HeadMountedDisplay" to [YourProject].Build.cs PublicDependencyModuleNames in order to build successfully (appropriate if supporting VR).
	// or:
	//		Comment or delete the call to ResetOrientationAndPosition below (appropriate if not supporting VR)
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AMenuCharacterVR::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void AMenuCharacterVR::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void AMenuCharacterVR::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AMenuCharacterVR::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AMenuCharacterVR::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AMenuCharacterVR::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}
