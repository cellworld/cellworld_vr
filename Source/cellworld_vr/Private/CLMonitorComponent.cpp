// (c) Copyright 2020 HP Development Company, L.P.


#include "CLMonitorComponent.h"
#include "IXRTrackingSystem.h"
#include "StereoRendering.h"
#include "IHeadMountedDisplay.h"

 //Sets default values for this component's properties
UCLMonitorComponent::UCLMonitorComponent()
{
   
}

void UCLMonitorComponent::StartMonitoring() {
    UE_LOG(LogTemp, Warning, TEXT("Monitor Started"));

    samples.Empty();
    sd.Empty();
    cogLoadAcum = 0;
    UHPGliaDelegates* gliaDelegates = UHPGliaClient::GetGliaDelegates();
    gliaDelegates->OnCogLoadReceived.AddDynamic(this, &UCLMonitorComponent::RegisterCognitiveLoad);
}

void UCLMonitorComponent::StopMonitoring() {
    UE_LOG(LogTemp, Warning, TEXT("Monitor Stopped"));

    UHPGliaDelegates* gliaDelegates = UHPGliaClient::GetGliaDelegates();
    gliaDelegates->OnCogLoadReceived.RemoveDynamic(this, &UCLMonitorComponent::RegisterCognitiveLoad);
}

void UCLMonitorComponent::RegisterCognitiveLoad(FCognitiveLoad CognitiveLoad) {
    samples.Add(CognitiveLoad.CognitiveLoad);
    sd.Add(CognitiveLoad.StandardDeviation);
    cogLoadAcum += CognitiveLoad.CognitiveLoad;
}


TArray<float> UCLMonitorComponent::GetData() {
    return samples;
}

TArray<float> UCLMonitorComponent::GetSD() {
    return sd;
}

float UCLMonitorComponent::GetMean() {
    return (cogLoadAcum / samples.Num());
}
