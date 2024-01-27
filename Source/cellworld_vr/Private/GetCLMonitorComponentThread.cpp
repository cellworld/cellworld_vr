#pragma once 
#include "GetCLMonitorComponentThread.h" 
#include "GameStateMain.h"
#include "GameModeMain.h"
#include "MouseKeyboardPlayerController.h"
#include "GetCLMonitorComponentActor.h"
#include "EyeTrackerFunctionLibrary.h"
//#include "GameInstanceMain.h"
#include <iostream>
#include <sstream>
#include "TextFileManager.h"
#include "Misc/DateTime.h" 
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Kismet/GameplayStatics.h" 
#include "Kismet/KismetStringLibrary.h"

FGetCLMonitorComponentThread::FGetCLMonitorComponentThread(AGetCLMonitorComponentActor* funActor, FString SaveDataDirectory)
{
	CurrentThreadActor = funActor;
	/* init eye-tracker */
}

bool FGetCLMonitorComponentThread::ConnectToDevice() {
	const FString access_key = "F8OK38DWnRgqJgr5aaUhgcfBPHoEe5toBiDGGREkR2DWeZxgTKFpCF5YvAdnHd-S";
	const FString client_id = "25b17c6b-3386-45f8-9e1e-88d76259b5bf";
	UHPGliaClient::ConnectToGliaAsync(client_id, access_key, ELicensingModel::CORE);

	bIsConnected = UHPGliaClient::IsConnected();
	/* to do: check connection, returns false but I get data that matches the demo...? confusing...*/
	if (!bIsConnected) { UE_LOG(LogTemp, Error, TEXT("[FGetCLMonitorComponentThread::ConnectToDevice()] HPGlia Client connection: FAILED.")); }
	else { UE_LOG(LogTemp, Log, TEXT("[FGetCLMonitorComponentThread::ConnectToDevice()] HPGlia Client connection: SUCCESS.")); }

	return bIsConnected;
}

bool FGetCLMonitorComponentThread::GetEyeTracking(FEyeTracking& OutEyeTracking) {
	return UHPGliaClient::GetEyeTracking(EyeData);
}

void FGetCLMonitorComponentThread::ConvertCombinedGazeToLocation()
{
}



FGetCLMonitorComponentThread::~FGetCLMonitorComponentThread()
{
	// Destructor
	if (Thread != nullptr)
	{
		Thread->Kill(true);
		delete Thread;
	}
}

FString FGetCLMonitorComponentThread::GetVariableHeader() 
{
	const FString H1 = "SystemTime,CombinedGaze_x,CombinedGaze_y,CombinedGaze_z,CombinedGaze_Confidence,LeftGaze_x,LeftGaze_y, LeftGaze_z,LeftGaze_Confidence,RightGaze_x,RightGaze_y,RightGaze_z,RightGaze_Confidence,";
	const FString H2 = "LeftPupilPosition_x,LeftPupilPosition_y,LeftPupilPosition_z,LeftPupilPosition_Confidence,RightPupilPosition_x,RightPupilPosition_y,RightPupilPosition_z,RightPupilPosition_Confidence,";
	const FString H3 = "LeftPupilDilation,LeftPupilDilation_Confidence,RightPupilDilation,RightPupilDilation_Confidence,LeftEyeOpenness,LeftEyeOpenness_Confidence,RightEyeOpenness,RightEyeOpenness_Confidence\n";
	return H1 + H2 + H3;
}

void FGetCLMonitorComponentThread::Disconnect() {
	UHPGliaClient::DisconnectFromGlia();
}

bool FGetCLMonitorComponentThread::SaveToString(FEyeTracking HPEye) {
	FString line = HPEye.SystemTime.ToString() + "," +
		FString::SanitizeFloat(HPEye.CombinedGaze.X) + "," + FString::SanitizeFloat(HPEye.CombinedGaze.Y) + "," + FString::SanitizeFloat(HPEye.CombinedGaze.Z) + "," +
		FString::SanitizeFloat(HPEye.LeftGaze.X) + "," + FString::SanitizeFloat(HPEye.LeftGaze.Y) + "," + FString::SanitizeFloat(HPEye.LeftGaze.Z) + "," + FString::SanitizeFloat(HPEye.LeftGazeConfidence) + "," +
		FString::SanitizeFloat(HPEye.RightGaze.X) + "," + FString::SanitizeFloat(HPEye.RightGaze.Y) + "," + FString::SanitizeFloat(HPEye.RightGaze.Z) + "," + FString::SanitizeFloat(HPEye.RightGazeConfidence) + "," +
		FString::SanitizeFloat(HPEye.LeftPupilPosition.X) + "," + FString::SanitizeFloat(HPEye.LeftPupilPosition.Y) + "," + FString::SanitizeFloat(HPEye.LeftPupilPosition.Z) + "," + FString::SanitizeFloat(HPEye.LeftPupilPositionConfidence) + "," +
		FString::SanitizeFloat(HPEye.RightPupilPosition.X) + "," + FString::SanitizeFloat(HPEye.RightPupilPosition.Y) + "," + FString::SanitizeFloat(HPEye.RightPupilPosition.Z) + "," + FString::SanitizeFloat(HPEye.RightPupilPositionConfidence) + "," +
		FString::SanitizeFloat(HPEye.LeftPupilDilation) + "," + FString::SanitizeFloat(HPEye.LeftPupilDilationConfidence) + "," + FString::SanitizeFloat(HPEye.RightPupilDilation) + "," + FString::SanitizeFloat(HPEye.RightPupilDilationConfidence) + "," +
		FString::SanitizeFloat(HPEye.LeftEyeOpenness) + "," + FString::SanitizeFloat(HPEye.LeftEyeOpennessConfidence) + "," + FString::SanitizeFloat(HPEye.RightEyeOpenness) + "," + FString::SanitizeFloat(HPEye.RightEyeOpennessConfidence) + "\n";

	/* print eye data */
	//UE_LOG(LogTemp, Log, TEXT("[FGetCLMonitorComponentThread::Run()] HPGlia::GetEyeTracking() saving line -> %s"), *save_line);
	//IFileManager* FileManager;
	//if (!FileManager) { 
	//	return false; 
	//}
	return UTextFileManager::SaveTxt(*line, *filename);
}

bool FGetCLMonitorComponentThread::PushDataToParentActor(FEyeTracking HPEye) {
	if (!CurrentThreadActor) { return false; }
	CurrentThreadActor->eye_left = HPEye.LeftGaze;
	CurrentThreadActor->eye_right = HPEye.RightGaze;
	CurrentThreadActor->pupil_position_left = HPEye.LeftPupilPosition;
	CurrentThreadActor->pupil_position_right = HPEye.RightPupilPosition;
	CurrentThreadActor->eye_combined_gaze = HPEye.CombinedGaze;

	return true;
}

uint32 FGetCLMonitorComponentThread::Run()
{
	ProjectDirectory = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
	filename = ProjectDirectory + "Data/data.csv";
	FString save_line = FGetCLMonitorComponentThread::GetVariableHeader();
		
	UE_LOG(LogTemp, Warning, TEXT("[FGetCLMonitorComponentThread::Run()] HPGlia::GetEyeTracking() saving line -> %s"), *save_line);

	/* check omnicept runtime connection */
	FGetCLMonitorComponentThread::ConnectToDevice();

	/* check HMD */
	if (!GEngine->XRSystem->GetHMDDevice()->IsHMDConnected()) {
		UE_LOG(LogTemp, Warning, TEXT("[FGetCLMonitorComponentThread::Run()] HMD NOT connected."));
		UE_DEBUG_BREAK();
	}

	/* to do: change bStopThread to GS */
	//int i = 0; 
	while (!bStopThread) {

		/* collect sample */
		FEyeTracking HPEye;
		UHPGliaClient::GetEyeTracking(HPEye);


		/* push sample GetCLMonitorActor*/
		if (!FGetCLMonitorComponentThread::PushDataToParentActor(HPEye)) { UE_DEBUG_BREAK(); }

		/* save the data */
		if (!FGetCLMonitorComponentThread::SaveToString(HPEye)) { UE_DEBUG_BREAK(); }

		/* debug*/
		//UE_LOG(LogTemp, Warning, TEXT("[FGetCLMonitorComponentThread::Run()] left: %f, %f, %f."), HPEye.LeftGaze.X, HPEye.LeftGaze.Y, HPEye.LeftGaze.Z);

		FPlatformProcess::Sleep(0.1f); // sleep 
	}

	Disconnect();
	return 0;
}

void FGetCLMonitorComponentThread::Stop()
{
	bStopThread = true;
}

void FGetCLMonitorComponentThread::Exit()
{
}
