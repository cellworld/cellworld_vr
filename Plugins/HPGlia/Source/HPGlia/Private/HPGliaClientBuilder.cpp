// (c) Copyright 2020-2021 HP Development Company, L.P.

#include "HPGliaClientBuilder.h"
#include <omnicept/lib-abi-cpp/IMessage.h>
#include "HPGliaTypeConverter.h"
#include <omnicept/lib-abi-cpp/SessionLicense.h>


UHPGliaClientBuilder::UHPGliaClientBuilder()
{
    m_tryingToConnect = false;
}

void UHPGliaClientBuilder::Tick(float DeltaTime)
{
    getClientIfReady();
}

bool UHPGliaClientBuilder::IsTickable() const
{
    return m_tryingToConnect;
}

bool UHPGliaClientBuilder::IsTickableInEditor() const
{
    return false;
}

bool UHPGliaClientBuilder::IsTickableWhenPaused() const
{
    return m_tryingToConnect;
}

TStatId UHPGliaClientBuilder::GetStatId() const
{
    return TStatId();
}

void UHPGliaClientBuilder::stopBuildClient()
{
    m_shouldBuild = false;
}

void UHPGliaClientBuilder::buildClient(FString clientID, FString accessKey, ELicensingModel requestedLicense, HP::Omnicept::Client::StateCallback_T onStateChange, UHPGliaClientBuilder::BuiltCallback_T builtCallback)
{
    bool startedAsyncClientBuild = false;
    HP::Omnicept::Abi::LicensingModel licenseModel = HPGliaTypeConverter::LicenseModelConversion(requestedLicense);
    bool isRunningInEditor = false;
    #if WITH_EDITOR
        isRunningInEditor = true;
    #endif
        
    try
    {
        auto license = std::make_unique<HP::Omnicept::Abi::SessionLicense>(std::string{ TCHAR_TO_UTF8(*clientID) }, std::string{ TCHAR_TO_UTF8(*accessKey) }, licenseModel, isRunningInEditor);
        m_asyncBuilder = HP::Omnicept::Glia::StartBuildClient_Async("Ue4OmniceptClient", std::move(license), onStateChange);
        startedAsyncClientBuild = true;
    }
    catch (const std::invalid_argument& error)
    {
        FString err = FString(error.what());
        UE_LOG(LogTemp, Warning, TEXT("Failed to start async client builder. std::invalid_argument error %s"), *err);
        builtCallback(false, err, nullptr);
    }
    catch (const std::exception& error)
    {
        FString err = FString(error.what());
        UE_LOG(LogTemp, Warning, TEXT("Failed to start async client builder. std::exception error %s"), *err);
        builtCallback(false, err, nullptr);
    }
    catch (...)
    {
        FString err = "Unknown Exception";
        UE_LOG(LogTemp, Warning, TEXT("Failed to start async client builder. %s"), *err);
        builtCallback(false, err, nullptr);
    }
    if (startedAsyncClientBuild)
    {
        m_finishClient =
            [builtCallback](std::unique_ptr<HP::Omnicept::Glia::AsyncClientBuilder> asyncBuilder)
        {
            bool wasClientBuilt = false;
            FString buildResult;
            std::unique_ptr<HP::Omnicept::Client> clientPtr;
            try
            {
                clientPtr = asyncBuilder->getBuildClientResultOrThrow();
                wasClientBuilt = true;
                buildResult = "SUCCESS";
            }
            catch (const HP::Omnicept::Abi::HandshakeError& error)
            {
                buildResult = FString(error.what());
                UE_LOG(LogTemp, Warning, TEXT("HandshakeError %s"), *buildResult);
            }
            catch (const HP::Omnicept::Abi::ProtocolError& error)
            {
                buildResult = FString(error.what());
                UE_LOG(LogTemp, Warning, TEXT("Protocol Error %s"), *buildResult);
            }
            catch (const HP::Omnicept::Abi::TransportError& error)
            {
                buildResult = FString(error.what());
                UE_LOG(LogTemp, Warning, TEXT("Glia Transport Error %s"), *buildResult);
            }
            catch (const std::logic_error& error)
            {
                buildResult = FString(error.what());
                UE_LOG(LogTemp, Warning, TEXT("Std logic error %s"), *buildResult);
            }
            catch (const std::exception& error)
            {
                buildResult = FString(error.what());
                UE_LOG(LogTemp, Warning, TEXT("Std exception %s"), *buildResult);
            }
            catch (...)
            {
                UE_LOG(LogTemp, Warning, TEXT("Glia Connection Error"));
                buildResult = "Glia Connection Error";
            }
            builtCallback(wasClientBuilt, buildResult, std::move(clientPtr));
        };

        m_tryingToConnect = true;
        m_shouldBuild = true;
    }
}

void UHPGliaClientBuilder::getClientIfReady()
{
    if (m_tryingToConnect && m_asyncBuilder != nullptr && m_asyncBuilder->isClientBuildFinished())
    {
        if (m_shouldBuild)
        {
            m_finishClient(std::move(m_asyncBuilder));
        }
        m_asyncBuilder.reset();
        m_finishClient = nullptr;
        m_tryingToConnect = false;
    }
}
