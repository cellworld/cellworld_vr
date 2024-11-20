// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "BoxComponent.h"
#include "GameModeMain.h"
#include "PawnMain.h"
#include "cellworld_vr/cellworld_vr.h"
#include "Evaluation/IMovieSceneEvaluationHook.h"
#include "GameFramework/Actor.h"
#include "DoorExperiment.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAnimationCollisionOverlap);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEventCollisionBeginOverlap);

UCLASS(Blueprintable)
class CELLWORLD_VR_API ADoorExperiment : public AActor {
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADoorExperiment() {
		DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
		DoorMesh->SetRelativeScale3D(FVector(1.0f,2.0f,1.0f));
		DoorMesh->SetupAttachment(RootComponent);
		DoorMesh->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);

		AnimationCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("AnimationCollisionBox"));
		AnimationCollisionBox->SetupAttachment(DoorMesh);
		AnimationCollisionBox->SetRelativeLocation(FVector(200.0f,0.0f,0.0f));
		AnimationCollisionBox->SetBoxExtent(FVector(150.0f,100.0f,50.f),false);
		AnimationCollisionBox->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);

		EventCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("EventCollisionBox"));
		EventCollisionBox->SetupAttachment(DoorMesh);
		EventCollisionBox->SetRelativeLocation(FVector(-100.0f,0.0f,0.0f));
		EventCollisionBox->SetBoxExtent(FVector(150.0f,100.0f,50.f),false);
		EventCollisionBox->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
		EventCollisionBox->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::ADoorExperiment::OnEventCollisionBeginOverlap);

		// default
		UE_LOG(LogExperiment, Log, TEXT("[ADoorExperiment::ADoorExperiment] Default TriggeringPawnClass: APawnMain"))
		TriggeringPawnClass = APawnMain::StaticClass();
	}

	UFUNCTION(Blueprintable, BlueprintCallable)
	void SetEventTriggeringPawnClass(TSubclassOf<AActor> NewPawnClass) {
		UE_LOG(LogExperiment, Log, TEXT("[SetEventTriggeringPawnClass] Setting new triggering actor class."))
		TriggeringPawnClass = NewPawnClass;
	}

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override {};

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override {}

	/* delegates */
	// called when successful event overlap occurs 
	UPROPERTY(Blueprintable, BlueprintReadWrite)
	FOnEventCollisionBeginOverlap OnEventCollisionBeginOverlapDelegate; 

	UFUNCTION(BlueprintCallable, Blueprintable) // todo: within game mode or idk where tf bind to SpawnOcclusions!
	void OnEventCollisionBeginOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor,
		class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
		if (!ensure(TriggeringPawnClass->IsValidLowLevelFast())) { return; }
		if (!ensure(OtherActor->IsValidLowLevelFast())) { return; }
		
		if (TriggeringPawnClass && OtherActor && OtherActor->IsA(TriggeringPawnClass)) {
			// Trigger animation here
			UE_LOG(LogExperiment, Warning, TEXT("[OnEventCollisionBeginOverlap] Door triggered by the specified pawn class!"));
			OnEventCollisionBeginOverlapDelegate.Broadcast();
		}else {
			UE_LOG(LogExperiment, Warning, TEXT("[OnEventCollisionBeginOverlap] FAILED - Door triggered by the specified pawn class!"));
		}
	};
	
	// Set this in the editor to the specific class that should trigger the door
	UPROPERTY(EditAnywhere, Category = "TriggerComponents")
	TSubclassOf<AActor> TriggeringPawnClass;
	
	// Box component will call an event to trigger user-defined event (such as spawn or send message)
	UPROPERTY(VisibleAnywhere, Blueprintable, Category = "TriggerComponents")
	class UBoxComponent* EventCollisionBox;

	// Box component will trigger door animations
	UPROPERTY(VisibleAnywhere, Blueprintable, Category = "TriggerComponents")
	class UBoxComponent* AnimationCollisionBox;

	// Static mesh component for the cube
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TriggerComponents")
	class UStaticMeshComponent* DoorMesh;

	UFUNCTION(Blueprintable)
	virtual void OnEventCollisionOverlap() {};
	
	/* overlap events */
	UFUNCTION(Blueprintable)
	void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {};

	UFUNCTION(Blueprintable)
	void OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {}

};

