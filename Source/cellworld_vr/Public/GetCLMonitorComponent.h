#pragma once
#include "CoreMinimal.h"
#include "HPGliaClient.h"
#include "Components/ActorComponent.h"
#include "GetCLMonitorComponent.generated.h"

UCLASS(ClassGroup = (GliaDemos), meta = (BlueprintSpawnableComponent, Blueprintable))
class CELLWORLD_VR_API UGetCLMonitorComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UGetCLMonitorComponent();

protected:

public:
    TArray<float> samples;
    float cogLoadAcum = 0;

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
};

