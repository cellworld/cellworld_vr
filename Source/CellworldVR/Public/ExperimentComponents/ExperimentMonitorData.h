#pragma once

#include "Misc/Guid.h"
#include "ExperimentMonitorData.generated.h"

USTRUCT(Blueprintable)
struct FExperimentMontitorData
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		FString data; 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		bool bInEpisode; 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		bool bInExperiment; 
};