#pragma once
#include "BlueprintEditor.h"

#include "EventTimer.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTimerFinished);

USTRUCT(Blueprintable)
struct FEventTimer
{
	GENERATED_BODY()

public:
	
	FEventTimer()
	{
	};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EventTimer)
		FOnTimerFinished OnTimerFinishedDelegate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EventTimer)
		FTimerHandle TimerHandle;

	FTimerHandle* TimerHandlePtr = &TimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EventTimer)
		float Rate = 1.0f / 90.0f; // 90 Hz 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EventTimer)
		float StartDelay = -1.0f; // no delay in starting
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EventTimer)
		bool bLoop = true; // continue after finished (good to use if sampling every "Rate" seconds.

	FTimerDelegate TimerDelegate;
	
	void OnTimerFinished() const {
		UE_LOG(LogTemp, Log, TEXT("[FEventTimer::OnTimerFinished]"))
		OnTimerFinishedDelegate.Broadcast(); 
	};


	bool IsRunning() const
	{
		if (!TimerHandlePtr->IsValid()){ return false; }
		return GEngine->GetWorld()->GetTimerManager().IsTimerActive(*TimerHandlePtr); 
	}
	
	bool Start() {

		// TimerDelegate.BindRaw(this, &FEventTimer::OnTimerFinished);
		
		// GEngine->GetWorld()->GetTimerManager().SetTimer(TimerHandle,
		// 	TimerDelegate, Rate, bLoop, StartDelay);

		// GEngine->GetWorld()->GetTimerManager().SetTimer(TimerHandle, 10, bLoop, StartDelay);
		//
		return true; 
	};
	
	bool Stop() const {
		if (!TimerHandlePtr->IsValid()) {
			UE_LOG(LogTemp, Error,
				TEXT("[AExperimentServiceMonitor::StopTimerEpisode] Failed, TimerHandleIn is null."));
			return false;
		}

		check(TimerHandle.IsValid())
		GEngine->GetWorld()->GetTimerManager().ClearTimer(*TimerHandlePtr);
		// TimerHandle.Invalidate();
		// check(!TimerHandle.IsValid())
		return true;
	};

	float GetTimeRemaining() const {
		if (!TimerHandlePtr->IsValid()){ return -1.0f; }
		if (!GEngine->GetWorld()->GetTimerManager().IsTimerActive(*TimerHandlePtr)) { return -1.0f; }
		return GEngine->GetWorld()->GetTimerManager().GetTimerRemaining(*TimerHandlePtr);	
	}
};