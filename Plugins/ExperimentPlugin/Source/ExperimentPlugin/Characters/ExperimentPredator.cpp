#include "ExperimentPredator.h"
#include "Net/UnrealNetwork.h"

AExperimentPredator::AExperimentPredator() : Super() {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentPredator::AExperimentPredator()]"));
	SetActorEnableCollision(false);
	SetReplicates(true);
	SetNetDormancy(ENetDormancy::DORM_Never);
	bNetLoadOnClient		 = true;
	NetUpdateFrequency	 = 100.0f;
	MinNetUpdateFrequency = 60.0f;

	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
	SetRootComponent(SkeletalMeshComponent);
	SkeletalMeshComponent->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
}

void AExperimentPredator::PostInitializeComponents() {
	Super::PostInitializeComponents();
	SetReplicateMovement(true);
}

void AExperimentPredator::BeginPlay() {
	Super::BeginPlay();
}

void AExperimentPredator::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}

void AExperimentPredator::OnCapture() {
	if (!HasAuthority()) return;
	
	bIsCaptured = true;
	OnRep_IsCaptured(); // Apply material immediately on server
	StartRevertTimer();
}

void AExperimentPredator::RevertMaterial() {
	if (!HasAuthority()) return;
	UE_LOG(LogTemp, Log, TEXT("[AExperimentPredator::RevertMaterial]"))
	bIsCaptured = false;
	OnRep_IsCaptured(); // Apply revert immediately on server
}

void AExperimentPredator::OnRep_IsCaptured() {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentPredator::OnRep_IsCaptured] bIsCaptured: %s"),
		bIsCaptured ? TEXT("True") : TEXT("False"))

	if (bIsCaptured) {
		ApplyCaptureMaterial();
	} else {
		ApplyOriginalMaterial();
	}
}

void AExperimentPredator::ApplyCaptureMaterial() {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentPredator::ApplyCaptureMaterial]"))

	if (!SkeletalMeshComponent) { return; }
	SkeletalMeshComponent->SetOverlayMaterialMaxDrawDistance(0.0f); // cheat code (oopsie..): hides the material 
	UE_LOG(LogTemp, Log, TEXT("[AExperimentPredator::ApplyCaptureMaterial] OverlayMaterial changed"))
}

void AExperimentPredator::ApplyOriginalMaterial() {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentPredator::ApplyOriginalMaterial]"))
	if (!SkeletalMeshComponent) { return; }
	SkeletalMeshComponent->SetOverlayMaterialMaxDrawDistance(-1.0f); // cheat code (oopsie..): show the material 
	UE_LOG(LogTemp, Log, TEXT("[AExperimentPredator::ApplyOriginalMaterial] OverlayMaterial changed"))
}

void AExperimentPredator::StartRevertTimer() {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentPredator::StartRevertTimer]"))
	GetWorld()->GetTimerManager().SetTimer(
		RevertMaterialTimerHandle,
		this,
		&AExperimentPredator::RevertMaterial,
		OnCaptureMaterialDuration,
		false
	);
}

void AExperimentPredator::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AExperimentPredator, bIsCaptured);
}

