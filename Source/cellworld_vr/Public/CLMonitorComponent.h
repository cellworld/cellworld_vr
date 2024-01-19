// (c) Copyright 2020 HP Development Company, L.P.

#pragma once

#include "CoreMinimal.h"
#include "HPGliaClient.h"
#include "Components/ActorComponent.h"
#include "CLMonitorComponent.generated.h"

UCLASS()
class CELLWORLD_VR_API UCLMonitorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // Sets default values for this component's properties
    UCLMonitorComponent();

protected:
    TArray<float> samples;
    TArray<float> sd;

    float cogLoadAcum = 0;

public:

    // Called every frame
    UFUNCTION(BlueprintCallable, Category = "Cognitive Load")
    void StartMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Cognitive Load")
    void StopMonitoring();

    UFUNCTION()
    void RegisterCognitiveLoad(FCognitiveLoad CognitiveLoad);

    UFUNCTION(BlueprintCallable, Category = "Cognitive Load")
    TArray<float> GetData();

    UFUNCTION(BlueprintCallable, Category = "Cognitive Load")
    float GetMean();

    UFUNCTION(BlueprintCallable, Category = "Cognitive Load")
    TArray<float> GetSD();
};
