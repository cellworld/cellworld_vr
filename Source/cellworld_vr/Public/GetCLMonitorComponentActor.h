#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PawnMain.h"
#include "Core/Public/HAL/Runnable.h"
#include "Core/Public/HAL/RunnableThread.h"
#include "HeadMountedDisplayTypes.h" 
#include "IXRTrackingSystem.h"
#include "StereoRendering.h"
#include "IHeadMountedDisplay.h"
#include "ConfigManager.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraComponent.h"
#include "MouseKeyboardPlayerController.h"
#include "PlayerControllerVR.h"
#include "GetCLMonitorComponentThread.h"
#include "GetCLMonitorComponentActor.generated.h"

UCLASS()
class CELLWORLD_VR_API AGetCLMonitorComponentActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGetCLMonitorComponentActor();
	bool Calibrate();

private:
	FString SaveDataDirectory;

protected:
	// Called when the game starts or when spawned
	void InitThread();


	UWorld* WorldRef = nullptr;
	UWorld* tempWorld = nullptr;

	class FGetCLMonitorComponentThread* WorkerThread = nullptr;
	FRunnableThread* CurrentRunningThread = nullptr;

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	UWorld* GetWorldReferenceFromActor();

	/* to call end of thread */
	void Stop();

	/* used to draw line traces from eye */
	APawnMain* Pawn = nullptr;
	UCameraComponent* CameraComponent = nullptr;
	AMouseKeyboardPlayerController* MouseKeyboardPlayerController = nullptr; 
	APlayerControllerVR* PlayerControllerVR = nullptr;
	FVector eye_combined_gaze;
	FVector eye_left; 
	FVector eye_right; 
	FVector pupil_position_left; 
	FVector pupil_position_right;

	const int player_index = 0;
	bool IsVectorAllZeros(const FVector Vec);
	bool IsVectorAllNegativeOnes(const FVector Vec);
	bool DrawEyeTraceOnPlayer(float DeltaTime);

	/* helper functions for data  validation */
	bool IsEyeDataValid(FVector Vec); 
	bool IsWorldValid(UWorld*& World);
	bool GetPlayerCameraComponent(UCameraComponent*& Camera);
	bool IsTraceAvailable();

};
