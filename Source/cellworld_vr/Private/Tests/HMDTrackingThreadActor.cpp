// Fill out your copyright notice in the Description page of Project Settings.


#include "Tests/HMDTrackingThreadActor.h"

// Sets default values
AHMDTrackingThreadActor::AHMDTrackingThreadActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	
}

// Called when the game starts or when spawned
void AHMDTrackingThreadActor::BeginPlay()
{
	Super::BeginPlay();

	AHMDTrackingThreadActor::InitThread();
	
}

void AHMDTrackingThreadActor::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (CurrentRunningThread && WorkerThread) {
		// need to suspen thread THEN stop it. You can't stop and kill randomly
		CurrentRunningThread->Suspend(true);
		WorkerThread->bStopThread = true;
		CurrentRunningThread->Suspend(false);
		CurrentRunningThread->Kill(false); // was false
		CurrentRunningThread->WaitForCompletion();
	}

	/* destroy this actor */
	UE_LOG(LogTemp, Log, TEXT("[AHMDTrackingThreadActor::EndPlay()] Calling Destroy()"));
}

void AHMDTrackingThreadActor::InitThread()
{
	UE_LOG(LogTemp, Warning, TEXT("[AHMDTrackingThreadActor::InitThread] Start."));

	World = nullptr;
	if (GEngine) {
		World = GEngine->GetWorld();
		if (World == nullptr) {
			World = GetWorld();
		}
	}

	if (World == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("[AHMDTrackingThreadActor::InitThread] World ref from actor null. Aborting."));
		return;
	}

	/* to do: pass pawn reference */
	UE_LOG(LogTemp, Error, TEXT("[AHMDTrackingThreadActor::InitThread] Before Worker Thread."));

	/* create thread object */
	WorkerThread = new FHMDTrackingThread(this);
	CurrentRunningThread = FRunnableThread::Create(WorkerThread, TEXT("t_TestHMDTrackingThread"));
}

// Called every frame
void AHMDTrackingThreadActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool AHMDTrackingThreadActor::SaveData(FXRHMDData* Data)
{
	return false;
}

void AHMDTrackingThreadActor::PrintData()
{
}

