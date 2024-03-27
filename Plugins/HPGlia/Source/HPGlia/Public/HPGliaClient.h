// (c) Copyright 2020-2021 HP Development Company, L.P.

#pragma once

#include "CoreMinimal.h"

#include <omnicept/Glia.h>
#include <omnicept/lib-client-cpp/Client.h>
#include <omnicept/lib-abi-cpp/IMessage.h>
#include <omnicept/lib-abi-cpp/MessageTypes.h>
#include <omnicept/lib-abi-cpp/CognitiveLoad.h>
#include "HPGliaEyeTracking.h"
#include "HPGliaDelegates.h"
#include "HPGliaCognitiveLoad.h"
#include "HPGliaLicensingModelEnum.h"
#include "HPGliaClientBuilder.h"
#include "HPGliaSubscription.h"
#include <functional>
#include <memory>
#include <map>
#include <atomic>
#include "HPGliaClient.generated.h"

UCLASS()
class HPGLIA_API UHPGliaClient : public UObject
{
    GENERATED_BODY()
public:

    UHPGliaClient(const FObjectInitializer& ObjectInitializer);

    static void ConnectToGliaAsync(FString clientID, FString accessKey, ELicensingModel requestedLicense);

    static void DisconnectFromGlia();

    static void SetSubscriptions(TArray<FSubscription> subList);

    static bool GetHeartRate(int& OutHeartRate);

    static bool GetHeartRateVariability(FHeartRateVariability& OutHeartRateVariability);

    static bool GetCameraImage(FCameraImage& OutCameraImage);

    static bool GetEyeTracking(FEyeTracking& OutEyeTracking);

    static bool GetCognitiveLoad(FCognitiveLoad& OutCognitiveLoadStruct);

    static bool GetIMUArray(FIMUArray& OutIMUArray);
    
    static bool GetPPGArray(FPPGArray& OutPPGArray);

    static bool SendDataRecordRequest();

    static bool StopDataRecord();

    static bool IsConnected();

    static  UHPGliaDelegates * GetGliaDelegates();

private:

    template<class Ue4Type>
    static bool getDataIfValid(Ue4Type& out)
    {
        bool gotData = false;
        if (IsConnected())
        {
            auto lvc = m_gliaClient->getLastData<HPGliaTypeConverter::Ue4ToGliaMap<Ue4Type>::GliaType>();
            if (lvc.valid)
            {
                HPGliaTypeConverter::convertDomainTypeToUe4Type(lvc.data, out);
                gotData = true;
            }
        }
        return gotData;
    }

    static HP::Omnicept::Abi::MessageType getSubscriptionMessageType(ESubscribeableMsgType ue4EnumType)
    {
        HP::Omnicept::Abi::MessageType GliaType = HP::Omnicept::Abi::MessageType::null;
        switch (ue4EnumType)
        {
        case ESubscribeableMsgType::EyeTracking:
            GliaType = HP::Omnicept::Abi::MessageType::eyeTracking;
            break;
        case ESubscribeableMsgType::CameraImage:
            GliaType = HP::Omnicept::Abi::MessageType::cameraImage;
            break;
        case ESubscribeableMsgType::CognitiveLoad:
            GliaType = HP::Omnicept::Abi::MessageType::cognitiveLoad;
            break;
        case ESubscribeableMsgType::HeartRateVariability:
            GliaType = HP::Omnicept::Abi::MessageType::heartRateVariability;
            break;
        case ESubscribeableMsgType::IMUArray:
            GliaType = HP::Omnicept::Abi::MessageType::imuFrame;
            break;
        case ESubscribeableMsgType::PPGArray:
            GliaType = HP::Omnicept::Abi::MessageType::ppgFrame;
            break;
        case ESubscribeableMsgType::HeartRate:
            GliaType = HP::Omnicept::Abi::MessageType::heartRate;
            break;
        case ESubscribeableMsgType::DataVaultResult:
            GliaType = HP::Omnicept::Abi::MessageType::dataVaultResult;
            break;
        }
        return GliaType;
    }

    static UHPGliaDelegates* m_delegates;

    static HP::Omnicept::Client::State m_clientState;

    static std::shared_ptr<HP::Omnicept::Client> m_gliaClient;

    static UHPGliaClientBuilder* m_clientBuilder;

    static HP::Omnicept::Abi::SubscriptionList m_subList;

    static void CreateDelegate();

    //Callback used by gliaClientBuilder when client is done building
    static UHPGliaClientBuilder::BuiltCallback_T m_buildResultCallback;
    static HP::Omnicept::Client::StateCallback_T m_onStateChange;
    static std::atomic<HP::Omnicept::Client::StateCallback_T*> m_delegateOnStateChange;

};