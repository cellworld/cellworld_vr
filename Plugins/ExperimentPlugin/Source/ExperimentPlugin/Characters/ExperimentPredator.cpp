#include "ExperimentPredator.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

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

	if (!OnCaptureSoundCue) {
		UE_LOG(LogTemp, Warning,
			TEXT("[AExperimentPredator::AExperimentPredator] OnCaptureSoundCue Not assigned in BP. Looking for backup."));
		static ConstructorHelpers::FObjectFinder<USoundBase> OnCaptureCueLoad(
			 TEXT("SoundCue'/Game/SoundFX/fail_sound_cue.fail_sound_cue'")
		 );

		if (OnCaptureCueLoad.Object != nullptr) {
			OnCaptureSoundCue = OnCaptureCueLoad.Object;
			UE_LOG(LogTemp, Log, TEXT("[AExperimentPredator::AExperimentPredator] OnCaptureSoundCue valid"));
		}else{UE_LOG(LogTemp, Error, TEXT("[AExperimentPredator::AExperimentPredator] OnCaptureSoundCue Null"));}
	} else {
		UE_LOG(LogTemp, Log,
			TEXT("[AExperimentPredator::AExperimentPredator] OnCaptureSoundCue found in BP valid."));
	}
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

bool AExperimentPredator::Server_PlayCaptureSound_Validate() {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentPredator::Server_PlayCaptureSound_Validate]"))
	return true;
}

void AExperimentPredator::Server_PlayCaptureSound_Implementation() {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentPredator::Server_PlayCaptureSound_Implementation]"))
	if (OnCaptureSoundCue->IsValidLowLevelFast()) {
		const FVector OnCaptureSoundLocation = GetActorLocation();
		UE_LOG(LogTemp, Log, TEXT("[AExperimentPredator::Server_PlayCaptureSound_Implementation] Playing sound at location: %s!"),
			*OnCaptureSoundLocation.ToString())
		Multicast_PlayCaptureSound(OnCaptureSoundLocation);
		return;
	}
	UE_LOG(LogTemp, Error, TEXT("[AExperimentPredator::Server_PlayCaptureSound_Implementation] Sound not valid!"))
}

bool AExperimentPredator::Multicast_PlayCaptureSound_Validate(const FVector Location) {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentPredator::Multicast_PlayCaptureSound_Validate]"))
	return true;
}

void AExperimentPredator::Multicast_PlayCaptureSound_Implementation(const FVector Location) {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentPredator::Multicast_PlayCaptureSound_Implementation]"))
	if (OnCaptureSoundCue->IsValidLowLevelFast()) {
		UE_LOG(LogTemp, Log, TEXT("[Multicast_PlayCaptureSound_Implementation] Playing sound!"))
		UGameplayStatics::PlaySoundAtLocation(this, OnCaptureSoundCue, Location, 10.0f);
		return;
	}
	UE_LOG(LogTemp, Error, TEXT("[Multicast_PlayCaptureSound_Implementation] Sound not valid!"))
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
		Server_PlayCaptureSound();
		// Multicast_PlayCaptureSound_Implementation(GetActorLocation());
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
	DOREPLIFETIME(AExperimentPredator, OnCaptureSoundCue);
}

