// Copyright HTC Corporation. All Rights Reserved.


#include "ViveOpenXRFacialTracking.h"
#include "CoreMinimal.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/ObjectMacros.h"
#include "Engine/Engine.h"
#include "LiveLinkSourceFactory.h"
#include "ILiveLinkClient.h"
#include "Roles/LiveLinkAnimationRole.h"
#include "Roles/LiveLinkAnimationTypes.h"

#define LOCTEXT_NAMESPACE "ViveOpenXRFacialTracking"

void FViveOpenXRFacialTracking::ReceiveClient(ILiveLinkClient* InClient, FGuid InSourceGuid)
{
	LiveLinkClient = InClient;

	LiveLinkSourceGuid = InSourceGuid;
	bNewLiveLinkClient = true;
	LiveLinkEyeTrackingSubjectKey.Source = InSourceGuid;
	LiveLinkEyeTrackingSubjectKey.SubjectName = LiveLinkEyeTrackingSubjectName;
	LiveLinkLipTrackingSubjectKey.Source = InSourceGuid;
	LiveLinkLipTrackingSubjectKey.SubjectName = LiveLinkLipTrackingSubjectName;

	UpdateLiveLinkBlendShapes();
}

bool FViveOpenXRFacialTracking::IsSourceStillValid() const
{
	return LiveLinkClient != nullptr;
}

bool FViveOpenXRFacialTracking::RequestSourceShutdown()
{
	LiveLinkClient = nullptr;
	LiveLinkSourceGuid.Invalidate();

	return true;
}

FText FViveOpenXRFacialTracking::GetSourceMachineName() const
{
	return FText().FromString(FPlatformProcess::ComputerName());
}

FText FViveOpenXRFacialTracking::GetSourceStatus() const
{
	return LOCTEXT("ViveOpenXRFacialTrackingLiveLinkStatus", "Active");
}

FText FViveOpenXRFacialTracking::GetSourceType() const
{
	return LOCTEXT("ViveOpenXRFacialTrackingLiveLinkSourceType", "Vive OpenXR Facial Tracking");
}

void FViveOpenXRFacialTracking::UpdateStaticData()
{
	LiveLinkSkeletonStaticData.InitializeWith(FLiveLinkSkeletonStaticData::StaticStruct(), nullptr);
	FLiveLinkSkeletonStaticData* SkeletonDataPtr = LiveLinkSkeletonStaticData.Cast<FLiveLinkSkeletonStaticData>();
	check(SkeletonDataPtr);

	// Eye
	TMap<EEyeShape, float> EBlendShapes;
	for (int i = 0; i < (int32)EEyeShape::Max; i++)
	{
		if ((EEyeShape)i != EEyeShape::Eye_Frown)
		{
			EEyeShape temp = static_cast<EEyeShape>(i);
			EBlendShapes.Add((EEyeShape)i, 0.0f);
		}
	}

	FLiveLinkSkeletonStaticData EyeStaticData;
	EyeStaticData.PropertyNames.Reset((int32)EEyeShape::Max);
	//Iterate through all valid eye blend shapes to extract names
	const UEnum* EyeEnumPtr = StaticEnum<EEyeShape>();
	for (int32 eyeShape = 0; eyeShape < (int32)EEyeShape::Max; eyeShape++)
	{
		if (EBlendShapes.Contains((EEyeShape)eyeShape))
		{
			EyeStaticData.PropertyNames.Add(ParseEnumName(EyeEnumPtr->GetNameByValue(eyeShape)));
		}
	}
	//Push the associated eye static data
	FLiveLinkStaticDataStruct EyeStaticDataStruct(FLiveLinkBaseStaticData::StaticStruct());
	FLiveLinkBaseStaticData* EyeBaseStaticData = EyeStaticDataStruct.Cast<FLiveLinkBaseStaticData>();
	EyeBaseStaticData->PropertyNames = EyeStaticData.PropertyNames;
	//UE_LOG(LogViveOpenXRFacialTracking, Log, TEXT("Pushing ViveOpenXREye Subject '%s' with %d blend shapes"), *LiveLinkEyeTrackingSubjectName.ToString(), EyeBaseStaticData->PropertyNames.Num());
	LiveLinkClient->PushSubjectStaticData_AnyThread(LiveLinkEyeTrackingSubjectKey, ULiveLinkBasicRole::StaticClass(), MoveTemp(EyeStaticDataStruct));

	// Lip
	TMap<ELipShape, float> LBlendShapes;
	for (int i = 0; i < (int32)ELipShape::Max; i++)
	{
		ELipShape temp = static_cast<ELipShape>(i);
		LBlendShapes.Add(temp, 0.0f);
	}

	FLiveLinkSkeletonStaticData LipStaticData;
	LipStaticData.PropertyNames.Reset((int32)ELipShape::Max);
	//Iterate through all valid lip blend shapes to extract names
	const UEnum* LipEnumPtr = StaticEnum<ELipShape>();
	for (int32 lipShape = 0; lipShape < (int32)ELipShape::Max; lipShape++)
	{
		if (LBlendShapes.Contains((ELipShape)lipShape))
		{
			LipStaticData.PropertyNames.Add(ParseEnumName(LipEnumPtr->GetNameByValue(lipShape)));
		}
	}
	//Push the associated lip static data
	FLiveLinkStaticDataStruct LipStaticDataStruct(FLiveLinkBaseStaticData::StaticStruct());
	FLiveLinkBaseStaticData* LipBaseStaticData = LipStaticDataStruct.Cast<FLiveLinkBaseStaticData>();
	LipBaseStaticData->PropertyNames = LipStaticData.PropertyNames;
	//UE_LOG(LogViveOpenXRFacialTracking, Log, TEXT("Pushing ViveOpenXRLip Subject '%s' with %d blend shapes"), *LiveLinkLipTrackingSubjectName.ToString(), LipBaseStaticData->PropertyNames.Num());
	LiveLinkClient->PushSubjectStaticData_AnyThread(LiveLinkLipTrackingSubjectKey, ULiveLinkBasicRole::StaticClass(), MoveTemp(LipStaticDataStruct));
}

void FViveOpenXRFacialTracking::UpdateLiveLinkBlendShapes()
{
	// This code touches UObjects so needs to be run only in the game thread
	check(IsInGameThread());

	if (!LiveLinkClient) return;

	//If we can't retrieve blend shape enum, nothing we can do
	const UEnum* EyeEnumPtr = StaticEnum<EEyeShape>();
	const UEnum* LipEnumPtr = StaticEnum<ELipShape>();
	if (EyeEnumPtr == nullptr || LipEnumPtr == nullptr)
	{
		return;
	}

	// Per ReceiveClient initialization
	if (bNewLiveLinkClient)
	{
		LiveLinkClient->RemoveSubject_AnyThread(LiveLinkEyeTrackingSubjectKey);
		LiveLinkClient->RemoveSubject_AnyThread(LiveLinkLipTrackingSubjectKey);

		UpdateStaticData();
		bNewLiveLinkClient = false;
	}

	// Eye
	FLiveLinkFrameDataStruct EyeFrameDataStruct(FLiveLinkBaseFrameData::StaticStruct());
	FLiveLinkBaseFrameData* EyeFrameData = EyeFrameDataStruct.Cast<FLiveLinkBaseFrameData>();
	EyeFrameData->WorldTime = FPlatformTime::Seconds();
	EyeFrameData->PropertyValues.Reserve((int32)EEyeShape::Max);
	
	TMap<EEyeShape, float> EyeBlendShapes;
	bool isEyeActive;
	GetEyeExpressions(isEyeActive);
	EyeBlendShapes = eyeShapes;
	if (EyeBlendShapes.Num() > 0)
	{
		// Iterate through all of the blend shapes copying them into the LiveLink data type
		for (int32 eyeShape = 0; eyeShape < (int32)EEyeShape::Max; eyeShape++)
		{
			if (EyeBlendShapes.Contains((EEyeShape)eyeShape))
			{
				const float CurveValue = EyeBlendShapes.FindChecked((EEyeShape)eyeShape);
				EyeFrameData->PropertyValues.Add(CurveValue);
			}
		}
		//UE_LOG(LogViveOpenXRFacialTracking, Log, TEXT("[LiveLinkUpdate] Pushing ViveOpenXREye Subject '%s' with %d blend shapes"), *LiveLinkEyeTrackingSubjectName.ToString(), EyeFrameData->PropertyValues.Num());
	}
	else
	{
		for (int32 eyeShape = 0; eyeShape < (int32)EEyeShape::Max; eyeShape++)
		{
			if ((EEyeShape)eyeShape != EEyeShape::Eye_Frown)
			{
				EyeFrameData->PropertyValues.Add(0.0f);
			}
		}
	}
	// Share the eye data locally with the LiveLink client
	LiveLinkClient->PushSubjectFrameData_AnyThread(LiveLinkEyeTrackingSubjectKey, MoveTemp(EyeFrameDataStruct));

	// Lip
	FLiveLinkFrameDataStruct LipFrameDataStruct(FLiveLinkBaseFrameData::StaticStruct());
	FLiveLinkBaseFrameData* LipFrameData = LipFrameDataStruct.Cast<FLiveLinkBaseFrameData>();
	LipFrameData->WorldTime = FPlatformTime::Seconds();
	LipFrameData->PropertyValues.Reserve((int32)ELipShape::Max);

	TMap<ELipShape, float> LipBlendShapes;
	bool isLipActive;
	GetLipExpressions(isLipActive);
	LipBlendShapes = lipShapes;
	if (LipBlendShapes.Num() > 0)
	{
		// Iterate through all of the blend shapes copying them into the LiveLink data type
		for (int32 lipShape = 0; lipShape < (int32)ELipShape::Max; lipShape++)
		{
			if (LipBlendShapes.Contains((ELipShape)lipShape))
			{
				const float CurveValue = LipBlendShapes.FindChecked((ELipShape)lipShape);
				LipFrameData->PropertyValues.Add(CurveValue);
			}
		}
		//UE_LOG(LogViveOpenXRFacialTracking, Log, TEXT("[LiveLinkUpdate] Pushing ViveOpenXRLip Subject '%s' with %d blend shapes"), *LiveLinkLipTrackingSubjectName.ToString(), LipFrameData->PropertyValues.Num());
	}
	else 
	{
		for (int32 lipShape = 0; lipShape < (int32)ELipShape::Max; lipShape++)
		{
			LipFrameData->PropertyValues.Add(0.0f);
		}
	}
	// Share the lip data locally with the LiveLink client
	LiveLinkClient->PushSubjectFrameData_AnyThread(LiveLinkLipTrackingSubjectKey, MoveTemp(LipFrameDataStruct));

}

#undef LOCTEXT_NAMESPACE


