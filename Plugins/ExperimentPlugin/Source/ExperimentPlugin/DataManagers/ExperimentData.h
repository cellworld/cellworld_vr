#pragma once

#include "Misc/Guid.h"
#include "CoreMinimal.h"
#include "ExperimentData.generated.h"

UCLASS(Blueprintable)
class EXPERIMENTPLUGIN_API UExperimentData : public UObject {
	GENERATED_BODY()
	UExperimentData() {
		EpisodesCompleted = 0;
		EpisodesCompletedTime = 0.0;
	};
public:

	void Init() {
		EpisodesCompleted = 0;
		EpisodesCompletedTime = 0.0;
	}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
	int EpisodesCompleted;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
	float EpisodesCompletedTime;

	void AddEpisodeCompleted() { EpisodesCompleted += 1; }
	void AddEpisodeCompletedTime(const float InEpisodeDuration) { EpisodesCompletedTime += InEpisodeDuration; }

	int GetEpisodesCompleted() const { return EpisodesCompleted; }
	float GetEpisodesCompletedTime() const { return EpisodesCompletedTime; }

	/*Increments episode and adds a duration to the total completed time
	 * @param InEpisodeDuration: Duration of last completed episode.
	 * @see AddEpisodeCompleted()
	 * @see AddEpisodeCompletedTime(const float InEpisodeDuration)
	 */
	void AddEpisodeAndCompletedTime(const float InEpisodeDuration) {
		AddEpisodeCompleted();
		AddEpisodeCompletedTime(InEpisodeDuration); 
	}
	
};