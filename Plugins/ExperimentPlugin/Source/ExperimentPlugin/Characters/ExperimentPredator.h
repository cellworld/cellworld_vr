#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "ExperimentPredator.generated.h"

UCLASS()
class EXPERIMENTPLUGIN_API AExperimentPredator : public AActor {
	GENERATED_BODY()
public:
	// Sets default values for this actor's properties
	AExperimentPredator();

protected:
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USkeletalMeshComponent* SkeletalMeshComponent;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Predator")
	UMaterialInterface* OverlayMaterial;
	
	// essentially toggles overlay material on/off (off = 0 opacity)
	const float PToggleMaterialScalarOnCapture = 1.0f;
	const float PToggleMaterialScalarOriginal  = 0.0f;
	
	UPROPERTY()
	float OnCaptureMaterialDuration = 3.0f;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Predator")
	UMaterialInterface* M_OnCaptureAsset;

	UPROPERTY(Replicated, EditAnywhere, Category = "Predator")
	UMaterialInterface* M_OnCapture = nullptr;

	FTimerHandle RevertMaterialTimerHandle;
	
	UFUNCTION(BlueprintCallable, Category = "Predator")
	void OnCapture();

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Predator")
	TObjectPtr<USoundBase> OnCaptureSoundCue;
	
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Predator")
	void Server_PlayCaptureSound();
	bool Server_PlayCaptureSound_Validate();
	void Server_PlayCaptureSound_Implementation();

	UFUNCTION(NetMulticast, Reliable, WithValidation, BlueprintCallable, Category = "Predator")
	void Multicast_PlayCaptureSound(const FVector Location);
	bool Multicast_PlayCaptureSound_Validate(const FVector Location);
	void Multicast_PlayCaptureSound_Implementation(const FVector Location);
	
	UFUNCTION(BlueprintCallable, Category = "Predator")
	void RevertMaterial();

	/** RepNotify state */
	UPROPERTY(ReplicatedUsing = OnRep_IsCaptured)
	bool bIsCaptured = false;

	UFUNCTION()
	void OnRep_IsCaptured();

	void ApplyCaptureMaterial();
	void ApplyOriginalMaterial();

	// Helper to start the revert timer only on server
	void StartRevertTimer();
};


