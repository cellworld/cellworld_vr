// (c) Copyright 2019-2021 HP Development Company, L.P.

#pragma once

#include "HPGliaEyeTracking.h"
#include "HPGliaCognitiveLoad.h"
#include "HPGliaLicensingModelEnum.h"
#include "HPGliaHeartRateVariability.h"
#include "HPGliaCameraImage.h"
#include "HPGliaDataVaultResults.h"
#include "HPGliaSubscriptionResults.h"
#include "HPGliaIMU.h"
#include "HPGliaPPG.h"
#include <omnicept/lib-abi-cpp/HeartRate.h>
#include <omnicept/lib-abi-cpp/HeartRateVariability.h>
#include <omnicept/lib-abi-cpp/CameraImage.h>
#include <omnicept/lib-abi-cpp/EyeTracking.h>
#include <omnicept/lib-abi-cpp/CognitiveLoad.h>
#include <omnicept/lib-abi-cpp/LicensingModel.h>
#include <omnicept/lib-abi-cpp/DataVaultResult.h>
#include <omnicept/lib-abi-cpp/SubscriptionResultList.h>
#include <omnicept/lib-abi-cpp/IMU.h>
#include <omnicept/lib-abi-cpp/PPG.h>

class HPGliaTypeConverter
{
public:
    template<class GliaType, class Ue4Type>
    static void convertDomainTypeToUe4Type(const GliaType& from, Ue4Type& to);
    static HP::Omnicept::Abi::LicensingModel LicenseModelConversion(ELicensingModel ue);

    template<class Ue4DomainType> struct Ue4ToGliaMap;

    template <> struct Ue4ToGliaMap<FEyeTracking>{ using GliaType = HP::Omnicept::Abi::EyeTracking;};
    template <> struct Ue4ToGliaMap<FCognitiveLoad>{ using GliaType = HP::Omnicept::Abi::CognitiveLoad;};
    template <> struct Ue4ToGliaMap<FHeartRateVariability>{using GliaType = HP::Omnicept::Abi::HeartRateVariability;};
    template <> struct Ue4ToGliaMap<FCameraImage> { using GliaType = HP::Omnicept::Abi::CameraImage; };
    template <> struct Ue4ToGliaMap<FIMUArray> { using GliaType = HP::Omnicept::Abi::IMUFrame; };
    template <> struct Ue4ToGliaMap<FPPGArray> { using GliaType = HP::Omnicept::Abi::PPGFrame; };
    template <> struct Ue4ToGliaMap<FDataVaultResults> { using GliaType = HP::Omnicept::Abi::DataVaultResult; };
    template <> struct Ue4ToGliaMap<FSubscriptionResultsArray> { using GliaType = HP::Omnicept::Abi::SubscriptionResultList; };
    template <> struct Ue4ToGliaMap<int> { using GliaType = HP::Omnicept::Abi::HeartRate; }; // technically true ...

    template<class Ue4DomainType> static HP::Omnicept::Abi::MessageType getEnumforUe4Type() 
    { 
        return HP::Omnicept::Abi::MessageTypeUtils::getEnumForType<Ue4ToGliaMap<Ue4DomainType>::GliaType>(); 
    };

    template<class Ue4DomainType> static FString getTypeName()
    { 
        return FString( HP::Omnicept::Abi::MessageTypeUtils::getTypeName<Ue4ToGliaMap<Ue4DomainType>::GliaType>().c_str()); 
    }
};

