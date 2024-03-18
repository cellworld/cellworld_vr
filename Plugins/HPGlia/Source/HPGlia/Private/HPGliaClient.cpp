// (c) Copyright 2020-2021 HP Development Company, L.P.

#include "HPGliaClient.h"
#include "HPGliaTypeConverter.h"
#include <vector>

UHPGliaDelegates * UHPGliaClient::m_delegates = nullptr;
UHPGliaClientBuilder* UHPGliaClient::m_clientBuilder = nullptr;
std::shared_ptr<HP::Omnicept::Client> UHPGliaClient::m_gliaClient = nullptr;
UHPGliaClientBuilder::BuiltCallback_T UHPGliaClient::m_buildResultCallback;
HP::Omnicept::Client::StateCallback_T UHPGliaClient::m_onStateChange = nullptr;
std::atomic<HP::Omnicept::Client::StateCallback_T*> UHPGliaClient::m_delegateOnStateChange = nullptr;
HP::Omnicept::Client::State UHPGliaClient::m_clientState = HP::Omnicept::Client::State::DISCONNECTED;
HP::Omnicept::Abi::SubscriptionList UHPGliaClient::m_subList = *HP::Omnicept::Abi::SubscriptionList::GetSubscriptionListToAll();

UHPGliaClient::UHPGliaClient(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    m_onStateChange = [&clientState = m_clientState, &delegateStateCallback = m_delegateOnStateChange](HP::Omnicept::Client::State state)
    {
        clientState = state;
        if (delegateStateCallback != nullptr)
        {
            (*delegateStateCallback.load())(state);
        }
    };

    m_buildResultCallback = [&clientState = m_clientState, &gliaClient = m_gliaClient, &delegates = m_delegates, &builder = m_clientBuilder](bool result, FString msg, std::unique_ptr<HP::Omnicept::Client> client)
    {
        gliaClient = std::move(client);
        clientState = gliaClient != nullptr ? gliaClient->getClientState() : HP::Omnicept::Client::State::DISCONNECTED;
        builder->RemoveFromRoot();
        builder = nullptr;
        bool delegatesExist = m_delegates != nullptr;
        if (delegatesExist)
        {
            m_delegates->ConnectionReturned(result, msg);
        }
        if (gliaClient != nullptr)
        {
            if (delegatesExist)
            {
                m_delegates->setClientPointer(gliaClient);
                m_delegates->getSubscriptionResultsDelegate();
            }
            gliaClient->setSubscriptions(m_subList);
            gliaClient->startClient();
        }
    };
}

void UHPGliaClient::ConnectToGliaAsync(const FString clientID, const FString accessKey, const ELicensingModel requestedLicense)
{
    if (!IsConnected() && m_clientBuilder == nullptr)
    {
        if (m_delegates != nullptr)
        {
            m_delegates->Disconnected();
        }
        if (m_gliaClient != nullptr)
        {
            m_gliaClient.reset();
        }
        m_clientBuilder = NewObject<UHPGliaClientBuilder>();
        m_clientBuilder->buildClient(clientID, accessKey, requestedLicense, m_onStateChange, m_buildResultCallback);
        m_clientBuilder->AddToRoot();
    }
}

void UHPGliaClient::DisconnectFromGlia()
{
    if (m_clientBuilder != nullptr)
    {
        m_clientBuilder->stopBuildClient();
        m_clientBuilder->RemoveFromRoot();
        m_clientBuilder = nullptr;
    }
    if (m_gliaClient != nullptr)
    {
        m_gliaClient->disconnectClient();
    }
}

void UHPGliaClient::SetSubscriptions(const TArray<FSubscription> subList)
{
    std::shared_ptr<HP::Omnicept::Abi::SubscriptionList> gliaSubList = std::make_shared<HP::Omnicept::Abi::SubscriptionList>();
    for (auto& sub : subList)
    {
        if (sub.MessageType == ESubscribeableMsgType::None)
        {
            continue;
        }
        HP::Omnicept::Abi::MessageType gliaType = getSubscriptionMessageType(sub.MessageType);
        HP::Omnicept::Abi::SensorInfo gliaSensorInfo{ TCHAR_TO_UTF8(*sub.SensorInfo.SensorId), TCHAR_TO_UTF8(*sub.SensorInfo.SensorSubId), TCHAR_TO_UTF8(*sub.SensorInfo.Location) };
        try
        {
            gliaSubList->getSubscriptions().emplace_back(
                HP::Omnicept::Abi::Subscription{ gliaType, gliaSensorInfo, TCHAR_TO_UTF8(*sub.Sender), HP::Omnicept::Abi::MessageVersionSemantic{TCHAR_TO_UTF8(*sub.Version)} }
            );
        }
        catch (const std::invalid_argument& e)
        {
            FString exceptionStr(e.what());
            const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("ESubscribeableMsgType"), true);
            UE_LOG(LogTemp, Error, TEXT("Subscription message %s, version string %s : %s"), *EnumPtr->GetNameStringByIndex(static_cast<int>(sub.MessageType)), *sub.Version, *exceptionStr);
        }
    }
    m_subList = *gliaSubList;
    if (m_gliaClient != nullptr)
    {
        m_gliaClient->setSubscriptions(m_subList);
    }
}

bool UHPGliaClient::GetHeartRate(int & OutHeartRate)
{
    return getDataIfValid(OutHeartRate);
}

bool UHPGliaClient::GetHeartRateVariability(FHeartRateVariability & OutHeartRateVariability)
{
    return getDataIfValid(OutHeartRateVariability);
}

bool UHPGliaClient::GetCameraImage(FCameraImage & OutCameraImage)
{
    return getDataIfValid(OutCameraImage);
}

bool UHPGliaClient::GetEyeTracking(FEyeTracking & OutEyeTracking)
{
    return getDataIfValid(OutEyeTracking);
}

bool UHPGliaClient::GetCognitiveLoad(FCognitiveLoad & OutCognitiveLoadStruct)
{
    return getDataIfValid(OutCognitiveLoadStruct);
}

bool UHPGliaClient::GetIMUArray(FIMUArray& OutIMUArray)
{
    return getDataIfValid(OutIMUArray);
}

bool UHPGliaClient::GetPPGArray(FPPGArray& OutPPGArray)
{
    return getDataIfValid(OutPPGArray);
}

bool UHPGliaClient::SendDataRecordRequest()
{
    bool worked = false;

    if (IsConnected())
    {
        if (m_delegates != nullptr && m_delegates->OnDataVaultResultReceived.IsBound())
        {
            if (m_gliaClient->startDataRecord(UHPGliaDelegates::generateEnqueueCallback(m_delegates->m_dataVaultRetQueue)) == HP::Omnicept::Client::Result::SUCCESS)
            {
                worked = true;
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Error Starting data record, failed to send requests to Omnicept runtime."));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Start data recording without adding function delegates to FRecordDataReturnEvent."));
            worked = false;
        }
    }
    return worked;
}

bool UHPGliaClient::StopDataRecord()
{
    bool worked = false;

    if (IsConnected())
    {
        if (m_gliaClient->stopDataRecord() == HP::Omnicept::Client::Result::SUCCESS)
        {
            worked = true;
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Error Stopping data record, failed to send requests to Omnicept runtime."));
        }
    }
    return worked;
}
bool UHPGliaClient::IsConnected()
{
   return m_clientState != HP::Omnicept::Client::State::DISCONNECTED && m_gliaClient != nullptr;
}

UHPGliaDelegates * UHPGliaClient::GetGliaDelegates()
{
    /*To prevent having an unnecessary ticking object, the HPGliaDelegate Object is only created when requested*/
    if (m_delegates == nullptr)
    {
        CreateDelegate();
    }
    return m_delegates;
}
void UHPGliaClient::CreateDelegate()
{
    if (m_delegates == nullptr)
    {
        m_delegates = NewObject<UHPGliaDelegates>();
        m_delegates->AddToRoot(); /*I know this is to avoid UE's garbage collection, but am not sure if a new level is loaded whether it will get GC'd*/
        m_delegateOnStateChange = &m_delegates->m_stateCallback;
        m_delegates->setClientPointer(m_gliaClient);
    }
}
