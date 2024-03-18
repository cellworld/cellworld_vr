// Fill out your copyright notice in the Description page of Project Settings.


#include "Tests/TestActor.h"
#include "TextFileManager.h"
#include "Utils/QueryPerformanceCounter.h"

// Sets default values
ATestActor::ATestActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	ProjectDirectory = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir());
	filename = ProjectDirectory + "Data/HMDtest_" + UTextFileManager::GetCurrentTimeString() + ".csv"; // todo: if Data/ doesnt exists -> create folder 

}

// Called when the game starts or when spawned
void ATestActor::BeginPlay()
{
	Super::BeginPlay();
	bQuery_count_init_valid = UQueryPerformanceCounter::GetCounter2(query_count_init);
	/* save header*/
	const FString header = "time,px,py,pz,rx,ry,rz,tracking_status\n";
	UTextFileManager::SaveStringToFile(*header, *filename, false);
}

void ATestActor::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason); 
}

// Called every frame
void ATestActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	ATestActor::GetConfidenceValues();
}

void ATestActor::GetConfidenceValues()
{

	if (!this->IsHMDEnabled()) {
		UE_LOG(LogTemp, Error, TEXT("[UHMDTest::GetConfidenceValues()] Return: Error. Check logs."));
		return;
	}

	HMDData = ATestActor::GetHMDData();

	if (!ATestActor::HMDData.bValid) {
		UE_LOG(LogTemp, Error, TEXT("[UHMDTest::GetConfidenceValues()] Return: HMDData.bValid = false."));
		return;
	}

	if (!ATestActor::SaveData(HMDData)) {
		UE_LOG(LogTemp, Error, TEXT("[UHMDTest::GetConfidenceValues()] Return: SaveData failed."));
	}

	return;
}

/*
	Check if HMD is enabled

	todo:	change this to 1 or two ifs after
			testing if they all work
*/
bool ATestActor::IsHMDEnabled()
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

	if (!UHeadMountedDisplayFunctionLibrary::HasValidTrackingPosition()) {
		UE_LOG(LogTemp, Error, TEXT("[ATestActor::GetHMDData()] HasValidTrackingPosition failed."));
		return false;
	}

	return true;
}

/*
	Get FXRHMDData
*/
FXRHMDData ATestActor::GetHMDData()
{
	FXRHMDData HMDDataTemp;

	if (!ATestActor::IsHMDEnabled()) {
		return HMDDataTemp;
	}

	IXRTrackingSystem* XRSystem = GEngine->XRSystem.Get();

	if (!GetWorld() || !GetWorld()->IsValidLowLevelFast()) {
		UE_LOG(LogTemp, Error, TEXT("[UHMDTest::GetHMDData()] World Not valid."));
		return HMDDataTemp;
	}

	UWorld* World = GetWorld();

	XRSystem->GetHMDData(World, HMDDataTemp);

	return HMDDataTemp;
}

static FString f2s(float v) {
	return FString::SanitizeFloat(v);
}

static FString TrackingEnumToString(ETrackingStatus TrackingStatus) {
	
	switch (TrackingStatus)
	{
	case ETrackingStatus::InertialOnly:
		return "InertialOnly";
	case ETrackingStatus::NotTracked:
		return "NotTracked";
	case ETrackingStatus::Tracked:
		return "Tracked";
	}
	ensure(false);
	return TEXT("Unknown");
	
}

bool ATestActor::SaveData(FXRHMDData Data)
{
	/* header:
		times,px,py,pz,rx,ry,rz,tracking_status,
	//*/


	FVector p = Data.Position;
	FVector r = Data.Rotation.Euler();

	uint64 query_count_now = 0;
	double query_count_elapsed;

	if (!bQuery_count_init_valid || !UQueryPerformanceCounter::GetCounter2(query_count_now) || !UQueryPerformanceCounter::GetQueryElapsedTime(query_count_init, query_count_elapsed)) {
		query_count_elapsed = 0;
	}
	FString line = FString::SanitizeFloat(query_count_elapsed) + delim
		+ f2s(p.X) + delim + f2s(p.Y) + delim + f2s(p.Z) + delim
		+ f2s(r.X) + delim + f2s(r.Y) + delim + f2s(r.Z) + delim + TrackingEnumToString(Data.TrackingStatus) + "\n";

	return UTextFileManager::SaveStringToFile(*line, *filename, true);
}

