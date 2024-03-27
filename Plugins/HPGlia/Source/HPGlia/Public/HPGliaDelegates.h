// (c) Copyright 2020-2021 HP Development Company, L.P.

#pragma once

#define __STDC_WANT_SECURE_LIB__ 1

#include <omnicept/Glia.h>
#include <omnicept/lib-client-cpp/Client.h>
#include <omnicept/lib-abi-cpp/MessageTypes.h>
#include <omnicept/lib-abi-cpp/CognitiveLoad.h>
#include <omnicept/lib-abi-cpp/SubscriptionResultList.h>
#include "HPGliaCognitiveLoad.h"
#include "HPGliaEyeTracking.h"
#include "HPGliaHeartRateVariability.h"
#include "HPGliaCameraImage.h"
#include "HPGliaDataVaultResults.h"
#include "HPGliaSubscriptionResults.h"
#include "HPGliaIMU.h"
#include "HPGliaPPG.h"
#include "HPGliaTypeConverter.h"
#include "Tickable.h"
#include "Containers/Queue.h"
#include <functional>

#include "HPGliaDelegates.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGliaCogLoadEvent, FCognitiveLoad, cogLoad);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGliaEyeTrackingEvent, FEyeTracking, eyeTracking);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGliaHeartRateEvent, int, heartRate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGliaHeartRateVariabilityEvent, FHeartRateVariability, heartRateVariability);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGliaCameraImageEvent, FCameraImage, cameraImage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FConnectToGliaReturnEvent, bool, wasConnected, FString, connectionResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGliaDataVaultReturnEvent, FDataVaultResults, dataVaultResults);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGliaSubscriptionReturnEvent, FSubscriptionResultsArray, subscriptionResultsArray);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGliaIMUArrayEvent, FIMUArray, imuArray);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGliaPPGArrayEvent, FPPGArray, ppgArray);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGliaDisconnectedEvent);

/*Class is only meant to be used as a member of HPGliaClient and to expose delegates*/
/*On tick, this class will check the queues removing all the values present and calling the corresponding delegate*/
/*Note: Values that are sent faster than the game engine ticks (eyeTracking, for example) should not be used in this class*/

UCLASS(BlueprintType)
class UHPGliaDelegates : public UObject, public FTickableGameObject
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable, meta = (DisplayName = "Cognitive Load Delegate"), Category = Glia)
    FGliaCogLoadEvent OnCogLoadReceived;

    UPROPERTY(BlueprintAssignable, meta = (DisplayName = "Eye Tracking Delegate"), Category = Glia)
    FGliaEyeTrackingEvent OnEyeTrackingReceived;

    UPROPERTY(BlueprintAssignable, meta = (DisplayName = "Heart Rate Delegate"), Category = Glia)
    FGliaHeartRateEvent OnHeartRateReceived;

    UPROPERTY(BlueprintAssignable, meta = (DisplayName = "Heart Rate Variability Delegate"), Category = Glia)
    FGliaHeartRateVariabilityEvent OnHeartRateVariabilityReceived;

    UPROPERTY(BlueprintAssignable, meta = (DisplayName = "Camera Image Delegate"), Category = Glia)
    FGliaCameraImageEvent OnCameraImageReceived;

    UPROPERTY(BlueprintAssignable, meta = (DisplayName = "Data Vault Status Delegate"), Category = Glia)
    FGliaDataVaultReturnEvent OnDataVaultResultReceived;

    UPROPERTY(BlueprintAssignable, meta = (DisplayName = "Subscription Results Delegate"), Category = Glia)
    FGliaSubscriptionReturnEvent OnSubscriptionResultArrayReceived;

    UPROPERTY(BlueprintAssignable, meta = (DisplayName = "IMU Array Delegate"), Category = Glia)
    FGliaIMUArrayEvent OnIMUArrayReceived;

    UPROPERTY(BlueprintAssignable, meta = (DisplayName = "PPG Array Delegate"), Category = Glia)
    FGliaPPGArrayEvent OnPPGArrayReceived;

    UPROPERTY(BlueprintAssignable, meta = (DisplayName = "Unexpected Disconnect Delegate"), Category = Glia)
    FGliaDisconnectedEvent OnDisconnected;

    UPROPERTY(BlueprintAssignable, meta = (DisplayName = "Connect To Glia Return Delegate"), Category = Glia)
    FConnectToGliaReturnEvent OnConnectionReturned;
    
    void Tick(float DeltaTime) override;
    bool IsTickable() const override;
    bool IsTickableInEditor() const override;
    bool IsTickableWhenPaused() const override;
    TStatId GetStatId() const override;

private:
    friend class UHPGliaClient;
    UHPGliaDelegates();
    void setClientState(const HP::Omnicept::Client::State);
    HP::Omnicept::Client::StateCallback_T m_stateCallback;
    void setClientPointer(std::shared_ptr<HP::Omnicept::Client> client);
    
    template<typename Ue4GliaType, typename Delegate>
    void getGliaDataFireDelegate(TQueue<Ue4GliaType>& queue, Delegate& delegate);
    void getSubscriptionResultsDelegate();
    void checkClientState();
    void ConnectionReturned(bool wasConnected, FString connReturn);
    void Disconnected();
    void CheckQueues();
    void ClearQueues();
    void Cleanup();
   
    std::weak_ptr<HP::Omnicept::Client> m_clientptr;
    HP::Omnicept::Client::State m_clientState;
    TSet<HP::Omnicept::Abi::MessageType> m_typesWithActiveCallbacks;

    TQueue<HP::Omnicept::Client::State> m_clientStateQueue;
    TQueue<FCognitiveLoad> m_cogLoadQueue;
    TQueue<FEyeTracking> m_eyeTrackingQueue;
    TQueue<int> m_heartRateQueue;
    TQueue<FHeartRateVariability> m_heartRateVariabilityQueue;
    TQueue<FCameraImage> m_cameraImageQueue;
    TQueue<FDataVaultResults> m_dataVaultRetQueue;
    TQueue<FSubscriptionResultsArray> m_subscriptionRetArrayQueue;
    TQueue<FIMUArray> m_imuArrayQueue;
    TQueue<FPPGArray> m_ppgArrayQueue;

    template<class Ue4Type>
    static std::function<void(std::shared_ptr<typename HPGliaTypeConverter::Ue4ToGliaMap<Ue4Type>::GliaType>)> generateEnqueueCallback(TQueue<Ue4Type>& queue)
    {
        return [&queue](std::shared_ptr<HPGliaTypeConverter::Ue4ToGliaMap<Ue4Type>::GliaType> gliaData) -> void
        {
            Ue4Type ue;
            HPGliaTypeConverter::convertDomainTypeToUe4Type(*gliaData, ue);
            queue.Enqueue(ue);
        };
    }
   
};
