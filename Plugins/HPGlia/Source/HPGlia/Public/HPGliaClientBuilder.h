// (c) Copyright 2020-2021 HP Development Company, L.P.

#pragma once
#include <omnicept/lib-client-cpp/Client.h>
#include <omnicept/Glia.h>
#include "Tickable.h"
#include "HPGliaLicensingModelEnum.h"
#include <functional>


#include "HPGliaClientBuilder.generated.h"

/* Class is meant to be used in HPGliaClient to build the HP::Omnicept::Client class asynchronously. 
 * When HPGliaClient::Connect is called, it will create a UHPGliaClientBuilder and call UHPGliaClientBuilder::buildClient
 * UHPGliaClientBuilder will call the asynchronous client builder and use its Tick function to check if the client is done building.
 * When the client is done the build restult will be passed to the builtCallback, then this object  will mark itself for destroy and stop ticking.
 */

UCLASS()
class UHPGliaClientBuilder : public UObject, public FTickableGameObject
{
    GENERATED_BODY()

public:
   
    void Tick(float DeltaTime) override;
    bool IsTickable() const override;
    bool IsTickableInEditor() const override;
    bool IsTickableWhenPaused() const override;
    TStatId GetStatId() const override;
private:
    friend class UHPGliaClient;
    UHPGliaClientBuilder();

    using BuiltCallback_T = std::function<void(bool result, FString msg, std::unique_ptr<HP::Omnicept::Client>)>;
    std::unique_ptr<HP::Omnicept::Glia::AsyncClientBuilder> m_asyncBuilder;
    
    bool m_tryingToConnect;
    bool m_shouldBuild;
    std::function<void(std::unique_ptr<HP::Omnicept::Glia::AsyncClientBuilder>)> m_finishClient;
    // if disconnect is called after the build has started but before it has returned, this function will set the buildcallback to not return to glia client
    void stopBuildClient();
    void buildClient(FString clientID, FString accessKey, ELicensingModel requestedLicense, HP::Omnicept::Client::StateCallback_T onStateChange,
                     BuiltCallback_T builtCallback);
    void getClientIfReady();
};
