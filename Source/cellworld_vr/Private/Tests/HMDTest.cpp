// Fill out your copyright notice in the Description page of Project Settings.


#include "Tests/HMDTest.h"
#include "TextFileManager.h"

/*
	Gets confidence values of HMD tracking values. 

	todo:	everything lol 
*/

FXRHMDData UHMDTest::HMDData; 

void UHMDTest::GetConfidenceValues()
{

	if (!UHMDTest::IsHMDEnabled()) {
		UE_LOG(LogTemp, Error, TEXT("[UHMDTest::GetConfidenceValues()] Return: Error. Check logs."));
		UE_DEBUG_BREAK();
		return;
	}

	///*static FXRHMDData */
	UHMDTest::HMDData = UHMDTest::GetHMDData();
	
	//return; 
	//if (!UHMDTest::HMDData.bValid) {
	//	UE_LOG(LogTemp, Error, TEXT("[UHMDTest::GetConfidenceValues()] Return: HMDData.bValid = false."));
	//	//return;
	//}

	//if (!UHMDTest::SaveData(UHMDTest::HMDData)) {
	//	UE_LOG(LogTemp, Error, TEXT("[UHMDTest::GetConfidenceValues()] Return: SaveData failed."));
	//	//UE_DEBUG_BREAK(); 
	//}

	if (GEngine) {
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Example text that prints a float: %f"), 
			//HMDData.Confide);
	}
	
	return; 
}

/* 
	Check if HMD is enabled

	todo:	change this to 1 or two ifs after 
			testing if they all work 
*/
bool UHMDTest::IsHMDEnabled()
{
	/* enable hmd */
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

/*
	Get FXRHMDData 

	todo: "this" pointer invalid !!! 
*/
FXRHMDData UHMDTest::GetHMDData()
{
	FXRHMDData HMDDataTemp;

	if (!UHMDTest::IsHMDEnabled()) {
		return HMDDataTemp;
	}

	if (!GEngine->XRSystem.IsValid() || !GEngine->XRSystem->GetHMDDevice() || !GEngine->StereoRenderingDevice.IsValid()) {
		UE_DEBUG_BREAK();
	}

	IXRTrackingSystem* XRSystem = GEngine->XRSystem.Get();
	
	if (!XRSystem) {
		UE_LOG(LogTemp, Error, TEXT("[UHMDTest::GetHMDData()] XR System failed.."));
	}

	/* good until here */

	if (!UHeadMountedDisplayFunctionLibrary::HasValidTrackingPosition()) {
		return HMDDataTemp;
	}

	if (!GEngine->GetWorld()) {
		UE_LOG(LogTemp, Error, TEXT("[UHMDTest::GetHMDData()] World Not valid."));
		return HMDDataTemp; 
	}

	UWorld* World = GEngine->GetWorld(); 

	if (!World->IsValidLowLevelFast()) {
		UE_LOG(LogTemp, Error, TEXT("[UHMDTest::GetHMDData()] World Not valid."));
		return HMDDataTemp;
	}

	XRSystem->GetHMDData(World, HMDDataTemp);

	return HMDDataTemp;
}

static FString f2s(float v) {
	return FString::SanitizeFloat(v);
}

bool UHMDTest::SaveData(FXRHMDData Data)
{
	/* header: 
		time,px,py,pz,rx,ry,rz,tracking_status, 
	//*/
	FString ProjectDirectory = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());

	FString filename = ProjectDirectory + "Data/HMDtest.csv"; // todo: if Data/ doesnt exists -> create folder 
	FString delim = ",";

	FVector p  = Data.Position; 
	FVector r = Data.Rotation.Euler();

	FString line = "time" + delim
		+ f2s(p.X) + delim + f2s(p.Y) + delim + f2s(p.Z) + delim
		+ f2s(r.X) + delim + f2s(r.Y) + delim + f2s(r.Z) + "\n";

	return UTextFileManager::SaveTxt(*line, *filename);
}

void UHMDTest::test()
{
}

