// Copyright HTC Corporation. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Math/Vector.h"
#include "ViveCustomHandGestureComponent.h"
#include "ViveCustomHandGestureUtils.h"
#include "ViveCustomHandGesture.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogViveCustomHandGesture, Log, All);

using namespace wvr::hand;

struct EulerAngles {
	float roll, pitch, yaw;
};

UCLASS()
class VIVECUSTOMHANDGESTURE_API AViveCustomHandGesture : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AViveCustomHandGesture();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	const FString kUnknownGesture = TEXT("Unknown");

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vive|CustomHandGesture")
	TArray< FSingleHandGesture > LeftGestures;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vive|CustomHandGesture")
	TArray< FSingleHandGesture > RightGestures;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vive|CustomHandGesture")
	TArray< FDualHandGesture > DualHandGestures;

private:
	bool validPoseLeft = false, validPoseRight = false;
	TArray< FVector > s_JointPositionsLeft, s_JointPositionsRight;
	TArray< FQuat > s_JointRotationsLeft, s_JointRotationsRight;

	EWaveThumbState m_ThumbStateLeft = EWaveThumbState::None, m_ThumbStateRight = EWaveThumbState::None;
	EWaveFingerState m_IndexStateLeft = EWaveFingerState::None, m_IndexStateRight = EWaveFingerState::None;
	EWaveFingerState m_MiddleStateLeft = EWaveFingerState::None, m_MiddleStateRight = EWaveFingerState::None;
	EWaveFingerState m_RingStateLeft = EWaveFingerState::None, m_RingStateRight = EWaveFingerState::None;
	EWaveFingerState m_PinkyStateLeft = EWaveFingerState::None, m_PinkyStateRight = EWaveFingerState::None;

	void UpdateFingerState();
	EWaveThumbState GetThumbState(FVector root, FVector node1, FVector top, bool isLeft);
	EWaveFingerState GetFingerState(FVector root, FVector node1, FVector top, bool isLeft);
	bool MatchThumbState(FThumbState state);
	bool MatchRotationSingle(FRotator rotateCondition, bool isLeft);

	bool MatchDistanceSingle(EWaveHandJoint node1, EWaveHandJoint node2, EWaveJointDistance distance, bool isLeft);
	bool MatchDistanceDual(EWaveHandJoint leftNode, EWaveHandJoint rightNode, EWaveJointDistance distance);

	FString m_LeftHandGesture = TEXT("Default"), m_RightHandGesture = TEXT("Default");
	bool MatchGestureSingle(FSingleHandSetting gesture, bool isLeft);
	FString m_DualHandGesture = TEXT("Default");
	bool MatchGestureDual(FDualHandSetting gesture);
	EulerAngles ToEulerAnglesDegree(FQuat q);
};
