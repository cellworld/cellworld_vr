// Fill out your copyright notice in the Description page of Project Settings.

// ReSharper disable StringLiteralTypo
#pragma once

#include "CoreMinimal.h"
#include "TimelineComponent.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/TimelineComponent.h"
#include "MiscUtils/Timers/EventTimer.h"
#include "ExperimentDoorBase.generated.h"

// DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnValidEventTriggered);

UCLASS()
class EXPERIMENTPLUGIN_API AExperimentDoorBase : public AActor {
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AExperimentDoorBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	bool bCanCollide = true;

	UFUNCTION(Category = "Events|Trigger")
	virtual void OnEventCooldownFinished();

	bool SetupEventCooldownTimer();
	void SetCanCallEventTrigger(bool bNewCanCallEventTrigger);
	
	const float DeltaTimeTimeline = 0.1;
	bool bCanCallEventTrigger = true;

	UPROPERTY(BlueprintReadWrite,Category= "Door Events")
	float ValidEventTriggerCooldownTime = 10.0f;
	
public:

	UPROPERTY()
	UEventTimer* TriggerCooldownTimer = nullptr;

	UFUNCTION(Category = "Events|Trigger")
	virtual bool IsValidEventTriggerReady();
	
	void RPCTest();
	// FOnValidEventTriggered OnValidEventTriggered;
	virtual void OnValidEventTrigger();
	virtual void OnRep_Owner() override;	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable, WithValidation, Category = "Events|Trigger")
	virtual void Server_OnEventTrigger();
	virtual bool Server_OnEventTrigger_Validate();
	virtual void Server_OnEventTrigger_Implementation();

	bool dbg_bFuncCalled = false;
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;
	
	/* === door animation timeline params === */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Door Animation")
	TObjectPtr<UTimelineComponent> AnimationDoorTimeline = nullptr; 

	UFUNCTION(BlueprintCallable, Category = "Door Animation")
	virtual void AnimationDoorUpdate(const FVector InVector);

	UFUNCTION(BlueprintCallable, Category = "Door Animation")
	virtual void AnimationDoorFinished(); 
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Door Animation")
	TObjectPtr<UCurveVector> AnimationDoorCurveVector = nullptr;

	// event called prior to calling bound update function (AnimationDoorUpdate)
	FOnTimelineVector OnAnimationDoorUpdateVector;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Door Animation")
	FOnTimelineEvent OnAnimationDoorFinishedEvent;

	FOnTimelineEvent OnAnimationDoorUpdate;
	FVector InitialDoorLocation;
	FVector TargetDoorLocation; 

	/* ======================================= */

	UPROPERTY(BlueprintReadWrite, Replicated, Blueprintable, EditAnywhere)
	TObjectPtr<USceneComponent> SceneRootComponent;
	
	UPROPERTY(BlueprintReadWrite, Replicated, Blueprintable, EditAnywhere)
	TObjectPtr<UBoxComponent> AnimationBoxCollision;

	UPROPERTY(BlueprintReadWrite, Replicated, Blueprintable, EditAnywhere)
	TObjectPtr<UBoxComponent> EventBoxCollision;

	UPROPERTY(BlueprintReadWrite, Replicated, Blueprintable, EditAnywhere)
	TObjectPtr<UStaticMeshComponent> DoorMesh; 
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	virtual void OnAnimationOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	                             UPrimitiveComponent* OtherComp,
	                             int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintCallable)
	virtual void OnAnimationOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	                           int32 OtherBodyIndex);
	/* overlap events */
	UFUNCTION(BlueprintCallable)
	virtual void OnEventOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintCallable)
	virtual void OnEventOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

};
