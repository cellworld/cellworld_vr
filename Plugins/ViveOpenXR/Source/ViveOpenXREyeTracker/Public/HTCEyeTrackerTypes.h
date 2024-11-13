// Copyright HTC Corporation. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Math/UnrealMathSSE.h"
#include "Math/Vector.h"
#include "UObject/ObjectMacros.h"

#include "HTCEyeTrackerTypes.generated.h"

USTRUCT(BlueprintType)
struct FXrGazeDataHTC
{
	GENERATED_BODY()

public:

	FXrGazeDataHTC()
		: rightEyeGazeValid(false)
		, leftEyeGazeValid(false)
	{}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HTC Eye Gaze Data")
	bool rightEyeGazeValid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HTC Eye Gaze Data")
	bool leftEyeGazeValid;
};

USTRUCT(BlueprintType)
struct FXrPupilDataHTC
{
	GENERATED_BODY()

public:

	FXrPupilDataHTC()
		: rightEyeDiameterValid(false)
		, leftEyeDiameterValid(false)
		, rightEyePositionValid(false)
		, leftEyePositionValid(false)
		, rightEyePupilDiameter(0.f)
		, leftEyePupilDiameter(0.f)
		, rightEyePupilPosition(ForceInitToZero)
		, leftEyePupilPosition(ForceInitToZero)
	{}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HTC Eye Pupil Data")
	bool rightEyeDiameterValid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HTC Eye Pupil Data")
	bool leftEyeDiameterValid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HTC Eye Pupil Data")
	bool rightEyePositionValid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HTC Eye Pupil Data")
	bool leftEyePositionValid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HTC Eye Pupil Data")
	float rightEyePupilDiameter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HTC Eye Pupil Data")
	float leftEyePupilDiameter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HTC Eye Pupil Data")
	FVector2f rightEyePupilPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HTC Eye Pupil Data")
	FVector2f leftEyePupilPosition;

};


USTRUCT(BlueprintType)
struct FXrEyeGeometricDataHTC
{
	GENERATED_BODY()

public:

	FXrEyeGeometricDataHTC()
		: rightEyeIsValid(false)
		, leftEyeIsValid(false)
		, rightEyeOpenness(0.f)
		, leftEyeOpenness(0.f)
		, rightEyeWide(0.f)
		, leftEyeWide(0.f)
		, rightEyeSqueeze(0.f)
		, leftEyeSqueeze(0.f)
	{}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HTC Eye Geometric Data")
	bool rightEyeIsValid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HTC Eye Geometric Data")
	bool leftEyeIsValid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HTC Eye Geometric Data")
	float rightEyeOpenness;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HTC Eye Geometric Data")
	float leftEyeOpenness;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HTC Eye Geometric Data")
	float rightEyeWide;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HTC Eye Geometric Data")
	float leftEyeWide;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HTC Eye Geometric Data")
	float rightEyeSqueeze;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HTC Eye Geometric Data")
	float leftEyeSqueeze;

};