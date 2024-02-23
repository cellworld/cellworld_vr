// (c) Copyright 2020-2021 HP Development Company, L.P.

#pragma once

#include "CoreMinimal.h"
#include "HPGliaLicensingModelEnum.h"
#include "HPGliaSettings.generated.h"

USTRUCT(BlueprintType)
struct FHPGliaConnectionSettings
{
    GENERATED_USTRUCT_BODY()

public:
    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Connection Settings")
        FString ClientID;

    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Connection Settings")
        FString AccessKey;

    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Connection Settings")
        ELicensingModel LicenseMode;

    FHPGliaConnectionSettings(FString clientID, FString accessKey, ELicensingModel licenseMode) : 
        ClientID(clientID), AccessKey(accessKey), LicenseMode(licenseMode) {}
    FHPGliaConnectionSettings() : ClientID(""), AccessKey(""), LicenseMode(ELicensingModel::TRIAL) {}

};

UCLASS(config = Engine, defaultconfig)
class UHPGliaSettings : public UObject
{
    GENERATED_UCLASS_BODY()

        UPROPERTY(config, EditAnywhere, Category = Settings)
        FHPGliaConnectionSettings ConnectionSettings;
};