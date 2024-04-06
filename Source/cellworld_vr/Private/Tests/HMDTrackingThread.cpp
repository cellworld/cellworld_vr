// Fill out your copyright notice in the Description page of Project Settings.


#include "Tests/HMDTrackingThread.h"
#include "Async/Async.h"


// Sets default values
FHMDTrackingThread::FHMDTrackingThread(AHMDTrackingThreadActor* InOwnerActor)
{
	OwnerActor = InOwnerActor;
}

FHMDTrackingThread::~FHMDTrackingThread()
{
	// Destructor
	if (Thread != nullptr)
	{
		Thread->Kill(true);
		delete Thread;
	}
}


uint32 FHMDTrackingThread::Run()
{
	ProjectDirectory = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
	filename = ProjectDirectory + "Data/TestHMDTracking.csv"; // todo: if Data/ doesnt exists -> create folder 
	//FString save_line = FGetCLMonitorComponentThread::GetVariableHeader();
	//DataReceivedEvent.AddDynamic(this, &FHMDTrackingThread::GetHMDData);

	/* check HMD */
	/*if (!GEngine->XRSystem || !GEngine->XRSystem->GetHMDDevice() || !GEngine->XRSystem->GetHMDDevice()->IsHMDConnected())
	{
		bStopThread = true;
		UE_LOG(LogTemp, Warning, TEXT("[FGetCLMonitorComponentThread::Run()] HMD NOT connected."))
	}*/

	if (!GEngine) { 
		UE_LOG(LogTemp, Warning, TEXT("[FHMDTrackingThread::Run()] GEngine failed."))
		bStopThread = true;
	}

	if (!GEngine->XRSystem->GetHMDDevice() || !GEngine->XRSystem->GetHMDDevice()) {
		bStopThread = true;
		UE_LOG(LogTemp, Warning, TEXT("[FHMDTrackingThread::Run()] HMD NOT connected."))
	}

	/* check omnicept runtime connection */

	/* to do: change bStopThread to GS */
	while (!bStopThread) {

		/* collect sample */
		HMDData = FHMDTrackingThread::GetHMDData();
		/* push sample GetCLMonitorActor*/
		//if (!FGetCLMonitorComponentThread::PushDataToParentActor(HPEye)) { UE_DEBUG_BREAK(); }

		/* save the data */
		//if (!FGetCLMonitorComponentThread::SaveToString(HPEye)) { UE_DEBUG_BREAK(); }

		FPlatformProcess::Sleep(0.1f); // sleep 
	}

	return 0;
}

void FHMDTrackingThread::Stop()
{
	bStopThread = true;
}

void FHMDTrackingThread::Exit()
{
}

UWorld* FHMDTrackingThread::GetWorldFromActor(AActor* Actor)
{

	return Actor->GetWorld();

}

bool FHMDTrackingThread::IsHMDEnabled()
{

	/* enable hmd */
	if (!OwnerActor) { return false;  }

	if (!OwnerActor->GetWorld()) { 
		UE_LOG(LogTemp, Error, TEXT("[UHMDTest::IsHMDEnabled()] XRSystem.IsValid() Failed."));
		return false; 
	}

	if (!GEngine || !GEngine->XRSystem.IsValid()) {
		UE_LOG(LogTemp, Error, TEXT("[UHMDTest::IsHMDEnabled()] XRSystem.IsValid() Failed."));
		return false;
	}

	if (!GEngine->XRSystem->GetHMDDevice()) {
		UE_LOG(LogTemp, Error, TEXT("[UHMDTest::IsHMDEnabled()] GetHMDDEvice() Failed."));
		return false;
	}

	if (!GEngine->StereoRenderingDevice.IsValid()) {
		UE_LOG(LogTemp, Error, TEXT("[UHMDTest::IsHMDEnabled()] StereoRenderingDevice.IsValid() Failed."));
		return false;
	}

	if (!GEngine->XRSystem->IsHeadTrackingAllowed()) {
		UE_LOG(LogTemp, Error, TEXT("[UHMDTest::IsHMDEnabled()] IsHeadTrackingAllowed() failed."));
		return false;
	}

	return true;
}

/* get FXRHMDData */
FXRHMDData FHMDTrackingThread::GetHMDData()
{
	//AsyncTask(ENamedThreads::GameThread, [&]() {

	//}

	FXRHMDData HMDDataTemp;

	if (!FHMDTrackingThread::IsHMDEnabled()) {
		return HMDDataTemp;
	}

	if (!GEngine->XRSystem.IsValid() || !GEngine->XRSystem->GetHMDDevice() || !GEngine->StereoRenderingDevice.IsValid()) {
		UE_DEBUG_BREAK();
	}

	IXRTrackingSystem* XRSystem = GEngine->XRSystem.Get();

	if (!XRSystem) {
		UE_LOG(LogTemp, Error, TEXT("[UHMDTest::GetHMDData()] XR System failed.."));
		return HMDDataTemp;
	}

	// needs to be run on mainthread
	if (!UHeadMountedDisplayFunctionLibrary::HasValidTrackingPosition()) {
		return HMDDataTemp;
	}

	if (!OwnerActor->GetWorld()) {
		UE_LOG(LogTemp, Error, TEXT("[UHMDTest::GetHMDData()] World Not valid."));
		return HMDDataTemp;
	}
	
	/* get world ref */
	World = OwnerActor->GetWorld();

	if (!World->IsValidLowLevelFast()) {
		UE_LOG(LogTemp, Error, TEXT("[UHMDTest::GetHMDData()] World Not valid."));
		return HMDDataTemp;
	}

	XRSystem->GetHMDData(World, HMDDataTemp);

	return HMDDataTemp;
}

void FHMDTrackingThread::GetHMDDataAsync(FXRHMDData& Data)
{

}

bool FHMDTrackingThread::SaveToString(FXRHMDData Data) {
	//FString line = HPEye.SystemTime.ToString() + "," +
	//	FString::SanitizeFloat(HPEye.CombinedGaze.X) + "," + FString::SanitizeFloat(HPEye.CombinedGaze.Y) + "," + FString::SanitizeFloat(HPEye.CombinedGaze.Z) + "," +
	//	FString::SanitizeFloat(HPEye.LeftGaze.X) + "," + FString::SanitizeFloat(HPEye.LeftGaze.Y) + "," + FString::SanitizeFloat(HPEye.LeftGaze.Z) + "," + FString::SanitizeFloat(HPEye.LeftGazeConfidence) + "," +
	//	FString::SanitizeFloat(HPEye.RightGaze.X) + "," + FString::SanitizeFloat(HPEye.RightGaze.Y) + "," + FString::SanitizeFloat(HPEye.RightGaze.Z) + "," + FString::SanitizeFloat(HPEye.RightGazeConfidence) + "," +
	//	FString::SanitizeFloat(HPEye.LeftPupilPosition.X) + "," + FString::SanitizeFloat(HPEye.LeftPupilPosition.Y) + "," + FString::SanitizeFloat(HPEye.LeftPupilPosition.Z) + "," + FString::SanitizeFloat(HPEye.LeftPupilPositionConfidence) + "," +
	//	FString::SanitizeFloat(HPEye.RightPupilPosition.X) + "," + FString::SanitizeFloat(HPEye.RightPupilPosition.Y) + "," + FString::SanitizeFloat(HPEye.RightPupilPosition.Z) + "," + FString::SanitizeFloat(HPEye.RightPupilPositionConfidence) + "," +
	//	FString::SanitizeFloat(HPEye.LeftPupilDilation) + "," + FString::SanitizeFloat(HPEye.LeftPupilDilationConfidence) + "," + FString::SanitizeFloat(HPEye.RightPupilDilation) + "," + FString::SanitizeFloat(HPEye.RightPupilDilationConfidence) + "," +
	//	FString::SanitizeFloat(HPEye.LeftEyeOpenness) + "," + FString::SanitizeFloat(HPEye.LeftEyeOpennessConfidence) + "," + FString::SanitizeFloat(HPEye.RightEyeOpenness) + "," + FString::SanitizeFloat(HPEye.RightEyeOpennessConfidence) + "\n";

	//return UTextFileManager::SaveTxt(*line, *filename);
	return false; 
}


