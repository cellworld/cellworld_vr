#pragma once
#include "CoreMinimal.h"
#include "CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"
#include "MiscUtils/Timer/EventTimer.h"
#include "GameFramework/Pawn.h"
#include "Containers/Array.h" 
#include "BotEvadePawn.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMovementDetectedNew, FVector, Location, FRotator, Rotation);

UCLASS(Blueprintable)
class ABotEvadePawn : public APawn {
	GENERATED_BODY()
public:
	ABotEvadePawn();
	void SetupUpdateRoomScaleLocation(UCameraComponent* InCameraComponent);
	void UpdateRoomScaleLocation();
	UFUNCTION()
	void OnMovementDetected();

	UPROPERTY(EditAnywhere)
	// TObjectPtr<UEventTimer> EventTimer;
	UEventTimer* EventTimer;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpatialComponents")
	TObjectPtr<UCapsuleComponent> CapsuleComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpatialComponents")
	TObjectPtr<USceneComponent> VROrigin;
	UPROPERTY(BlueprintCallable, Blueprintable)
	FOnMovementDetectedNew MovementDetectedEvent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpatialComponents")
	TObjectPtr<UCameraComponent> CameraUpdateRoomscaleLocation; 	

	UFUNCTION()
	bool StartPositionSamplingTimer(const float InRateHz = 60.0f);

	UFUNCTION()
	bool StopPositionSamplingTimer();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Reset() override;

	void MoveForward(float AxisValue);
	void MoveRight(float AxisValue);
	void Turn(float AxisValue);
	void LookUp(float AxisValue);
protected:
	bool bSetupUpdateRoomScaleLocationComplete = false; 
};
