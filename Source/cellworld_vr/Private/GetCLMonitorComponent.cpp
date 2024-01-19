#include "GetCLMonitorComponent.h"

UGetCLMonitorComponent::UGetCLMonitorComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UGetCLMonitorComponent::StartMonitoring() {
    UE_LOG(LogTemp, Warning, TEXT("Monitor Started"));

    samples.Empty();
    cogLoadAcum = 0;
    UHPGliaDelegates* gliaDelegates = UHPGliaClient::GetGliaDelegates();
    gliaDelegates->OnCogLoadReceived.AddDynamic(this, &UGetCLMonitorComponent::RegisterCognitiveLoad);
}

void UGetCLMonitorComponent::StopMonitoring() {
    UE_LOG(LogTemp, Warning, TEXT("Monitor Stopped"));
    UHPGliaDelegates* gliaDelegates = UHPGliaClient::GetGliaDelegates();
    gliaDelegates->OnCogLoadReceived.RemoveDynamic(this, &UGetCLMonitorComponent::RegisterCognitiveLoad);
}

void UGetCLMonitorComponent::RegisterCognitiveLoad(FCognitiveLoad CognitiveLoad) {
    samples.Add(CognitiveLoad.CognitiveLoad);
    cogLoadAcum += CognitiveLoad.CognitiveLoad;
}

TArray<float> UGetCLMonitorComponent::GetData() {
    return samples;
}

float UGetCLMonitorComponent::GetMean() {
    return (cogLoadAcum / samples.Num());
}

