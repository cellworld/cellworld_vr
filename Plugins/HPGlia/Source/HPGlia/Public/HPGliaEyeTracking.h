// (c) Copyright 2019-2020 HP Development Company, L.P.

#pragma once

#include "HPGliaEyeTracking.generated.h"

UENUM(BlueprintType)
enum class EWhichEye : uint8
{
    Unknown,
    Left, 
    Right,
    Both
};

USTRUCT(BlueprintType)
struct FEyeTracking
{
    GENERATED_BODY()
public:

    UPROPERTY(BlueprintReadWrite, Category = "Gaze")
    FDateTime SystemTime;

    UPROPERTY(BlueprintReadWrite, Category = "Gaze")
    FVector CombinedGaze;
    UPROPERTY(BlueprintReadWrite, Category = "Gaze")
    float CombinedGazeConfidence;

    UPROPERTY(BlueprintReadWrite, Category = "Gaze")
    FVector LeftGaze;
    UPROPERTY(BlueprintReadWrite, Category = "Gaze")
    float LeftGazeConfidence;

    UPROPERTY(BlueprintReadWrite, Category = "Gaze")
    FVector RightGaze;
    UPROPERTY(BlueprintReadWrite, Category = "Gaze")
    float RightGazeConfidence;

    UPROPERTY(BlueprintReadWrite, Category = "Pupil Position")
    FVector LeftPupilPosition;
    UPROPERTY(BlueprintReadWrite, Category = "Pupil Position")
    float LeftPupilPositionConfidence;
    UPROPERTY(BlueprintReadWrite, Category = "Pupil Position")
    FVector RightPupilPosition;
    UPROPERTY(BlueprintReadWrite, Category = "Pupil Position")
    float RightPupilPositionConfidence;

    UPROPERTY(BlueprintReadWrite, Category = "Pupil Dilation")
    float LeftPupilDilation;
    UPROPERTY(BlueprintReadWrite, Category = "Pupil Dilation")
    float LeftPupilDilationConfidence;
    UPROPERTY(BlueprintReadWrite, Category = "Pupil Dilation")
    float RightPupilDilation;
    UPROPERTY(BlueprintReadWrite, Category = "Pupil Dilation")
    float RightPupilDilationConfidence;

    UPROPERTY(BlueprintReadWrite, Category = "Eye Openness")
    float LeftEyeOpenness;
    UPROPERTY(BlueprintReadWrite, Category = "Eye Openness")
    float LeftEyeOpennessConfidence;
    UPROPERTY(BlueprintReadWrite, Category = "Eye Openness")
    float RightEyeOpenness;
    UPROPERTY(BlueprintReadWrite, Category = "Eye Openness")
    float RightEyeOpennessConfidence;
};
