#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Tickable.h"
#include "TimerManager.h"
#include "MiscUtils/MiscUtils.h"
#include "EventTimer.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTimerFinished);

UCLASS(Blueprintable)
class MISCUTILS_API UEventTimer : public UObject, public FTickableGameObject {
	GENERATED_BODY()

	UEventTimer():Rate(1.0f), StartDelay(-1.0f), bLoop(true), TimerHandlePtr(&TimerHandle) {};
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EventTimer)
		FOnTimerFinished OnTimerFinishedDelegate;

	/* Duration of timer (in seconds). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EventTimer)
		float Rate = 1.0f; // 90 Hz 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EventTimer)
		float StartDelay = -1.0f; // no delay in starting
	
	// continue after finished (good to use if sampling every "Rate" seconds.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EventTimer)
		bool bLoop = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EventTimer)
	FTimerHandle TimerHandle;

	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EventTimer)
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
	bool Stop()  {
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
	float GetTimeElapsed() const {
		if (!TimerHandlePtr->IsValid()){ return -1.0f; }
		return GetWorld()->GetTimerManager().GetTimerElapsed(*TimerHandlePtr);  
	}
	
};

UCLASS(Blueprintable)
class MISCUTILS_API UStopwatch : public UObject {
	GENERATED_BODY()
public:
	UStopwatch():StartTime(-1.0f),bIsRunning(false){};

	virtual ~UStopwatch() override {
	};
	
	void Start() {
		StartTime = FPlatformTime::Seconds();
		bIsRunning = true; 
		UE_LOG(LogTimer, Log, TEXT("UStopwatch::Start(): StartTime = %0.2f"), StartTime)
	}

	double GetElapsedTime() const {
		if (!IsRunning()){ return -1.0f; }
		return FPlatformTime::Seconds() - StartTime;
	}
	
	double Lap() {
		const double FinalElapsedTime = GetElapsedTime(); 
		UE_LOG(LogTimer, Log, TEXT("UStopwatch::Lap(); Elapsed time: %0.2f"), FinalElapsedTime)
		return FinalElapsedTime; 	
	}

	void Reset() {
		UE_LOG(LogTimer, Log, TEXT("UStopwatch::Reset()"))
		StartTime = -1.0f;
		bIsRunning = false;
	}
		
	bool IsRunning() const {
		return bIsRunning; 
	}
	
private: 
	double StartTime = -1.0f;
	bool bIsRunning = false; 
};

