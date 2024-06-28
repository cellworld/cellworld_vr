
#include "HUDExperiment.h"
#include "Components/EditableTextBox.h"

void UHUDExperiment::NativeConstruct()
{
	Super::NativeConstruct();
	this->Init();
}

void UHUDExperiment::SetTimeRemaining(const FString& TimeRemainingIn)
{
	TextBoxTimeRemaining->SetText(FText::FromString(TimeRemainingIn));
}

void UHUDExperiment::Init()
{
	this->SetTimeRemaining(FString("N/A"));
	this->SetCurrentStatus(FString("Waiting"));
}

void UHUDExperiment::SetCurrentStatus(const FString& CurrentStatusIn)
{
	TextBoxCurrentStatus->SetText(FText::FromString(CurrentStatusIn));
}
