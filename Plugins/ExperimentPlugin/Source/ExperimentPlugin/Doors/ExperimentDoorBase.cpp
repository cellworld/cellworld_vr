// Fill out your copyright notice in the Description page of Project Settings.

#include "ExperimentDoorBase.h"

#include "CapsuleComponent.h"
#include "Curves/CurveVector.h"
#include "ExperimentPlugin/Characters/ExperimentCharacter.h"

// Sets default values
AExperimentDoorBase::AExperimentDoorBase() {
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick		   = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	static ConstructorHelpers::FObjectFinder<UCurveVector> Curve(TEXT("/Script/Engine.CurveVector'/Game/Levels/BaseDoors/AnimationDoorCurveVector.AnimationDoorCurveVector'"));
	check(Curve.Succeeded());
	AnimationDoorCurveVector = Curve.Object;
	
	SceneRootComponent = CreateDefaultSubobject<USceneComponent>(FName("SceneRootComponent"));
	SceneRootComponent->AddRelativeLocation(FVector(0.0f,0.0f,-25.0f));
	RootComponent = SceneRootComponent;
	
	AnimationBoxCollision = CreateDefaultSubobject<UBoxComponent>(FName("AnimationBoxCollisionn"));
	AnimationBoxCollision->AddRelativeLocation(FVector(200.0f,0.0f,0.0f));
	AnimationBoxCollision->OnComponentBeginOverlap.AddDynamic(this, &AExperimentDoorBase::OnAnimationOverlapBegin);
	AnimationBoxCollision->OnComponentEndOverlap.AddDynamic(this, &AExperimentDoorBase::OnAnimationOverlapEnd);
	AnimationBoxCollision->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
	AnimationBoxCollision->SetBoxExtent(FVector(150.0f,100.0f,50.0f));
	AnimationBoxCollision->SetupAttachment(RootComponent);
	
	EventBoxCollision = CreateDefaultSubobject<UBoxComponent>(FName("EventBoxCollision"));
	EventBoxCollision->AddRelativeLocation(FVector(-100.0f,0.0f,0.0f));
	EventBoxCollision->OnComponentBeginOverlap.AddDynamic(this, &AExperimentDoorBase::OnEventOverlapBegin);
	EventBoxCollision->OnComponentEndOverlap.AddDynamic(this, &AExperimentDoorBase::OnEventOverlapEnd);
	EventBoxCollision->SetBoxExtent(FVector(50.0f,100.0f,50.0f));
	EventBoxCollision->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
	EventBoxCollision->SetupAttachment(RootComponent);
	
	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorStaticMesh"));
	DoorMesh->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	DoorMesh->SetWorldScale3D(FVector(1.0f,2.0f,1.0f));
	DoorMesh->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	DoorMesh->SetupAttachment(RootComponent);
	
	InitialDoorLocation = DoorMesh->GetRelativeLocation();
	TargetDoorLocation  = InitialDoorLocation + FVector(0.0f,0.0f,500.0f);
	
	static ConstructorHelpers::FObjectFinder<UStaticMesh>
		DoorMeshAsset(TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'"));

	if (DoorMeshAsset.Succeeded()) {
		UE_LOG(LogTemp, Log,TEXT("[AExperimentDoorBase::AExperimentDoorBase] Set mesh: OK"))
		DoorMesh->SetStaticMesh(DoorMeshAsset.Object);
	} else {
		UE_LOG(LogTemp,Error,TEXT("[AExperimentDoorBase::AExperimentDoorBase] Set mesh: Failed"));
	}

	static ConstructorHelpers::FObjectFinder<UMaterial>
	DoorMaterialAsset(TEXT("Material'/Game/NecrosUtilityMatPack/materials/floor/masters/m_diamondplate_01.m_diamondplate_01'"));
	if (DoorMaterialAsset.Succeeded()) {
		UE_LOG(LogTemp,Log,TEXT("[AExperimentDoorBase::AExperimentDoorBase] Set Material: OK"))
		DoorMesh->SetMaterial(0, DoorMaterialAsset.Object);
	}else {
		UE_LOG(LogTemp, Error,TEXT("[AExperimentDoorBase::AExperimentDoorBase] Set Material: Failed"));
	}
}

// Called when the game starts or when spawned
void AExperimentDoorBase::BeginPlay() {
	Super::BeginPlay();

	if (!ensure(AnimationDoorCurveVector)) return;

	// AnimationDoorCurveVector = NewObject<UCurveVector>(this,FName("AnimationDoorCurveVector"));
	OnAnimationDoorUpdateVector.BindUFunction(this, FName("AnimationDoorUpdate"));
	OnAnimationDoorFinishedEvent.BindUFunction(this, FName("AnimationDoorFinished"));

	AnimationDoorTimeline = NewObject<UTimelineComponent>(this,FName("AnimationDoorTimeline"));
	AnimationDoorTimeline->RegisterComponent();
	if (!ensure(AnimationDoorTimeline)) { return; }
	
	// AnimationDoorTimeline->SetLooping(false);
	AnimationDoorTimeline->AddInterpVector(AnimationDoorCurveVector, OnAnimationDoorUpdateVector);
	AnimationDoorTimeline->SetTimelineFinishedFunc(OnAnimationDoorFinishedEvent);

	if (!ensure(SetupEventCooldownTimer())) { return; }
	SetCanCallEventTrigger(true);
}

void AExperimentDoorBase::OnEventCooldownFinished() {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentDoorBase::OnEventCooldownFinished] Setting: bCanCallEventTrigger = true"))
	if(GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("TIMER FINISHED"));
	SetCanCallEventTrigger(true);
	bCanCollide = true;
	// EventBoxCollision->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnEventOverlapBegin);
	EventBoxCollision->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);

}

bool AExperimentDoorBase::SetupEventCooldownTimer() {
	TriggerCooldownTimer = NewObject<UEventTimer>(this, UEventTimer::StaticClass());
	TriggerCooldownTimer->AddToRoot();
	if (TriggerCooldownTimer->IsValidLowLevel()) {
		UE_LOG(LogTemp, Log, TEXT("[AExperimentDoorBase::SetupEventCooldownTimer] Starting at Rate 10s!"))
		TriggerCooldownTimer->SetRateSeconds(3.0f); //todo: make sampling rate GI variable (or somewhere relevant) 
		TriggerCooldownTimer->bLoop = false;
		TriggerCooldownTimer->OnTimerFinishedDelegate.AddDynamic(this, &AExperimentDoorBase::OnEventCooldownFinished);
	} else {
		UE_LOG(LogTemp, Error, TEXT("[AExperimentDoorBase::SetupEventCooldownTimer]  Failed! EventTimer NULL!"))
		return false;
	}
	return true;
}

void AExperimentDoorBase::SetCanCallEventTrigger(bool bNewCanCallEventTrigger) {
	bCanCallEventTrigger = bNewCanCallEventTrigger;
	UE_LOG(LogTemp, Warning,
		TEXT("[AExperimentDoorBase::SetCanCallEventTrigger] bCanCallEventTrigger new state: %i (bCanCallEventTrigger: %i)"),
		bNewCanCallEventTrigger, bCanCallEventTrigger)
	if (!ensure(EventBoxCollision)) { return; }
	// if (bCanCallEventTrigger) {
	// 	EventBoxCollision->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
	// }else {
	// 	EventBoxCollision->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	// }
	if(GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green,
			FString::Printf(TEXT("SetCanCallEventTrigger: New state Called: %i"),
				bCanCallEventTrigger));
}

bool AExperimentDoorBase::IsValidEventTriggerReady() {
	return bCanCallEventTrigger;
}

void AExperimentDoorBase::OnValidEventTrigger() {
	if(GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("OnValidEventTrigger Called"));
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentDoorBase::OnValidEventTrigger] Called"))
	// todo: start cooldown timer
	if (!ensure(TriggerCooldownTimer)) { return; }

	if (IsValidEventTriggerReady()) {
		SetCanCallEventTrigger(false);
		if (!ensure(TriggerCooldownTimer->Start())) { return; }
		if(GEngine) {
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("STARTED TIMER"));
		}
	} else {
		if(GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("TIMER ALREADY RUNNING"));
	}
	
}

void AExperimentDoorBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void AExperimentDoorBase::AnimationDoorUpdate(const FVector InVector) {
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentDoorBase::AnimationDoorUpdate] InVector: %s"),
		*InVector.ToString())
	if (!ensure(DoorMesh)) { return; }

	const FVector NewLocation = FMath::Lerp(InitialDoorLocation,TargetDoorLocation,InVector);
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentDoorBase::AnimationDoorUpdate] NewLocation: %s"),
			*NewLocation.ToString())

	const float TimelineValue = AnimationDoorTimeline->GetPlaybackPosition();
	const FVector TimelineVec = AnimationDoorCurveVector->GetVectorValue(TimelineValue);
	
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentDoorBase::AnimationDoorUpdate] TimelineVector: %s"),
			*TimelineVec.ToString())
	
	DoorMesh->SetRelativeLocation(NewLocation,false,nullptr,ETeleportType::TeleportPhysics);
}

void AExperimentDoorBase::AnimationDoorFinished() {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentDoorBase::AnimationDoorFinished] Called"))
}

// Called every frame
void AExperimentDoorBase::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
	if (!ensure(AnimationDoorTimeline)) return; 
	AnimationDoorTimeline->TickComponent(DeltaTime, ELevelTick::LEVELTICK_TimeOnly, NULL);
}

void AExperimentDoorBase::OnAnimationOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {

	if(GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, FString::Printf(TEXT("ANIMATION: BEGIN! Name: %s"),
			*OtherActor->GetName()));
	
	if (!AnimationDoorTimeline->IsPlaying()) {
		UE_LOG(LogTemp, Warning, TEXT("[AExperimentDoorBase::OnAnimationOverlapBegin] ANIMATION: Starting timeline!"))
		AnimationDoorTimeline->Play();
	}else {
		if(GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("ANIMATION: ALREADY RUNNING!"));
	}
	
}

void AExperimentDoorBase::OnAnimationOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                       UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {

	if (!AnimationDoorTimeline->IsReversing()) {
		UE_LOG(LogTemp, Warning, TEXT("[AExperimentDoorBase::OnAnimationOverlapBegin] ANIMATION: Starting timeline!"))
		AnimationDoorTimeline->Reverse();
	}else {
		if(GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("ANIMATION: ALREADY!"));
	}
}

void AExperimentDoorBase::OnEventOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {

	// EventBoxCollision->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	if (!ensure(EventBoxCollision)) return;
	EventBoxCollision->OnComponentBeginOverlap.RemoveDynamic(this,&ThisClass::OnEventOverlapBegin);
	// if (!bCanCollide) {
	// 	if(GEngine)
	// 		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, FString::Printf(TEXT("EVENT: CAN'T COLLIDE")));
	// 	return;
	// };
	bCanCollide = false;
	if(GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, FString::Printf(TEXT("EVENT: BEGIN! Name: %s | Comp: %i"),
			*OtherActor->GetName(), OtherComp->GetUniqueID()));
	
	if (!ensure(OtherActor->IsValidLowLevelFast())) { return; }

	ACharacter* CharacterCast = Cast<ACharacter>(OtherActor);
	if (!CharacterCast) {
		UE_LOG(LogTemp, Warning, TEXT("[AExperimentDoorBase::OnEventOverlapBegin] EVENT AExperimentCharacter Cast failed - NULL"))
		return;
	}

	UCapsuleComponent* OtherCapsuleCast = Cast<UCapsuleComponent>(OtherComp);
	
	UE_LOG(LogTemp, Warning, TEXT("Overlap Component: %s (ID: %d)"), *OtherComp->GetName(), OtherComp->GetUniqueID());
	if (OtherCapsuleCast && OtherCapsuleCast == CharacterCast->GetCapsuleComponent()) {
		UE_LOG(LogTemp, Warning, TEXT("valid capsule component"))

		if (!IsValidEventTriggerReady()) {
			UE_LOG(LogTemp, Warning, TEXT("[AExperimentDoorBase::OnEventOverlapBegin] EVENT  Timer not ready!"))
			return;
		}
		
		if (!ensure(EventBoxCollision)) { return; }
		EventBoxCollision->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
		OnValidEventTrigger();
	}
}

void AExperimentDoorBase::OnEventOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                            UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
	if(GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("Event: END!"));
}

