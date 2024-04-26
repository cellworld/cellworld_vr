// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TextFileManager.generated.h"

/**
 * 
 */
UCLASS()
class CELLWORLD_VR_API UTextFileManager : public UBlueprintFunctionLibrary
{

public:
	GENERATED_BODY()
	UFUNCTION(BlueprintCallable, Category = "Custom", meta = (Keywords = "Save"))
	static bool SaveArrayText(FString SaveDirectory, FString FileName, TArray<FString> SaveText, bool AllowOverWriting);
	bool ReadFileToArray(FString LoadDirectory, TArray<FString>& outarr);
	static bool SaveTxt(FString SaveText, FString Filename);
	static bool SaveStringToFile(const FString& Line, const FString& Filepath, bool bAppend);
	TArray<FString> GetCSVFile(FString Path);
	static FString GetCurrentTimeString();
	static FString GetStandardHeader( 
		FString SubjectId, // current subject ID 
		FString WaypointOrder, // current waypoint order
		FString TrialNumber, // current trial number
		FString StartWaypointNumber, // starting waypoint number
		FString PhotosphereNum, // current photosphere number (leave as 0 if none, or can save next as 22_)
		FString TimeNow, // formatted string YYYYMMDD_HHMMSS.MS
		FString StartCodeUnique, // from actor calling this function 
		FString StartCodeGlobal, // from GS
		FString ExperimenterName,
		FString VariableLine);


	//UFUNCTION(BlueprintCallable, Category = "Custom", meta = (Keywords = "LoadCSV"))
	//TArray<FString> GetCSVFile(FString Path);

};