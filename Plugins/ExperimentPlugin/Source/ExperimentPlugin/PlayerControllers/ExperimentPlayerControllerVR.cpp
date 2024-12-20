#include "ExperimentPlayerControllerVR.h"
#include "InputMappingContext.h"
#include "ExperimentPlugin/Characters/ExperimentCharacter.h"
#include "ExperimentPlugin/PlayerStates/ExperimentPlayerState.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

AExperimentPlayerControllerVR::AExperimentPlayerControllerVR() {
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> IMPClassObject(TEXT("InputMappingContext'/Game/SpatialAnchorsSample/Inputs/Mappings/IMC_VRPawn'"));
	if (IMPClassObject.Succeeded()) {
		DefaultMappingContext = IMPClassObject.Object;
	}
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentPlayerControllerVR::AExperimentPlayerControllerVR] DefaultMappingContext found? %s"),
		DefaultMappingContext ? *FString("valid") : *FString("NULL"));
	
}

void AExperimentPlayerControllerVR::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, PossessedCharacter)
}

void AExperimentPlayerControllerVR::PostInitializeComponents() {
	Super::PostInitializeComponents();
	const FInputModeGameOnly InputModeGameOnly; 
	// SetInputMode(InputModeGameOnly);
}

void AExperimentPlayerControllerVR::BeginPlay() {
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentPlayerControllerVR::BeginPlay]"));
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentPlayerControllerVR::BeginPlay] bReplicates: %s"),
			bReplicates ? *FString("valid") : *FString("NULL"));
	
	if (GetWorld()) {
		PossessedCharacter = Cast<AExperimentCharacter>(GetCharacter());
	}else {
		UE_LOG(LogTemp, Error, TEXT("[AExperimentPlayerControllerVR::BeginPlay] GetWorld() NULL"))
	}
	
	// if (UWorld* World = GetWorld()) {
	// 	if (UGameViewportClient* ViewportClient = World->GetGameViewport()) {
	// 		ViewportClient->RemoveAllViewportWidgets();
	// 		UE_LOG(LogTemp, Log,TEXT("[AExperimentPlayerControllerVR::BeginPlay] Removed all widgets from viewport."))
	// 	}
	// }
	return;
}

// Called to bind functionality to input
void AExperimentPlayerControllerVR::SetupInputComponent() {
	Super::SetupInputComponent();
	UE_LOG(LogTemp, Log, TEXT("[AExperimentPlayerControllerVR::SetupInputComponent] Called"))

	// InputComponent->BindAction("ResetOrigin", EInputEvent::IE_Pressed, this, &AExperimentPlayerControllerVR::ResetOrigin);
	// InputComponent->BindAction("QuitGame", EInputEvent::IE_Pressed, this, &AExperimentPlayerControllerVR::QuitGame);
	// InputComponent->BindAction("RestartGame", EInputEvent::IE_Pressed, this, &AExperimentPlayerControllerVR::RestartGame);
	// InputComponent->BindAxis("MoveForward",this, &AExperimentPlayerControllerVR::MoveForward);
	// InputComponent->BindAxis("MoveRight",this, &AExperimentPlayerControllerVR::MoveRight);
	SetReplicateMovement(true);
}

void AExperimentPlayerControllerVR::RestartGame() {
	if(GEngine)
		GEngine->AddOnScreenDebugMessage(2, 1.0f, FColor::Yellow, TEXT("AExperimentPlayerControllerVR::RestartGame"));
}

bool AExperimentPlayerControllerVR::Client_SetInputModeGameOnly_Validate() {
	UE_LOG(LogTemp, Log,TEXT("[AExperimentPlayerControllerVR::Client_SetInputModeGameOnly_Validate] Called"))
	return true;
}

void AExperimentPlayerControllerVR::Client_SetInputModeGameOnly_Implementation() {
	UE_LOG(LogTemp, Log,TEXT("[AExperimentPlayerControllerVR::Client_SetInputModeGameOnly_Implementation] Called"))
	if (!HasAuthority()) {
		UE_LOG(LogTemp, Log,TEXT("[AExperimentPlayerControllerVR::Client_SetInputModeGameOnly_Implementation] Running from client"))
		const FInputModeGameOnly InputModeGameOnly; 
		this->SetInputMode(InputModeGameOnly);
		SetShowMouseCursor(false);
		EnableInput(this);
		SetMotionControlsEnabled(true);
		UE_LOG(LogTemp, Log,
			TEXT("[AExperimentPlayerControllerVR::Client_SetInputModeGameOnly_Implementation] Current input mode: %s"),
			*GetCurrentInputModeDebugString())
	}
}

void AExperimentPlayerControllerVR::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
	if (!PossessedCharacter) {
		PossessedCharacter = Cast<AExperimentCharacter>(GetCharacter());
		UE_LOG(LogTemp, Error, TEXT("[AExperimentPlayerControllerVR::Tick] PossessedPawn NULL. Recasting."))
	}

	// AExperimentPlayerState* PlayerStateTemp = GetPlayerState<AExperimentPlayerState>(); 
	// UE_LOG(LogTemp, Log, TEXT("[AExperimentPlayerControllerVR::Tick] IsSpectator? %s"),
	// 	PlayerStateTemp ? PlayerStateTemp->IsSpectator() : TEXT("PlayerState NULL"))
	
	UE_LOG(LogTemp, Log, TEXT("[AExperimentPlayerControllerVR::Tick] bIsLocalPlayerController = %s"),
			IsLocalPlayerController() ? TEXT("true") : TEXT("false"))

	UE_LOG(LogTemp, Log, TEXT("[AExperimentPlayerControllerVR::Tick] PC (%s) controlling pawn (%s)"),
			*GetName(), GetPawn() ? *GetPawn()->GetName() : TEXT("NULL"))

}

void AExperimentPlayerControllerVR::OnPossess(APawn* InPawn) {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentPlayerControllerVR::OnPossess]"))
	if (!HasAuthority() || !InPawn) return;
	
	PossessedCharacter = Cast<AExperimentCharacter>(InPawn);
	UE_LOG(LogTemp, Log, TEXT("[AExperimentPlayerControllerVR::OnPossess] Called. Pawn: %s"), *PossessedCharacter->GetName())
	
	Super::OnPossess(InPawn);
	if (InPawn->GetNetOwner() != this) {
		UE_LOG(LogTemp, Log, TEXT("[AExperimentPlayerControllerVR::OnPossess] InPawn's NetOwner is not `this`. Calling SetOwner(this)"))
		InPawn->SetOwner(this);
		InPawn->ForceNetUpdate();
	}
	
	UE_LOG(LogTemp, Log, TEXT("[AExperimentPlayerControllerVR::OnPossess] After SetOwner() and AFTER Super::"))
	
	if (InPawn->GetNetOwner() != this) {
		UE_LOG(LogTemp, Error, TEXT("[AExperimentPlayerControllerVR::OnPossess] Pawn's net owner is not current PC."))
	}else {
		UE_LOG(LogTemp, Log, TEXT("[AExperimentPlayerControllerVR::OnPossess] Pawn (%s) already has `this` (%s) as net owner (%s)"),
			*InPawn->GetName(), *GetName(), *InPawn->GetNetOwner()->GetName())
	}
	Client_SetInputModeGameOnly();
}

void AExperimentPlayerControllerVR::ResetOrigin() {
	if (PossessedCharacter) {
		//FVector TargetLocation = ...; // Set this to your desired world location
		//FVector Direction = (TargetLocation - GetActorLocation()).GetSafeNormal();
		//FRotator NewRotation = Direction.Rotation();
		//SetActorRotation(NewRotation);
	}
}

void AExperimentPlayerControllerVR::QuitGame() {
	/*FGenericPlatformMisc::RequestExit(false);*/
	UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit,false);
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentPlayerControllerVR::QuitGame] Requesting Exit."));
}

void AExperimentPlayerControllerVR::MoveForward(float AxisValue) {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentPlayerControllerVR::MoveForward]"))

	if(GEngine) {
		GEngine->AddOnScreenDebugMessage(1, 1.0f, FColor::Yellow, TEXT("[AExperimentPlayerControllerVR::MoveForward]"));	}
	
	if (PossessedCharacter) {
		PossessedCharacter->MoveForward(AxisValue);
	}
}

void AExperimentPlayerControllerVR::MoveRight(float AxisValue) {
	if (PossessedCharacter) {
		PossessedCharacter->MoveRight(AxisValue);
	}
}
