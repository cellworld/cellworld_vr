// (c) Copyright 2019-2021 HP Development Company, L.P.

#include "HPGliaBPLibrary.h"

void UHPGliaBPLibrary::ConnectToGliaAsync(FHPGliaConnectionSettings connectionSettings)
{
    if (connectionSettings.ClientID != "" || connectionSettings.AccessKey != "")
    {
        UHPGliaClient::ConnectToGliaAsync(connectionSettings.ClientID, connectionSettings.AccessKey, connectionSettings.LicenseMode);
    }
    else
    {
        UHPGliaClient::ConnectToGliaAsync(GetDefault<UHPGliaSettings>()->ConnectionSettings.ClientID, GetDefault<UHPGliaSettings>()->ConnectionSettings.AccessKey, GetDefault<UHPGliaSettings>()->ConnectionSettings.LicenseMode);
    }
}

void UHPGliaBPLibrary::DisconnectFromGlia()
{
    UHPGliaClient::DisconnectFromGlia();
}

void UHPGliaBPLibrary::SetSubscriptions(TArray<FSubscription> subscriptionList)
{
    UHPGliaClient::SetSubscriptions(subscriptionList);
}

void UHPGliaBPLibrary::GetHeartRate(int& OutHeartRate, EExecution& ExecutionBranch)
{
    OutHeartRate = 0;
    bool worked = UHPGliaClient::GetHeartRate(OutHeartRate);
    
    ExecutionBranch = worked ? EExecution::Then : EExecution::Error;
}

void UHPGliaBPLibrary::GetHeartRateVariability(FHeartRateVariability& OutHeartRateVariability, EExecution& ExecutionBranch)
{
    bool worked = UHPGliaClient::GetHeartRateVariability(OutHeartRateVariability);
    
    ExecutionBranch = worked ? EExecution::Then : EExecution::Error;
}

void UHPGliaBPLibrary::GetCameraImage(FCameraImage& OutCameraImage, EExecution& ExecutionBranch)
{
    bool worked = UHPGliaClient::GetCameraImage(OutCameraImage);

    ExecutionBranch = worked ? EExecution::Then : EExecution::Error;
}

void UHPGliaBPLibrary::GliaMakeTexture2D(const EPixelFormat pixelFormat, const int width, const int height, UTexture2D*& newTexture)
{
    newTexture = UTexture2D::CreateTransient(width, height, pixelFormat);
}

void UHPGliaBPLibrary::GliaLoadImageBytesIntoTexture(const TArray<uint8>& buffer, UTexture2D* InTexture, UTexture2D*& OutTexture)
{
    bool isValidBuffer = false;
    bool isValidTexture = false;
    
    if (buffer.GetData())
    {
        isValidBuffer = true;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid input buffer while loading Glia buffer to texture."));
    }

    if (InTexture)
    {
        isValidTexture = true;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid input texture while loading Glia buffer to texture."));
    }

    if (isValidBuffer && isValidTexture)
    {
        if (buffer.Num() == InTexture->GetSizeX() * InTexture->GetSizeY())
        {
            void* TextureData = InTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
            FMemory::Memcpy(TextureData, buffer.GetData(), InTexture->GetSizeX() * InTexture->GetSizeY());
            InTexture->PlatformData->Mips[0].BulkData.Unlock();
            OutTexture = InTexture;
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Input Buffer size does not match input texture dimensions while loading Glia buffer to texture."));
        }
    }
}

void UHPGliaBPLibrary::GliaUpdateTextureResource(UTexture2D* InTexture, UTexture2D*& OutTexture)
{
    if (InTexture)
    {
        InTexture->UpdateResource();
        OutTexture = InTexture;
    }
}

void UHPGliaBPLibrary::SendDataRecordRequest(EExecution& ExecutionBranch)
{
    bool worked = UHPGliaClient::SendDataRecordRequest();

    ExecutionBranch = worked ? EExecution::Then : EExecution::Error;
}

void UHPGliaBPLibrary::StopDataRecord(EExecution& ExecutionBranch)
{
    bool worked = UHPGliaClient::StopDataRecord();

    ExecutionBranch = worked ? EExecution::Then : EExecution::Error;
}

void UHPGliaBPLibrary::GetEyeTracking(FEyeTracking& OutEyeTracking, EExecution& ExecutionBranch)
{
    bool worked = UHPGliaClient::GetEyeTracking(OutEyeTracking);

    ExecutionBranch = worked ? EExecution::Then : EExecution::Error;
}

void UHPGliaBPLibrary::GetCognitiveLoad(
    FCognitiveLoad& OutCognitiveLoadStruct,
    EExecution& ExecutionBranch)
{
    bool worked = UHPGliaClient::GetCognitiveLoad(OutCognitiveLoadStruct);

    ExecutionBranch = worked ? EExecution::Then : EExecution::Error;
}

void UHPGliaBPLibrary::GetIMUArray(FIMUArray& outIMUArray, EExecution& ExecutionBranch)
{
    bool worked = UHPGliaClient::GetIMUArray(outIMUArray);

    ExecutionBranch = worked ? EExecution::Then : EExecution::Error;
}

void UHPGliaBPLibrary::GetPPGArray(FPPGArray& outPPGArray, EExecution& ExecutionBranch)
{
    bool worked = UHPGliaClient::GetPPGArray(outPPGArray);

    ExecutionBranch = worked ? EExecution::Then : EExecution::Error;
}

bool UHPGliaBPLibrary::isConnected()
{  
    return UHPGliaClient::IsConnected();
}

void UHPGliaBPLibrary::GetGliaDelegates(UHPGliaDelegates* & OutDelegates)
{
    OutDelegates = UHPGliaClient::GetGliaDelegates();
}
