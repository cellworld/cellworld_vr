#pragma once
#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Pawn.h"
#include "ExperimentPredator.generated.h"

UCLASS()
class EXPERIMENTPLUGIN_API AExperimentPredator : public APawn
{
	GENERATED_BODY()
public:
	// Sets default values for this actor's properties
	AExperimentPredator();
	TObjectPtr<UStaticMeshComponent> SphereMeshComponent;
	class USkeletalMeshComponent* SkeletalMeshComponent;
};
