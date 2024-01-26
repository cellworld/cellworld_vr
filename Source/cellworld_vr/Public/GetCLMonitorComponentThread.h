#pragma once
#include "CoreMinimal.h"
#include "Kismet/GameplayStatics.h" 
#include "Core/Public/HAL/RunnableThread.h"
#include "Core/Public/HAL/Runnable.h"
#include "HeadMountedDisplayTypes.h" 
#include "HPGlia.h"
#include "HPGliaClient.h"
#include "GetCLMonitorComponent.h"
#include "IHeadMountedDisplay.h"
#include "IXRTrackingSystem.h"
#include "GameFramework/Actor.h"
//#include "GetPlayerTraversePath.generated.h"

class FRunnableThread;
class AGetCLMonitorComponentActor;

class CELLWORLD_VR_API FGetCLMonitorComponentThread : public FRunnable
{

public:
	// Sets default values for this actor's properties
	FGetCLMonitorComponentThread(AGetCLMonitorComponentActor* funActor, FString SaveDataDirectory);
	virtual ~FGetCLMonitorComponentThread();

	FString GetVariableHeader();

	bool ConnectToDevice();
	void Disconnect();


	// FRunnable functions 
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;

	bool bIsConnected;
	const FString clientID;
	const FString accessKey;
	const ELicensingModel requestedLicense = ELicensingModel::CORE;


	FEyeTracking EyeData;
	bool GetEyeTracking(FEyeTracking& OutEyeTracking);

	bool bStopThread = false;
	AGetCLMonitorComponentActor* CurrentThreadActor;
private:
	FXRHMDData HMDData;
	UWorld* World = nullptr;

protected:
	// Called when the game starts or when spawned
	FRunnableThread* Thread = nullptr;

	void ConvertCombinedGazeToLocation();
	bool SaveToString(FEyeTracking HPEye);
	bool PushDataToParentActor(FEyeTracking HPEye);
	FString ProjectDirectory;
	FString filename;;


	// to save data 
	int32 DataCount = 0;
	TArray<FVector> DataBuffer;

	// temp vars
	UWorld* tempWorld;
	APawn* tempPawn;

};
