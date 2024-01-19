#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameInstanceMain.h"
#include "ConfigManager.generated.h"

/**
 *
 */
UCLASS()
class CELLWORLD_VR_API UConfigManager : public UBlueprintFunctionLibrary
{

public:

	GENERATED_BODY()

	//UConfigManager(); 

	static bool LoadHPClientKeys(UGameInstanceMain* GI, TArray<FString>& lines);

	void UpdateGameInstance();
	static FString ReadLineRegexp(FString regexp, FString line);
	static bool GetCSVFile(FString Path, TArray<FString>& Lines);

private:


	/* function args */
	//FString config_path; // store config file location
	//TArray<FString> config_lines; // read config file to lines
	//
	///* regexp patterns  */
	//FString config_order; 
	//FString config_trial; 
	//FString config_startwaypoint; 

	///* load config variables */
	////bool res; 
	//FString match_order; 
	//FString match_trial; 
	//FString match_startwaypoint;
	//FString match_subjectID;
	//FString match_experimenter_name;

	//FString ReadLineRegexp(FString regexp, FString line); 
	/* input targets */

	///* rgeexp targets */
	//FString exp_subjectID			= TEXT("(?<=SubjectID:)\\s*(\\d+)");
	//FString exp_order				= TEXT("(?<=Order:)\\s*([A-Da-d])"); // first letter character after colon
	//FString exp_trial				= TEXT("(?<=Trial:)\\s*(\\d)"); // first single digit integer after colon
	//FString exp_startwaypoint		= TEXT("(?<=StartWaypoint:)\\s*(\\d+)"); // first multi-digit integer after colon 
	//FString exp_experimenter_name	= TEXT("?<=Experimenter Initials:\\s*(\\w{2,3})"); // consecutive letters after the target phrase 
	//TArray<FString> exp_arr;

};