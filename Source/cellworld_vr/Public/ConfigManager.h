#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
//#include "GameInstanceMain.h"
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

	static bool LoadHPClientKeys(FString, TArray<FString>& lines);

	void UpdateGameInstance();
	static FString ReadLineRegexp(FString regexp, FString line);
	static bool GetCSVFile(FString Path, TArray<FString>& Lines);

private:


};