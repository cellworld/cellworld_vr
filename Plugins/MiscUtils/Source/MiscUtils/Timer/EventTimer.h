#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Tickable.h"
#include "TimerManager.h"
#include "EventTimer.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTimerFinished);

UCLASS(Blueprintable)
class MISCUTILS_API UEventTimer : public UObject, public FTickableGameObject {
	GENERATED_BODY()

	UEventTimer():Rate(1.0f), StartDelay(-1.0f), bLoop(true), TimerHandlePtr(&TimerHandle) {};
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EventTimer)
		FOnTimerFinished OnTimerFinishedDelegate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EventTimer)
		// float Rate = 1.0f / 90.0f; // 90 Hz 
		float Rate = 1.0f; // 90 Hz 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EventTimer)
		float StartDelay = -1.0f; // no delay in starting
	
	// continue after finished (good to use if sampling every "Rate" seconds.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EventTimer)
		bool bLoop = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EventTimer)
	FTimerHandle TimerHandle;

	FTimerHandle* TimerHandlePtr = &TimerHandle;

	virtual void Tick(float DeltaTime) override {
		// UE_LOG(LogTemp, Log, TEXT("[EventTimer] Time remaining: %0.4f"), this->GetTimeRemaining())	
	};
	virtual ETickableTickType GetTickableTickType() const override {
		return ETickableTickType::Always;
	}
	virtual TStatId GetStatId() const override {
		RETURN_QUICK_DECLARE_CYCLE_STAT(FMyTickableThing, STATGROUP_Tickables);
	}
	virtual bool IsTickableWhenPaused() const override {
		return true;
	}
	virtual bool IsTickableInEditor() const override  {
		return false;
	}
	
	void OnTimerFinished() const {
		OnTimerFinishedDelegate.Broadcast();
	}

	void SetRateHz(const float InRate) {
		this->Rate = 1.0f / InRate; 
	}

	void SetRateSeconds(const float InRate) {
		this->Rate = InRate; 
	}
	
	bool Start() const {
		if (!TimerHandlePtr) { return false; }

		GetWorld()->GetTimerManager().SetTimer(*TimerHandlePtr,
			this, &UEventTimer::OnTimerFinished, // func to call when done 
			Rate, bLoop, StartDelay);
		
		return TimerHandlePtr->IsValid(); 
	}
	bool Stop() {
		if (!TimerHandlePtr) { return false; }
		
		GetWorld()->GetTimerManager().ClearTimer(*TimerHandlePtr);
		return !TimerHandlePtr->IsValid();
	}
	
	bool IsRunning() const {
		if (!TimerHandlePtr) { return false; }
		return GetWorld()->GetTimerManager().IsTimerActive(*TimerHandlePtr);
	}
	
	float GetTimeRemaining() const {
		if (!TimerHandlePtr->IsValid()) { return -1.0f; }
		return GetWorld()->GetTimerManager().GetTimerRemaining(*TimerHandlePtr);
	}
	
};
