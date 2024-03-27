// (c) Copyright 2020-2021 HP Development Company, L.P.

#include "HPGliaDelegates.h"
#include <omnicept/lib-client-cpp/Client.h>
#include "HPGliaTypeConverter.h"

// Delegate Functions
UHPGliaDelegates::UHPGliaDelegates()
{
    m_clientState = HP::Omnicept::Client::State::DISCONNECTED;
    m_stateCallback = [this](const HP::Omnicept::Client::State state) { setClientState(state); };
}

void UHPGliaDelegates::setClientState(const HP::Omnicept::Client::State state)
{
    m_clientStateQueue.Enqueue(state);
}

void UHPGliaDelegates::setClientPointer(std::shared_ptr<HP::Omnicept::Client> client)
{
    Disconnected();
    m_clientptr = client;
    m_clientState = client != nullptr ? client->getClientState() : HP::Omnicept::Client::State::DISCONNECTED;
}

bool UHPGliaDelegates::IsTickable() const
{
    return m_clientState != HP::Omnicept::Client::State::DISCONNECTED;
}

bool UHPGliaDelegates::IsTickableInEditor() const
{
    return IsTickable();
}

/*Tickable when paused to prevent queue from filling up. Delegates will still fire. Can set to false and make queues circular if necessary */
bool UHPGliaDelegates::IsTickableWhenPaused() const
{
    return IsTickable();
}

TStatId UHPGliaDelegates::GetStatId() const
{
    return TStatId();
}

void UHPGliaDelegates::Tick(float DeltaTime)
{
    checkClientState();
    if (m_clientState == HP::Omnicept::Client::State::RUNNING)
    {
        CheckQueues();
    }
}

void UHPGliaDelegates::CheckQueues()
{
    getGliaDataFireDelegate(m_cogLoadQueue, OnCogLoadReceived);
    getGliaDataFireDelegate(m_eyeTrackingQueue, OnEyeTrackingReceived);
    getGliaDataFireDelegate(m_heartRateQueue, OnHeartRateReceived);
    getGliaDataFireDelegate(m_heartRateVariabilityQueue, OnHeartRateVariabilityReceived);
    getGliaDataFireDelegate(m_cameraImageQueue, OnCameraImageReceived);
    getGliaDataFireDelegate(m_dataVaultRetQueue, OnDataVaultResultReceived);
    getGliaDataFireDelegate(m_subscriptionRetArrayQueue, OnSubscriptionResultArrayReceived);
    getGliaDataFireDelegate(m_imuArrayQueue, OnIMUArrayReceived);
    getGliaDataFireDelegate(m_ppgArrayQueue, OnPPGArrayReceived);
}

/*Not sure if getting enqueueing the module control signal is necessary since it is only done on the stop callback*/
void UHPGliaDelegates::checkClientState()
{
    if (!m_clientStateQueue.IsEmpty())
    {
        if (m_clientStateQueue.Dequeue(m_clientState))
        {
            if (m_clientState == HP::Omnicept::Client::State::DISCONNECTED)
            {
                UE_LOG(LogTemp, Error, TEXT("Connection to Glia has been lost"));
                Disconnected();
                if (OnDisconnected.IsBound())
                {
                    OnDisconnected.Broadcast();
                }
            }
        }
    }
}

void UHPGliaDelegates::ConnectionReturned(bool wasConnected, FString connReturn)
{
    if (OnConnectionReturned.IsBound())
    {
        OnConnectionReturned.Broadcast(wasConnected, connReturn);
    }
    Cleanup();
}

void UHPGliaDelegates::Disconnected()
{
    m_clientState = HP::Omnicept::Client::State::DISCONNECTED;
    m_clientptr.reset();
    Cleanup();
}

template<typename Ue4GliaType, typename Delegate>
void UHPGliaDelegates::getGliaDataFireDelegate(TQueue<Ue4GliaType>& queue, Delegate & delegate)
{
    HP::Omnicept::Abi::MessageType type = HPGliaTypeConverter::getEnumforUe4Type<Ue4GliaType>();
    bool hasCallback = m_typesWithActiveCallbacks.Contains(type);
    if (hasCallback && delegate.IsBound())
    {
        while (!queue.IsEmpty())
        {
            Ue4GliaType bt;
            if (queue.Dequeue(bt))
            {
                if (delegate.IsBound())
                {
                    delegate.Broadcast(bt);
                }
            }
        }
    }

    else if (hasCallback != delegate.IsBound())
    {
        //All events bound to delegate have become unbound since last tick
        if (hasCallback)
        {
            auto client = m_clientptr.lock();
            if (client != nullptr)
            {
                client->unregisterCallback<HPGliaTypeConverter::Ue4ToGliaMap<Ue4GliaType>::GliaType>();
            }
            m_typesWithActiveCallbacks.Remove(type);
            queue.Empty();
        }
        //Event has been bound to previously unbound delegate since last tick
        else
        {
            auto client = m_clientptr.lock();
            if (client != nullptr)
            {
                if (client->registerCallback(generateEnqueueCallback(queue)) == HP::Omnicept::Client::Result::SUCCESS)
                {
                    m_typesWithActiveCallbacks.Emplace(type);
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("Failed to Register callback for %s"), *HPGliaTypeConverter::getTypeName<Ue4GliaType>());
                }
            }
        }
    }
}

void UHPGliaDelegates::getSubscriptionResultsDelegate()
{
    getGliaDataFireDelegate(m_subscriptionRetArrayQueue, OnSubscriptionResultArrayReceived);
}

void UHPGliaDelegates::ClearQueues()
{
    m_clientStateQueue.Empty();
    m_cogLoadQueue.Empty();
    m_eyeTrackingQueue.Empty();
    m_heartRateQueue.Empty();
    m_heartRateVariabilityQueue.Empty();
    m_cameraImageQueue.Empty();
    m_dataVaultRetQueue.Empty();
    m_subscriptionRetArrayQueue.Empty();
    m_imuArrayQueue.Empty();
    m_ppgArrayQueue.Empty();
}

void UHPGliaDelegates::Cleanup()
{
    m_typesWithActiveCallbacks.Empty();
    ClearQueues();
}
