#include "ConfigManager.h"
#include "TextFileManager.h"
#include "Engine/World.h"
#include "Misc/Paths.h"
//#include "GameInstanceMain.h"
#include "Misc/FileHelper.h"
#include "Kismet/GameplayStatics.h" 
#include "Internationalization/Regex.h" 


FString UConfigManager::ReadLineRegexp(FString regexp, FString line)
{
	/* use regular expression to search for target within a line */
	/* to do: move this function to UTextFileManager*/

	const FRegexPattern myPattern(*regexp);
	FRegexMatcher myMatch(myPattern, *line);

	if (myMatch.FindNext()) {
		FString Match = myMatch.GetCaptureGroup(0);
		FString Name = myMatch.GetCaptureGroup(1);

		UE_LOG(LogTemp, Log, TEXT("Matched string: %s"), *Match);
		UE_LOG(LogTemp, Log, TEXT("Name: %s"), *Name);

		return Name;
	}
	else {
		return "";
		UE_LOG(LogTemp, Display, TEXT("No match found."));
	}
}

bool UConfigManager::GetCSVFile(FString Path, TArray<FString> &Lines)
{
	UE_LOG(LogTemp, Warning, TEXT("[UConfigManager::GetCSVFile()] Searching for file: %s."),*Path);
	if (FPaths::FileExists(Path))
	{
		FFileHelper::LoadANSITextFileToStrings(*Path, NULL, Lines);
		//UE_LOG(LogTemp, Warning, TEXT("[UConfigManager::GetCSVFile()] Content: %s."), *Lines[1]);
		return true; 
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("[UConfigManager::GetCSVFile()] File not found."));
		return false; 
	}
}


/* 
* Updates game instance with HP client and access keys.
*/
bool UConfigManager::LoadHPClientKeys(FString filename, TArray<FString>& lines) {

	/* 
		to do: attempt to clean up objects. will load keys in struct and will add instructions to readme.
	*/

	///* set result to false */
	//bool res; 
	//TArray<FString> config_lines; // read config file to lines

	///* vars */
	//FString match_client_id = "";
	//FString match_access_key = "";

	///* rgeexp targets */
	//FString exp_client_id = TEXT("client id:\\s*([^\\s,.]+)");
	//FString exp_access_key = TEXT("access key:\\s*([^\\s,.]+)");
	//
	///* store in array */
	//TArray<FString> exp_arr;
	//exp_arr.Add(exp_client_id);
	//exp_arr.Add(exp_access_key);

	///* get file */
	//if (filename.Len() < 5) { // at least x.txt/csv etc (to do: check if valid path)
	//	filename = FPaths::ProjectDir();
	//	filename.Append(TEXT("Setup/HP_client_keys.txt"));
	//	UE_DEBUG_BREAK();
	//}



	///* load file */
	//res = UConfigManager::GetCSVFile(filename, config_lines);

	///* check if file valid and not empty */
	//if (!res || (config_lines.Num() == 0)) { // failed to read || empty file 
	//	UE_LOG(LogTemp, Error, TEXT("[UConfigManager::LoadConfigFile()] GetCSVFile() failed."));
	//	return false;
	//}

	///* parse through file */
	//for (int j = 0; j < config_lines.Num(); j++) { // get first elemtn in config_lines

	//	FString currentConfigLine = config_lines[j];
	//	for (FString expression : exp_arr) {

	//		// get matches with regexp 
	//		FString match = ReadLineRegexp(*expression, *currentConfigLine); 

	//		// if we found a match
	//		if (match.Len() < 1) {
	//			UE_LOG(LogTemp, Error, TEXT("[UConfigManager::LoadConfigFile] Found 0 matches."))
	//			return false;
	//		}

	//		if (expression == exp_client_id) { 
	//			match_client_id = match;
	//			exp_arr.RemoveSingle(expression);
	//			break;
	//		}

	//		if (expression == exp_access_key) {
	//			match_access_key = match;
	//			exp_arr.RemoveSingle(expression);
	//			break;
	//		}
	//	}
	//}

	//if (GI == nullptr) {
	//	UE_LOG(LogTemp, Error, TEXT("[UConfigManager::LoadConfigFile()] UGameInstanceMain is NULLPTR."));
	//	UE_DEBUG_BREAK();
	//	return false; 
	//}

	///* save to game instance */
	//GI->client_id  = match_client_id;
	//GI->access_key = match_access_key; 

	return false; 
}


void UConfigManager::UpdateGameInstance()
{

}