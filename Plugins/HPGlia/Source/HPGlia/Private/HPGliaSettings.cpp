// (c) Copyright 2020-2021 HP Development Company, L.P.

#include "HPGliaSettings.h"

UHPGliaSettings::UHPGliaSettings(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , ConnectionSettings("", "", ELicensingModel::CORE)
{
}