#include "MainMenuWidgetBasic.h"

#include "GameInstanceMain.h"
#include "Components/Button.h"
#include "cellworld_vr/cellworld_vr.h"
#include "Components/CheckBox.h"
#include "Components/ComboBoxString.h"
#include "Kismet/GameplayStatics.h"

void UMainMenuWidgetBasic::NativeConstruct() {
    Super::NativeConstruct();

    // SetIsFocusable(true);
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) {
        UE_LOG(LogExperiment, Error, TEXT("[UMainMenuWidgetBasic::NativeConstruct()] PC not valid!"))
    }else {
        UE_LOG(LogExperiment, Log, TEXT("[UMainMenuWidgetBasic::NativeConstruct()] PC is valid!"))
        // SetUserFocus(PC);
    }
    
    if (StartButton) {
        StartButton->SetIsEnabled(true);
        StartButton->OnClicked.AddDynamic(this, &ThisClass::OnStartButtonClicked);
        // if (PC) { StartButton->SetUserFocus(PC); }
    }

    if (ResetButton) {
        ResetButton->SetIsEnabled(true);
        ResetButton->OnClicked.AddDynamic(this, &ThisClass::OnResetButtonClicked);
        // if (PC) { ResetButton->SetUserFocus(PC); }
    }

    // default run server - FALSE
    if (SpawnExperimentServerCheckBox) {
        SpawnExperimentServerCheckBox->SetIsChecked(false); // default
        SpawnExperimentServerCheckBox->SetIsEnabled(true);

        SpawnExperimentServerCheckBox->OnCheckStateChanged.AddDynamic(this,
            &ThisClass::OnSpawnExperimentServerCheckBoxStateChanged);
        // if (PC) { SpawnExperimentServerCheckBox->SetUserFocus(PC); }
    }

    // Add options for predator speed
    if (PredatorSpeedComboBox) { // temp 
        PredatorSpeedComboBox->AddOption(TEXT("10"));
        PredatorSpeedComboBox->AddOption(TEXT("20"));
        PredatorSpeedComboBox->AddOption(TEXT("50"));
        PredatorSpeedComboBox->SetSelectedOption(TEXT("10")); // default
        PredatorSpeedComboBox->SetIsEnabled(true);
        PredatorSpeedComboBox->OnSelectionChanged.AddDynamic(this, &UMainMenuWidgetBasic::OnPredatorSpeedChanged); // bind
        // if (PC) { PredatorSpeedComboBox->SetUserFocus(PC); }
    }

    // Add options for visual range
    if (VisualRangeComboBox) {
        VisualRangeComboBox->AddOption(TEXT("2"));
        VisualRangeComboBox->AddOption(TEXT("4"));
        VisualRangeComboBox->AddOption(TEXT("8"));
        VisualRangeComboBox->AddOption(TEXT("16"));
        VisualRangeComboBox->AddOption(TEXT("32"));
        VisualRangeComboBox->AddOption(TEXT("64"));
        VisualRangeComboBox->SetSelectedOption(TEXT("8")); // default
        VisualRangeComboBox->SetIsEnabled(true);
        VisualRangeComboBox->OnSelectionChanged.AddDynamic(this, &UMainMenuWidgetBasic::OnVisualRangeChanged); // bind event 
        // if (PC) { VisualRangeComboBox->SetUserFocus(PC); }
    }
}

void UMainMenuWidgetBasic::OnSpawnExperimentServerCheckBoxStateChanged(const bool bIsChecked) {
    UE_LOG(LogTemp, Warning, TEXT("[UMainMenuWidgetBasic::OnSpawnExperimentServerCheckBoxStateChanged] CheckBox clicked: %i"), bIsChecked);

    UE_LOG(LogExperiment, Warning, TEXT("[UMainMenuWidgetBasic::OnSpawnExperimentServerCheckBoxStateChanged] PredatorSpeed attempted change to: %i"), bIsChecked);
    UGameInstanceMain* GameInstance = Cast<UGameInstanceMain>(GetGameInstance());
    if (!GameInstance) {
        UE_LOG(LogExperiment, Error, TEXT("[UMainMenuWidgetBasic::OnSpawnExperimentServerCheckBoxStateChanged] GameInstance NULL!"));
        return;
    }

    if (GameInstance->ExperimentParameters.IsValid()) {
        GameInstance->ExperimentParameters->bSpawnExperimentService = bIsChecked ;
    } else {
        UE_LOG(LogExperiment, Error, TEXT("[UMainMenuWidgetBasic::OnPredatorSpeedChanged] SelectedItem is not numeric!"));
    }
  
    UE_LOG(LogExperiment, Log, TEXT("[UMainMenuWidgetBasic::OnSpawnExperimentServerCheckBoxStateChanged] bSpawnExperimentService: %i"), bIsChecked);
}

void UMainMenuWidgetBasic::OnStartButtonClicked() {
    // Example action: Start the experiment or game logic
    UE_LOG(LogTemp, Warning, TEXT("[UMainMenuWidgetBasic::OnStartButtonClicked] Start button clicked"));
    UGameInstanceMain* GameInstance = Cast<UGameInstanceMain>(GetGameInstance());
    if (!GameInstance) {
        UE_LOG(LogExperiment, Error, TEXT("[UMainMenuWidgetBasic::OnStartButtonClicked] GameInstance NULL!"));
        return;
    }
    GameInstance->OpenLevel("/Game/Levels/L_Maze?Name=Player");
}

void UMainMenuWidgetBasic::OnResetButtonClicked() {
    UE_LOG(LogTemp, Warning, TEXT("[UMainMenuWidgetBasic::OnResetButtonClicked] Reset button clicked"));
    
    const FName CurrentLevelName = FName(*UGameplayStatics::GetCurrentLevelName(this, true));
}

void UMainMenuWidgetBasic::OnPredatorSpeedChanged(const FString SelectedItem, ESelectInfo::Type SelectionType) {
    // Example action: Adjust predator speed based on selected option
    UE_LOG(LogExperiment, Warning, TEXT("[UMainMenuWidgetBasic::OnPredatorSpeedChanged] PredatorSpeed attempted change to: %s"), *SelectedItem);
    UGameInstanceMain* GameInstance = Cast<UGameInstanceMain>(GetGameInstance());
    if (!GameInstance) {
        UE_LOG(LogExperiment, Error, TEXT("[UMainMenuWidgetBasic::OnPredatorSpeedChanged] GameInstance NULL!"));
        return;
    }

    if (SelectedItem.IsNumeric() && GameInstance->ExperimentParameters.IsValid()) {
        GameInstance->ExperimentParameters->SetPredatorSpeedMetric(FCString::Atof(*SelectedItem));
    } else {
        UE_LOG(LogExperiment, Error, TEXT("[UMainMenuWidgetBasic::OnPredatorSpeedChanged] SelectedItem is not numeric!"));
    }
    UE_LOG(LogExperiment, Log, TEXT("[UMainMenuWidgetBasic::OnPredatorSpeedChanged] SUCCESS PredatorSpeed changed to: %s"), *SelectedItem);
}

void UMainMenuWidgetBasic::OnVisualRangeChanged(FString SelectedItem, ESelectInfo::Type SelectionType) {
    // Example action: Adjust visual range based on selected option
    UE_LOG(LogExperiment, Warning, TEXT("[UMainMenuWidgetBasic::OnVisualRangeChanged] VisualRange attempted changed to: %s"), *SelectedItem);
    UGameInstanceMain* GameInstance = Cast<UGameInstanceMain>(GetGameInstance());
    if (!GameInstance) {
        UE_LOG(LogExperiment, Error, TEXT("[UMainMenuWidgetBasic::OnVisualRangeChanged] GameInstance NULL!"));
        return;
    }

    if (SelectedItem.IsNumeric() && GameInstance->ExperimentParameters.IsValid()) {
        GameInstance->ExperimentParameters->SetVisualRangeMetric(FCString::Atof(*SelectedItem));
    } else {
        UE_LOG(LogExperiment, Error, TEXT("[UMainMenuWidgetBasic::OnVisualRangeChanged] SelectedItem is not numeric!"));
    }
    UE_LOG(LogExperiment, Log, TEXT("[UMainMenuWidgetBasic::OnVisualRangeChanged] SUCCESS VisualRange changed to: %s"), *SelectedItem);
}

void UMainMenuWidgetBasic::Init() {
    // Initialization logic if needed
}
