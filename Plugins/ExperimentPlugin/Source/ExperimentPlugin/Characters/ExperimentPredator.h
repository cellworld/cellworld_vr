#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExperimentPredator.generated.h"

UCLASS()
class EXPERIMENTPLUGIN_API AExperimentPredator : public AActor {
	GENERATED_BODY()
public:
	// Sets default values for this actor's properties
	AExperimentPredator();

	void PostInitializeComponents() override;
	void BeginPlay() override;
	void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Predator")
	void OnCapture();

	UFUNCTION(BlueprintCallable, Category = "Predator")
	void RevertMaterial();
};


