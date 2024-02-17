// Fill out your copyright notice in the Description page of Project Settings.


#include "PredatorController/ExperimentServiceMonitor.h"
#include "ExperimentUtils.h"
//#include "GenericPlatform/GenericPlatformProcess.h"


// Sets default values
AExperimentServiceMonitor::AExperimentServiceMonitor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}
bool AExperimentServiceMonitor::SpawnAndPossessPredator() {

	if (GEngine->GetWorld())
	{
		// Define spawn parameters
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		// Specify the location and rotation for the new actor
		FVector Location(-2426.0f, 1264.0f, 90.0f); // Change to desired spawn location
		FRotator Rotation(0.0f, 0.0f, 0.0f); // Change to desired spawn rotation

		// Spawn the character
		PredatorCharacter = GEngine->GetWorld()->SpawnActor<ACharacterPredator>(ACharacterPredator::StaticClass(), Location, Rotation, SpawnParams);

		// Ensure the character was spawned
		if (!PredatorCharacter)
		{
			return false;
		}
	}
	return true;
}
/* subscribes to server and calls UpdatePredator() when messages[header] matches input header. */
bool AExperimentServiceMonitor::SubscribeToServer(FString header)
{
	PredatorMessageClient->Subscribe();

	if (!PredatorMessageClient) { return false; }
	MessageRoute = PredatorMessageClient->AddRoute(header);
	
	if (!MessageRoute) { return false; }
	MessageRoute->MessageReceived.AddDynamic(this, &AExperimentServiceMonitor::UpdatePredator);

	return true;
}

bool AExperimentServiceMonitor::ServerConnect()
{
	PredatorMessageClient = UMessageClient::NewMessageClient();

	if (PredatorMessageClient == nullptr){
		return false; 
	}

	bConnectedToServer = PredatorMessageClient->Connect(ServerIP, ServerPort);
	return bConnectedToServer;
}

void AExperimentServiceMonitor::ServerConnectAttempts(int attempts)
{
	int i = 0; 
	while (i <= attempts) {
		GEngine->AddOnScreenDebugMessage(1, 2.0f, FColor::Red, TEXT("[AExperimentServiceMonitor::ServerConnectAttempts] Trying to connect."));
		this->ServerConnect();
		i++; 

		if (bConnectedToServer) { 
			GEngine->AddOnScreenDebugMessage(1, 5.0f, FColor::Green, TEXT("Predator controller connection: Success."));
			return; 
		}
	}
}

void AExperimentServiceMonitor::UpdatePredator(FMessage message)
{
	if (PredatorMessageClient == nullptr) { return; }
	n_samples++;
	FStep step = UExperimentUtils::JsonStringToStep(message.body); 
	FVector new_location_ue = UExperimentUtils::canonicalToVr(step.location,map_length); // ue --> unreal engine units 

	GEngine->AddOnScreenDebugMessage(1, 5.0f, FColor::Green, FString::Printf(TEXT("New location: %f %f %f"), new_location_ue.X, new_location_ue.Y, new_location_ue.Z));
	return;
}

// Called when the game starts or when spawned
void AExperimentServiceMonitor::BeginPlay()
{
	Super::BeginPlay();

	if (!this->SpawnAndPossessPredator()) {
		UE_DEBUG_BREAK();
	}

	this->ServerConnectAttempts(5);

	if (!bConnectedToServer) {
		GEngine->AddOnScreenDebugMessage(1, 2.0f, FColor::Red, TEXT("[AExperimentServiceMonitor::BeginPlay()] Failed to connect to server."));
	}
	this->SubscribeToServer(predator_step_header);
}

void AExperimentServiceMonitor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!bConnectedToServer) { return; }
}

