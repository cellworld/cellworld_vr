#include "GameInstanceMain.h"

UGameInstanceMain::UGameInstanceMain()
{
	/* directories (dir_)*/
	dir_project = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
	dir_setup   = dir_project + "Setup/";
	dir_savedata = dir_project + "Data/";

	/* filepaths (file_)*/
	file_hp_client_keys = dir_setup + "HP_client_keys.csv";
}

void UGameInstanceMain::Init()
{
	Super::Init();
}

void UGameInstanceMain::Shutdown()
{
}

void UGameInstanceMain::StartGameInstance()
{
	Super::StartGameInstance();
}