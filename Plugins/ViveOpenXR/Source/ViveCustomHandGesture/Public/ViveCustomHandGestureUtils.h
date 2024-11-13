// Copyright HTC Corporation. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "ViveCustomHandGestureUtils.generated.h"

UENUM(BlueprintType, Category = "Vive|CustomHandGesture")
enum class EWaveGestureType : uint8
{
	Invalid = 0,//WVR_HandGestureType::WVR_HandGestureType_Invalid,	/**< The gesture is invalid. */
	Unknown = 1,//WVR_HandGestureType::WVR_HandGestureType_Unknown, /**< Unknown gesture type. */
	Fist = 2,//WVR_HandGestureType::WVR_HandGestureType_Fist,    /**< Represent fist gesture. */
	Five = 3,//WVR_HandGestureType::WVR_HandGestureType_Five,    /**< Represent five gesture. */
	OK = 4,//WVR_HandGestureType::WVR_HandGestureType_OK,      /**< Represent ok gesture. */
	Like = 5,//WVR_HandGestureType::WVR_HandGestureType_ThumbUp, /**< Represent thumb up gesture. */
	Point = 6,//WVR_HandGestureType::WVR_HandGestureType_IndexUp, /**< Represent index up gesture. */
	Palm_Pinch = 7,//WVR_HandGestureType::WVR_HandGestureType_Palm_Pinch,
	Yeah = 8,//WVR_HandGestureType::WVR_HandGestureType_Yeah     /**< Represent yeah gesture. */
};

static const unsigned int EWaveHandJointCount = 26;
UENUM(BlueprintType, Category = "Vive|CustomHandGesture")
enum class EWaveHandJoint : uint8
{
	Palm = 0,//WVR_HandJoint::WVR_HandJoint_Palm,
	Wrist = 1,//WVR_HandJoint::WVR_HandJoint_Wrist,
	Thumb_Joint0 = 2,//WVR_HandJoint::WVR_HandJoint_Thumb_Joint0,
	Thumb_Joint1 = 3,//WVR_HandJoint::WVR_HandJoint_Thumb_Joint1,
	Thumb_Joint2 = 4,//WVR_HandJoint::WVR_HandJoint_Thumb_Joint2,	// 5
	Thumb_Tip = 5,//WVR_HandJoint::WVR_HandJoint_Thumb_Tip,
	Index_Joint0 = 6,//WVR_HandJoint::WVR_HandJoint_Index_Joint0,
	Index_Joint1 = 7,//WVR_HandJoint::WVR_HandJoint_Index_Joint1,
	Index_Joint2 = 8,//WVR_HandJoint::WVR_HandJoint_Index_Joint2,
	Index_Joint3 = 9,//WVR_HandJoint::WVR_HandJoint_Index_Joint3,	// 10
	Index_Tip = 10,//WVR_HandJoint::WVR_HandJoint_Index_Tip,
	Middle_Joint0 = 11,//WVR_HandJoint::WVR_HandJoint_Middle_Joint0,
	Middle_Joint1 = 12,//WVR_HandJoint::WVR_HandJoint_Middle_Joint1,
	Middle_Joint2 = 13,//WVR_HandJoint::WVR_HandJoint_Middle_Joint2,
	Middle_Joint3 = 14,//WVR_HandJoint::WVR_HandJoint_Middle_Joint3,	// 15
	Middle_Tip = 15,//WVR_HandJoint::WVR_HandJoint_Middle_Tip,
	Ring_Joint0 = 16,//WVR_HandJoint::WVR_HandJoint_Ring_Joint0,
	Ring_Joint1 = 17,//WVR_HandJoint::WVR_HandJoint_Ring_Joint1,
	Ring_Joint2 = 18,//WVR_HandJoint::WVR_HandJoint_Ring_Joint2,
	Ring_Joint3 = 19,//WVR_HandJoint::WVR_HandJoint_Ring_Joint3,		// 20
	Ring_Tip = 20,//WVR_HandJoint::WVR_HandJoint_Ring_Tip,
	Pinky_Joint0 = 21,//WVR_HandJoint::WVR_HandJoint_Pinky_Joint0,
	Pinky_Joint1 = 22,//WVR_HandJoint::WVR_HandJoint_Pinky_Joint1,
	Pinky_Joint2 = 23,//WVR_HandJoint::WVR_HandJoint_Pinky_Joint2,
	Pinky_Joint3 = 24,//WVR_HandJoint::WVR_HandJoint_Pinky_Joint3,	// 25
	Pinky_Tip = 25,//WVR_HandJoint::WVR_HandJoint_Pinky_Tip,
};


UENUM(BlueprintType, Category = "Vive|CustomHandGesture")
enum class EWaveThumbState : uint8
{
	None = 0,
	Close = 1,
	Open = 2,
};

UENUM(BlueprintType, Category = "Vive|CustomHandGesture")
enum class EWaveFingerState : uint8
{
	None = 0,
	Close = 1,
	Relax = 2,
	Open = 4,
};

UENUM(BlueprintType, Category = "Vive|CustomHandGesture")
enum class EWaveJointDistance : uint8
{
	Near = 0,
	Far,
};

USTRUCT(BlueprintType)
struct FThumbState
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vive|CustomHandGesture")
	bool Close = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vive|CustomHandGesture")
	bool Open = false;

public:
	uint64_t State()
	{
		return (Close ? 1 << (uint8)EWaveThumbState::Close : 0)
			| (Open ? 1 << (uint8)EWaveThumbState::Open : 0);
	}
};

USTRUCT(BlueprintType)
struct FFingerState
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vive|CustomHandGesture")
	bool Close = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vive|CustomHandGesture")
	bool Relax = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vive|CustomHandGesture")
	bool Open = false;

public:
	uint64_t State()
	{
		return (Close ? 1 << (uint8)EWaveFingerState::Close : 0)
			| (Relax ? 1 << (uint8)EWaveFingerState::Relax : 0)
			| (Open ? 1 << (uint8)EWaveFingerState::Open : 0);
	}
};

USTRUCT(BlueprintType)
struct FSingleHandNodeDistance
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vive|CustomHandGesture")
	EWaveHandJoint Node1 = EWaveHandJoint::Palm;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViveR|CustomHandGesture")
	EWaveHandJoint Node2 = EWaveHandJoint::Palm;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vive|CustomHandGesture")
	EWaveJointDistance Distance = EWaveJointDistance::Near;
};

USTRUCT(BlueprintType)
struct FSingleHandSetting
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vive|CustomHandGesture")
	FThumbState Thumb;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vive|CustomHandGesture")
	FFingerState Index;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vive|CustomHandGesture")
	FFingerState Middle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vive|CustomHandGesture")
	FFingerState Ring;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vive|CustomHandGesture")
	FFingerState Pinky;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vive|CustomHandGesture")
	TArray< FSingleHandNodeDistance > SingleHandNodeDistances;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vive|CustomHandGesture")
	FRotator RotationCondition = FRotator::ZeroRotator;
};

USTRUCT(BlueprintType)
struct FSingleHandGesture
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vive|CustomHandGesture")
	FString Name = TEXT("Default");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vive|CustomHandGesture")
	FSingleHandSetting Setting;
};

USTRUCT(BlueprintType)
struct FDualHandNodeDistance
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vive|CustomHandGesture")
	EWaveHandJoint LeftNode = EWaveHandJoint::Palm;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vive|CustomHandGesture")
	EWaveHandJoint RightNode = EWaveHandJoint::Palm;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vive|CustomHandGesture")
	EWaveJointDistance Distance = EWaveJointDistance::Near;
};

USTRUCT(BlueprintType)
struct FDualHandSetting
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vive|CustomHandGesture")
	FSingleHandSetting LeftHand;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vive|CustomHandGesture")
	FSingleHandSetting RightHand;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vive|CustomHandGesture")
	TArray< FDualHandNodeDistance > DualHandNodeDistances;
};

USTRUCT(BlueprintType)
struct FDualHandGesture
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vive|CustomHandGesture")
	FString Name = TEXT("Default");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vive|CustomHandGesture")
	FDualHandSetting Setting;
};

namespace wvr {
	namespace hand {
		class WaveHandHelper {
		public:
			static float VectorAngle(FVector a, FVector b)
			{
				float angleCosine = FVector::DotProduct(a, b) / (a.Size() * b.Size());
				float angleRadians = FMath::Acos(angleCosine);
				return FMath::RadiansToDegrees(angleRadians);
			}
			static EWaveThumbState GetThumbState(FVector root, FVector node1, FVector top)
			{
				FVector a = node1 - root, b = top - node1;
				float angle = VectorAngle(a, b);

				return angle < 15 ? EWaveThumbState::Open : EWaveThumbState::Close;
			}
			static EWaveFingerState GetFingerState(FVector root, FVector node1, FVector top)
			{
				FVector a = node1 - root, b = top - node1;
				float angle = VectorAngle(a, b);

				if (angle < 25) return EWaveFingerState::Open;
				if (angle > 75) return EWaveFingerState::Close;

				return EWaveFingerState::Relax;
			}
			static bool MatchThumbState(FThumbState state, EWaveThumbState thumb)
			{
				uint32_t value = 1 << (uint8)thumb;
				if ((state.State() & value) != 0) { return true; }
				return false;
			}
			static bool MatchFingerState(FFingerState state, EWaveFingerState finger)
			{
				uint32_t value = 1 << (uint8)finger;
				if ((state.State() & value) != 0) { return true; }
				return false;
			}
		};
	}
}
