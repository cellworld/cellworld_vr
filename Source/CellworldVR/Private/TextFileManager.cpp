// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "TextFileManager.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h" 
#include "Misc/Paths.h"

bool UTextFileManager::SaveArrayText(FString SaveDirectory, FString FileName, TArray<FString> SaveText, bool AllowOverWriting = false)
{
	
	// Filepath 
	SaveDirectory += FileName;
	FString FileContent;
	//FString VarNames = "gaze_origin_x, gaze_origin_y, gaze_origin_z, gaze_direction_x, gaze_direction_y, //gaze_direction_z, fixation_point_x, fixation_point_y, fixation_point_z, confidence_value";

	// check if we overwrite
	if (!AllowOverWriting) {
		// check if exists
		if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*SaveDirectory)) {
			// check if we were able to load sucessfully
			if (!FFileHelper::LoadFileToString(FileContent, *SaveDirectory, FFileHelper::EHashOptions::None))
			{
				UE_LOG(LogTemp, Warning, TEXT("FileManipulation: Text From File: %s"), *FileContent);
				//FileContent += VarNames;
				FileContent += LINE_TERMINATOR;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("FileManipulation: Did not load text from file"));

			}
		}
	}	


	FString FinalString = *FileContent;
	for (FString& Each : SaveText) {
		FinalString += Each;
		FinalString += LINE_TERMINATOR;
	}
	return FFileHelper::SaveStringToFile(FinalString, *SaveDirectory);

}


TArray<FString> UTextFileManager::GetCSVFile(FString Path)
{
	//GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("GetCSVFile()"));
	TArray<FString> Lines;
	FString CsvFile = Path;
	if (FPaths::FileExists(CsvFile))
	{
		//UE_LOG(LogTemp, Log, TEXT("GetCSVFile found: %s"), *CsvFile);
		FFileHelper::LoadANSITextFileToStrings(*CsvFile, NULL, Lines);
	}

	return Lines;
}

FString UTextFileManager::GetCurrentTimeString()
{
	FDateTime TimeNow = FDateTime::Now();
	return FString::Printf(TEXT("%i%i%i_%i_%i_%i.%s"),
		TimeNow.GetYear(), TimeNow.GetMonth(), TimeNow.GetDay(),
		TimeNow.GetHour(), TimeNow.GetMinute(), TimeNow.GetSecond(),
		*FString::Printf(TEXT("%03d"), TimeNow.GetMillisecond()) );
}

FString UTextFileManager::GetStandardHeader(
	FString SubjectId, // current subject ID 
	FString WaypointOrder, // current waypoint order
	FString TrialNumber, // current trial number
	FString StartWaypointNumber,
	FString PhotosphereNum,
	FString TimeNow, // formatted string YYYYMMDD_HHMMSS.MS
	FString StartCodeUnique, // from actor calling this function 
	FString StartCodeGlobal, // from GS
	FString ExperimenterName, // from GI 
	FString VariableLine // e.g. -> time, x, y, z
)
{
	return "Subject_ID:\t" + SubjectId + "\n"
		+ "Waypoint_order:\t" + WaypointOrder + "\n"
		+ "Trial:\t" + TrialNumber + "\n"
		+ "StartWaypoint:\t" + StartWaypointNumber + "\n"
		+ "Photosphere_num:\t" + PhotosphereNum + "\n"
		+ "datetime:\t" + TimeNow + "\n"
		+ "start_code_unique:\t" + StartCodeUnique + "\n"
		+ "start_code_game_state\t" + StartCodeGlobal + "\n"
		+ "Experimenter_name\t" + ExperimenterName + "\n"
		+ "notes:\t\n"
		+ VariableLine + "\n";
}


bool UTextFileManager::ReadFileToArray(FString LoadDirectory,TArray<FString> &outarr) {

	TArray<FString> CSVLines = UTextFileManager::GetCSVFile(LoadDirectory);
	return true; 
}

bool UTextFileManager::SaveTxt(FString SaveText, FString Filename)
{
	/*
	Saves data to ProjectDirectory/MyData/
	Savetxt: line to save
	Filename: F:/ProjectFolder/DataFolder/name.txt to save.
	*/
	bool res;
	FString path = Filename; // absolute path
	IPlatformFile& file = FPlatformFileManager::Get().GetPlatformFile();
	if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*path)) 
	{
		IFileHandle* handle = file.OpenWrite(*path, false, true); // true -> append
		if (!handle) {
			UE_LOG(LogTemp, Error, TEXT("[UTextFileManager::SaveTxt()] Handle == nullptr;"));
			return false;
		}
		res = handle->Write((const uint8*)TCHAR_TO_ANSI(*SaveText), SaveText.Len());
		//delete handle;
	}
	else {
		IFileHandle* handle = file.OpenWrite(*path, false, true); // true -> append
		if (handle == nullptr) { 
			UE_LOG(LogTemp, Error, TEXT("[UTextFileManager::SaveTxt()] Handle == nullptr;"));
			return false; 
		};
		res = handle->Write((const uint8*)TCHAR_TO_ANSI(*SaveText), SaveText.Len());
		delete handle; 
	}
	return res; 
	//return FFileHelper::SaveStringToFile(SaveText, *path);
}

bool UTextFileManager::SaveStringToFile(const FString& Line, const FString& Filepath, bool bAppend)
{
	// Now attempt to save the string to the file.

	if (FFileHelper::SaveStringToFile(Line, *Filepath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM, &IFileManager::Get(), bAppend ? FILEWRITE_Append : 0))
	{
		// If we successfully save the file, return true.
		return true;
	}
	else
	{
		// If the save fails, log an error or handle it as needed.
		UE_LOG(LogTemp, Error, TEXT("Failed to save file %s."), *Filepath);
		return false; // Return false as the save operation failed.
	}
}
