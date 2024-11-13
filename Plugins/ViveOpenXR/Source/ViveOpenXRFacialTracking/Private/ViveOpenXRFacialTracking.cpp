// Copyright HTC Corporation. All Rights Reserved.


#include "ViveOpenXRFacialTracking.h"
#include "Misc/App.h"
#include "Engine/World.h"
#include "ILiveLinkClient.h"

#include "Roles/LiveLinkAnimationRole.h"
#include "Roles/LiveLinkAnimationTypes.h"
#include "LiveLinkSourceFactory.h"

#define LOCTEXT_NAMESPACE "ViveOpenXRFacialTracking"
DEFINE_LOG_CATEGORY(LogViveOpenXRFacialTracking);


void FViveOpenXRFacialTrackingModule::StartupModule()
{
	IViveOpenXRFacialTrackingModule::StartupModule();
	CreateLiveLinkSource();
}

void FViveOpenXRFacialTrackingModule::ShutdownModule()
{
	IViveOpenXRFacialTrackingModule::ShutdownModule();
}

TSharedPtr<ILiveLinkSource> FViveOpenXRFacialTrackingModule::CreateLiveLinkSource()
{
	if (!FTSource.IsValid())
	{
		TSharedPtr<FViveOpenXRFacialTracking> Source(new FViveOpenXRFacialTracking());
		FTSource = Source;
		return FTSource;
	}
	return FTSource;
}

TSharedPtr<ILiveLinkSource> FViveOpenXRFacialTrackingModule::GetLiveLinkSource()
{
	if (!FTSource.IsValid())
	{
		CreateLiveLinkSource();
	}
	return FTSource;
}

bool FViveOpenXRFacialTrackingModule::IsLiveLinkSourceValid() const
{
	return FTSource.IsValid();
}


IMPLEMENT_MODULE(FViveOpenXRFacialTrackingModule, ViveOpenXRFacialTracking)

FLiveLinkSubjectName FViveOpenXRFacialTracking::LiveLinkEyeTrackingSubjectName(TEXT("Eye"));
FLiveLinkSubjectName FViveOpenXRFacialTracking::LiveLinkLipTrackingSubjectName(TEXT("Lip"));


FViveOpenXRFacialTracking::FViveOpenXRFacialTracking()
{
	m_FacialTrackingType = XrFacialTrackingTypeHTC::XR_FACIAL_TRACKING_TYPE_EYE_DEFAULT_HTC;
	// Register modular feature manually
	IModularFeatures::Get().RegisterModularFeature(IOpenXRExtensionPlugin::GetModularFeatureName(), static_cast<IOpenXRExtensionPlugin*>(this));
	Register();
}

FViveOpenXRFacialTracking::~FViveOpenXRFacialTracking()
{
	// Unregister modular feature manually
	IModularFeatures::Get().UnregisterModularFeature(IOpenXRExtensionPlugin::GetModularFeatureName(), static_cast<IOpenXRExtensionPlugin*>(this));
	Unregister();
}

void FViveOpenXRFacialTracking::MapEyeShapes()
{
	eyeShapeMap.Add(EEyeShape::Eye_Left_Blink, XrEyeExpressionHTC::XR_EYE_EXPRESSION_LEFT_BLINK_HTC);
	eyeShapeMap.Add(EEyeShape::Eye_Left_Wide, XrEyeExpressionHTC::XR_EYE_EXPRESSION_LEFT_WIDE_HTC);
	eyeShapeMap.Add(EEyeShape::Eye_Right_Blink, XrEyeExpressionHTC::XR_EYE_EXPRESSION_RIGHT_BLINK_HTC);
	eyeShapeMap.Add(EEyeShape::Eye_Right_Wide, XrEyeExpressionHTC::XR_EYE_EXPRESSION_RIGHT_WIDE_HTC);
	eyeShapeMap.Add(EEyeShape::Eye_Left_Squeeze, XrEyeExpressionHTC::XR_EYE_EXPRESSION_LEFT_SQUEEZE_HTC);
	eyeShapeMap.Add(EEyeShape::Eye_Right_Squeeze, XrEyeExpressionHTC::XR_EYE_EXPRESSION_RIGHT_SQUEEZE_HTC);
	eyeShapeMap.Add(EEyeShape::Eye_Left_Down, XrEyeExpressionHTC::XR_EYE_EXPRESSION_LEFT_DOWN_HTC);
	eyeShapeMap.Add(EEyeShape::Eye_Right_Down, XrEyeExpressionHTC::XR_EYE_EXPRESSION_RIGHT_DOWN_HTC);
	eyeShapeMap.Add(EEyeShape::Eye_Left_Left, XrEyeExpressionHTC::XR_EYE_EXPRESSION_LEFT_OUT_HTC);
	eyeShapeMap.Add(EEyeShape::Eye_Right_Left, XrEyeExpressionHTC::XR_EYE_EXPRESSION_RIGHT_IN_HTC);
	eyeShapeMap.Add(EEyeShape::Eye_Left_Right, XrEyeExpressionHTC::XR_EYE_EXPRESSION_LEFT_IN_HTC);
	eyeShapeMap.Add(EEyeShape::Eye_Right_Right, XrEyeExpressionHTC::XR_EYE_EXPRESSION_RIGHT_OUT_HTC);
	eyeShapeMap.Add(EEyeShape::Eye_Left_Up, XrEyeExpressionHTC::XR_EYE_EXPRESSION_LEFT_UP_HTC);
	eyeShapeMap.Add(EEyeShape::Eye_Right_Up, XrEyeExpressionHTC::XR_EYE_EXPRESSION_RIGHT_UP_HTC);
}

void FViveOpenXRFacialTracking::MapLipShapes()
{
	lipShapeMap.Add(ELipShape::Jaw_Right, XrLipExpressionHTC::XR_LIP_EXPRESSION_JAW_RIGHT_HTC);
	lipShapeMap.Add(ELipShape::Jaw_Left, XrLipExpressionHTC::XR_LIP_EXPRESSION_JAW_LEFT_HTC);
	lipShapeMap.Add(ELipShape::Jaw_Forward, XrLipExpressionHTC::XR_LIP_EXPRESSION_JAW_FORWARD_HTC);
	lipShapeMap.Add(ELipShape::Jaw_Open, XrLipExpressionHTC::XR_LIP_EXPRESSION_JAW_OPEN_HTC);
	lipShapeMap.Add(ELipShape::Mouth_Ape_Shape, XrLipExpressionHTC::XR_LIP_EXPRESSION_MOUTH_APE_SHAPE_HTC);
	lipShapeMap.Add(ELipShape::Mouth_Upper_Right, XrLipExpressionHTC::XR_LIP_EXPRESSION_MOUTH_UPPER_RIGHT_HTC);
	lipShapeMap.Add(ELipShape::Mouth_Upper_Left, XrLipExpressionHTC::XR_LIP_EXPRESSION_MOUTH_UPPER_LEFT_HTC);
	lipShapeMap.Add(ELipShape::Mouth_Lower_Right, XrLipExpressionHTC::XR_LIP_EXPRESSION_MOUTH_LOWER_RIGHT_HTC);
	lipShapeMap.Add(ELipShape::Mouth_Lower_Left, XrLipExpressionHTC::XR_LIP_EXPRESSION_MOUTH_LOWER_LEFT_HTC);
	lipShapeMap.Add(ELipShape::Mouth_Upper_Overturn, XrLipExpressionHTC::XR_LIP_EXPRESSION_MOUTH_UPPER_OVERTURN_HTC);
	lipShapeMap.Add(ELipShape::Mouth_Lower_Overturn, XrLipExpressionHTC::XR_LIP_EXPRESSION_MOUTH_LOWER_OVERTURN_HTC);
	lipShapeMap.Add(ELipShape::Mouth_Pout, XrLipExpressionHTC::XR_LIP_EXPRESSION_MOUTH_POUT_HTC);
	lipShapeMap.Add(ELipShape::Mouth_Smile_Right, XrLipExpressionHTC::XR_LIP_EXPRESSION_MOUTH_SMILE_RIGHT_HTC);
	lipShapeMap.Add(ELipShape::Mouth_Smile_Left, XrLipExpressionHTC::XR_LIP_EXPRESSION_MOUTH_SMILE_LEFT_HTC);
	lipShapeMap.Add(ELipShape::Mouth_Sad_Right, XrLipExpressionHTC::XR_LIP_EXPRESSION_MOUTH_SAD_RIGHT_HTC);
	lipShapeMap.Add(ELipShape::Mouth_Sad_Left, XrLipExpressionHTC::XR_LIP_EXPRESSION_MOUTH_SAD_LEFT_HTC);
	lipShapeMap.Add(ELipShape::Cheek_Puff_Right, XrLipExpressionHTC::XR_LIP_EXPRESSION_CHEEK_PUFF_RIGHT_HTC);
	lipShapeMap.Add(ELipShape::Cheek_Puff_Left, XrLipExpressionHTC::XR_LIP_EXPRESSION_CHEEK_PUFF_LEFT_HTC);
	lipShapeMap.Add(ELipShape::Cheek_Suck, XrLipExpressionHTC::XR_LIP_EXPRESSION_CHEEK_SUCK_HTC);
	lipShapeMap.Add(ELipShape::Mouth_Upper_UpRight, XrLipExpressionHTC::XR_LIP_EXPRESSION_MOUTH_UPPER_UPRIGHT_HTC);
	lipShapeMap.Add(ELipShape::Mouth_Upper_UpLeft, XrLipExpressionHTC::XR_LIP_EXPRESSION_MOUTH_UPPER_UPLEFT_HTC);
	lipShapeMap.Add(ELipShape::Mouth_Lower_DownRight, XrLipExpressionHTC::XR_LIP_EXPRESSION_MOUTH_LOWER_DOWNRIGHT_HTC);
	lipShapeMap.Add(ELipShape::Mouth_Lower_DownLeft, XrLipExpressionHTC::XR_LIP_EXPRESSION_MOUTH_LOWER_DOWNLEFT_HTC);
	lipShapeMap.Add(ELipShape::Mouth_Upper_Inside, XrLipExpressionHTC::XR_LIP_EXPRESSION_MOUTH_UPPER_INSIDE_HTC);
	lipShapeMap.Add(ELipShape::Mouth_Lower_Inside, XrLipExpressionHTC::XR_LIP_EXPRESSION_MOUTH_LOWER_INSIDE_HTC);
	lipShapeMap.Add(ELipShape::Mouth_Lower_Overlay, XrLipExpressionHTC::XR_LIP_EXPRESSION_MOUTH_LOWER_OVERLAY_HTC);
	lipShapeMap.Add(ELipShape::Tongue_LongStep1, XrLipExpressionHTC::XR_LIP_EXPRESSION_TONGUE_LONGSTEP1_HTC);
	lipShapeMap.Add(ELipShape::Tongue_Left, XrLipExpressionHTC::XR_LIP_EXPRESSION_TONGUE_LEFT_HTC);
	lipShapeMap.Add(ELipShape::Tongue_Right, XrLipExpressionHTC::XR_LIP_EXPRESSION_TONGUE_RIGHT_HTC);
	lipShapeMap.Add(ELipShape::Tongue_Up, XrLipExpressionHTC::XR_LIP_EXPRESSION_TONGUE_UP_HTC);
	lipShapeMap.Add(ELipShape::Tongue_Down, XrLipExpressionHTC::XR_LIP_EXPRESSION_TONGUE_DOWN_HTC);
	lipShapeMap.Add(ELipShape::Tongue_Roll, XrLipExpressionHTC::XR_LIP_EXPRESSION_TONGUE_ROLL_HTC);
	lipShapeMap.Add(ELipShape::Tongue_LongStep2, XrLipExpressionHTC::XR_LIP_EXPRESSION_TONGUE_LONGSTEP2_HTC);
	lipShapeMap.Add(ELipShape::Tongue_UpRight_Morph, XrLipExpressionHTC::XR_LIP_EXPRESSION_TONGUE_UPRIGHT_MORPH_HTC);
	lipShapeMap.Add(ELipShape::Tongue_UpLeft_Morph, XrLipExpressionHTC::XR_LIP_EXPRESSION_TONGUE_UPLEFT_MORPH_HTC);
	lipShapeMap.Add(ELipShape::Tongue_DownRight_Morph, XrLipExpressionHTC::XR_LIP_EXPRESSION_TONGUE_DOWNRIGHT_MORPH_HTC);
	lipShapeMap.Add(ELipShape::Tongue_DownLeft_Morph, XrLipExpressionHTC::XR_LIP_EXPRESSION_TONGUE_DOWNLEFT_MORPH_HTC);
}

void FViveOpenXRFacialTracking::Register()
{
	UE_LOG(LogViveOpenXRFacialTracking, Log, TEXT("Entry FT Register."));

	RegisterOpenXRExtensionModularFeature();

	check(GConfig && GConfig->IsReadyForUse());
	FString modeName;
	if (GConfig->GetString(TEXT("/Script/ViveOpenXRRuntimeSettings.ViveOpenXRRuntimeSettings"), TEXT("bEnableFacialTracking"), modeName, GEngineIni))
	{
		if (modeName.Equals("False"))
		{
			m_bEnableFacialTracking = false;
		}
		else if (modeName.Equals("True"))
		{
			m_bEnableFacialTracking = true;

		}
	}

	if (m_bEnableFacialTracking)
	{
		UE_LOG(LogViveOpenXRFacialTracking, Log, TEXT("Enable FacialTracking."));
	}
	else
	{
		UE_LOG(LogViveOpenXRFacialTracking, Log, TEXT("Disable FacialTracking."));
	}
}

void FViveOpenXRFacialTracking::Unregister()
{
	UE_LOG(LogViveOpenXRFacialTracking, Log, TEXT("Entry FacialTracking Unregister."));

	UnregisterOpenXRExtensionModularFeature();
}

void FViveOpenXRFacialTracking::Tick(float DeltaTime)
{
	if (!m_bEnableFacialTracking) return;
	UpdateLiveLinkBlendShapes();
}

const void* FViveOpenXRFacialTracking::OnCreateSession(XrInstance InInstance, XrSystemId InSystem, const void* InNext)
{
	if (!m_bEnableFacialTracking) return InNext;

	UE_LOG(LogViveOpenXRFacialTracking, Log, TEXT("Entry FacialTracking OnCreateSession."));
	XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrCreateFacialTrackerHTC", (PFN_xrVoidFunction*)&FT_ext.xrCreateFacialTrackerHTC));
	XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrDestroyFacialTrackerHTC", (PFN_xrVoidFunction*)&FT_ext.xrDestroyFacialTrackerHTC));
	XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrGetFacialExpressionsHTC", (PFN_xrVoidFunction*)&FT_ext.xrGetFacialExpressionsHTC));

  return InNext;
}

const void* FViveOpenXRFacialTracking::OnBeginSession(XrSession InSession, const void* InNext)
{
	if (!m_bEnableFacialTracking) return InNext;

	UE_LOG(LogViveOpenXRFacialTracking, Log, TEXT("Entry FacialTracking OnBeginSession."));

	m_Session = InSession;
	static FName SystemName(TEXT("OpenXR"));

	MapEyeShapes();
	MapLipShapes();

	for (int i = 0; i < eyeExpressionCount; i++) eyeWeightings.Add((XrEyeExpressionHTC)(i), 0.0f);
	for (int i = 0; i < lipExpressionCount; i++) lipWeightings.Add((XrLipExpressionHTC)(i), 0.0f);

	if (isEyeSupported) 
	{
		CreateFacialTracker(XR_FACIAL_TRACKING_TYPE_EYE_DEFAULT_HTC);
	}
	if (isLipSupported)
	{
		CreateFacialTracker(XR_FACIAL_TRACKING_TYPE_LIP_DEFAULT_HTC);
	}
  return InNext;
}

bool FViveOpenXRFacialTracking::GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions)
{
	if (m_bEnableFacialTracking)
	{
		UE_LOG(LogViveOpenXRFacialTracking, Log, TEXT("Add FacialTracking Extension Name: %s."), ANSI_TO_TCHAR(XR_HTC_FACIAL_TRACKING_EXTENSION_NAME));
		OutExtensions.Add(XR_HTC_FACIAL_TRACKING_EXTENSION_NAME);
	}
	return true;
}

void FViveOpenXRFacialTracking::UpdateDeviceLocations(XrSession InSession, XrTime DisplayTime, XrSpace TrackingSpace)
{
}

void FViveOpenXRFacialTracking::PostGetSystem(XrInstance InInstance, XrSystemId InSystem)
{
	if (!m_bEnableFacialTracking)	return;

#if !WITH_EDITOR
	UE_LOG(LogViveOpenXRFacialTracking, Log, TEXT("[PostGetSystem] FacialTracking Entry."));
#endif
	m_XrInstance = InInstance;
	m_XrSystemId = InSystem;
	XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrGetSystemProperties", (PFN_xrVoidFunction*)&FT_ext.xrGetSystemProperties));

	XrSystemFacialTrackingPropertiesHTC expressionProperties;
	expressionProperties.type = XrStructureType::XR_TYPE_SYSTEM_FACIAL_TRACKING_PROPERTIES_HTC;
	expressionProperties.next = NULL;
	XrSystemProperties systemProperties;
	systemProperties.type = XrStructureType::XR_TYPE_SYSTEM_PROPERTIES;
	systemProperties.next = &expressionProperties;

	XrResult result = FT_ext.xrGetSystemProperties(m_XrInstance, m_XrSystemId, &systemProperties);
	XR_ENSURE(result);

	if (expressionProperties.supportEyeFacialTracking == 0)
	{
		isEyeSupported = false;
#if !WITH_EDITOR
		UE_LOG(LogViveOpenXRFacialTracking, Log, TEXT("Initial eyetracking failed , the device may not support EyeExpression."));
#endif
	}
	else isEyeSupported = true;

	if (expressionProperties.supportLipFacialTracking == 0)
	{
		isLipSupported = false;
#if !WITH_EDITOR
		UE_LOG(LogViveOpenXRFacialTracking, Log, TEXT("Initial liptracking failed , the device may not support LipExpression."));
#endif
	}
	else isLipSupported = true;
}

void FViveOpenXRFacialTracking::PostCreateInstance(XrInstance InInstance)
{}

void FViveOpenXRFacialTracking::PostCreateSession(XrSession InSession)
{
	m_Session = InSession;
}

void FViveOpenXRFacialTracking::OnDestroySession(XrSession InSession)
{
	if (isEyeSupported)
	{
		DestroyFacialTracker(XR_FACIAL_TRACKING_TYPE_EYE_DEFAULT_HTC);
	}
	if (isLipSupported)
	{
		DestroyFacialTracker(XR_FACIAL_TRACKING_TYPE_LIP_DEFAULT_HTC);
	}
}

bool FViveOpenXRFacialTracking::CreateFacialTracker(XrFacialTrackingTypeHTC facialTrckingType)
{
	if (!m_bEnableFacialTracking)
	{
		UE_LOG(LogViveOpenXRFacialTracking, Warning, TEXT("[FacialTracking] Please enable Facial Tracking in Project Setting > Plugins > ViveOpenXR."));
		return false;
	}

	if (m_Session == XR_NULL_HANDLE)
	{
		UE_LOG(LogViveOpenXRFacialTracking, Error, TEXT("[CreateFacialTracker] m_Session == XR_NULL_HANDLE."));
		return false;
	}

	// Check tracker is already created.
	if (facialTrckingType == XrFacialTrackingTypeHTC::XR_FACIAL_TRACKING_TYPE_EYE_DEFAULT_HTC && m_FacialTracker_Eye != XR_NULL_HANDLE)
	{
		UE_LOG(LogViveOpenXRFacialTracking, Error, TEXT("[CreateFacialTracker] m_FacialTracker_Eye %lu is already created."), m_FacialTracker_Eye);
		return true;
	}
	if (facialTrckingType == XrFacialTrackingTypeHTC::XR_FACIAL_TRACKING_TYPE_LIP_DEFAULT_HTC && m_FacialTracker_Lip != XR_NULL_HANDLE)
	{
		UE_LOG(LogViveOpenXRFacialTracking, Error, TEXT("[CreateFacialTracker] m_FacialTracker_Lip %lu is already created."), m_FacialTracker_Lip);
		return true;
	}

	UE_LOG(LogViveOpenXRFacialTracking, Log, TEXT("[CreateFacialTracker] Entry CreateFacialTracker."));
	XrFacialTrackerCreateInfoHTC facialTrackerCreateInfo;
	facialTrackerCreateInfo.type = XrStructureType::XR_TYPE_FACIAL_TRACKER_CREATE_INFO_HTC;
	facialTrackerCreateInfo.next = NULL;
	facialTrackerCreateInfo.facialTrackingType = facialTrckingType;
	XrResult result = XrResult::XR_ERROR_HANDLE_INVALID;
	if (facialTrckingType == XrFacialTrackingTypeHTC::XR_FACIAL_TRACKING_TYPE_EYE_DEFAULT_HTC && isEyeSupported)
	{
		UE_LOG(LogViveOpenXRFacialTracking, Log, TEXT("[CreateFacialTracker] FacialTrackingType = Eye."));
		result = FT_ext.xrCreateFacialTrackerHTC(m_Session, &facialTrackerCreateInfo, &m_FacialTracker_Eye);
		if (m_FacialTracker_Eye == XR_NULL_HANDLE)
		{
			UE_LOG(LogViveOpenXRFacialTracking, Error, TEXT("[CreateFacialTracker] After xrCreateFacialTrackerHTC(Eye), FacialTracker_Eye still XR_NULL_HANDLE."));
		}

		XR_ENSURE(result);
	}
	else if (facialTrckingType == XrFacialTrackingTypeHTC::XR_FACIAL_TRACKING_TYPE_LIP_DEFAULT_HTC && isLipSupported)
	{
		UE_LOG(LogViveOpenXRFacialTracking, Log, TEXT("[CreateFacialTracker] FacialTrackingType = Lip."));
		result = FT_ext.xrCreateFacialTrackerHTC(m_Session, &facialTrackerCreateInfo, &m_FacialTracker_Lip);
		if (m_FacialTracker_Lip == XR_NULL_HANDLE)
		{
			UE_LOG(LogViveOpenXRFacialTracking, Error, TEXT("[CreateFacialTracker] After xrCreateFacialTrackerHTC(Lip), FacialTracker_Lip still XR_NULL_HANDLE."));
		}

		XR_ENSURE(result);
	}

	if (result == XrResult::XR_SUCCESS || result == XrResult::XR_SESSION_LOSS_PENDING)
	{
		return true;
	}
	else
	{
		UE_LOG(LogViveOpenXRFacialTracking, Error, TEXT("[CreateFacialTracker] Initial Facial Tracker failed: %s"), OpenXRResultToString(result));
		return false;
	}
}

bool FViveOpenXRFacialTracking::DestroyFacialTracker(XrFacialTrackingTypeHTC facialTrckingType)
{
	if (!m_bEnableFacialTracking)	return false;

	UE_LOG(LogViveOpenXRFacialTracking, Log, TEXT("[DestroyFacialTracker] Entry DestroyFacialTracker."));

	XrResult result = XrResult::XR_ERROR_HANDLE_INVALID;
	if(facialTrckingType == XrFacialTrackingTypeHTC::XR_FACIAL_TRACKING_TYPE_EYE_DEFAULT_HTC){
		if (m_FacialTracker_Eye == XR_NULL_HANDLE)
		{
			UE_LOG(LogViveOpenXRFacialTracking, Error, TEXT("[DestroyFacialTracker] m_FacialTracker_Eye is XR_NULL_HANDLE."));
			return true;
		}
		else
		{
			result = FT_ext.xrDestroyFacialTrackerHTC(m_FacialTracker_Eye);
			m_FacialTracker_Eye = XR_NULL_HANDLE;
		}
	}
	if(facialTrckingType == XrFacialTrackingTypeHTC::XR_FACIAL_TRACKING_TYPE_LIP_DEFAULT_HTC)
	{
		if (m_FacialTracker_Lip == XR_NULL_HANDLE)
		{
			UE_LOG(LogViveOpenXRFacialTracking, Error, TEXT("[DestroyFacialTracker] m_FacialTracker_Lip is XR_NULL_HANDLE."));
			return true;
		}
		else
		{
			result = FT_ext.xrDestroyFacialTrackerHTC(m_FacialTracker_Lip);
			m_FacialTracker_Lip = XR_NULL_HANDLE;
		}
	}
	XR_ENSURE(result);
	if (result != XR_SUCCESS)
	{
		UE_LOG(LogViveOpenXRFacialTracking, Error, TEXT("[DestroyFacialTracker] DestroyFacialTracker failed: %s"), OpenXRResultToString(result));
		return false;
	}
	return true;
}

bool FViveOpenXRFacialTracking::GetEyeExpressions(bool& isActive)
{
	if (!m_bEnableFacialTracking)	return false;

	TArray<float> blendShapes;

	if (m_FacialTracker_Eye == XR_NULL_HANDLE)
	{
		//UE_LOG(LogViveOpenXRFacialTracking, Error, TEXT("[GetEyeFacialExpressions] Eye FacialTracker is XR_NULL_HANDLE."));
		return false;
	}

	double currentTime = FApp::GetDeltaTime();
	if (FApp::GetDeltaTime() == EyeLastUpdateFrame)
	{
		return EyeLastUpdateResult == EError::WORK;
	}
	else EyeLastUpdateFrame = FApp::GetDeltaTime();

	XrFacialExpressionsHTC facialExpressions;
	facialExpressions.type = XR_TYPE_FACIAL_EXPRESSIONS_HTC;
	facialExpressions.next = NULL;
	facialExpressions.expressionCount = XR_FACIAL_EXPRESSION_EYE_COUNT_HTC;
	TArray<float> expressionWeightings_;
	expressionWeightings_.SetNum(facialExpressions.expressionCount);
	facialExpressions.expressionWeightings = expressionWeightings_.GetData();

	XrResult result = FT_ext.xrGetFacialExpressionsHTC(m_FacialTracker_Eye, &facialExpressions);
	XR_ENSURE(result);

	if (result != XR_SUCCESS)
	{
		UE_LOG(LogViveOpenXRFacialTracking, Error, TEXT("[GetEyeFacialExpressions] Eye xrGetFacialExpressionsHTC: %s"), OpenXRResultToString(result));
		EyeLastUpdateResult = EError::FAILED;
		return false;
	}

	if (facialExpressions.isActive == 1)
	{
		isActive = true;
		blendShapes.SetNum(facialExpressions.expressionCount);
		FMemory::Memcpy(blendShapes.GetData(), facialExpressions.expressionWeightings, facialExpressions.expressionCount * sizeof(float));
		EyeLastUpdateResult = EError::WORK;
	}
	else if (facialExpressions.isActive == 0)
	{
		isActive = false;
		EyeLastUpdateResult = EError::FAILED;
	}

	if (isActive)
	{
		if (blendShapes.Num() > 0)
		{
			for (int i = 0; i < eyeExpressionCount; i++)
			{
				eyeWeightings[static_cast<XrEyeExpressionHTC>(i)] = blendShapes[i];
			}

			for (auto elem : eyeWeightings)
			{
				eyeShapes.Add(*eyeShapeMap.FindKey(elem.Key), elem.Value);
			}
		}
	}

	return EyeLastUpdateResult == EError::WORK;
}

bool FViveOpenXRFacialTracking::GetLipExpressions(bool& isActive)
{
	if (!m_bEnableFacialTracking)	return false;

	TArray<float> blendShapes;

	if (m_FacialTracker_Lip == XR_NULL_HANDLE)
	{
		//UE_LOG(LogViveOpenXRFacialTracking, Error, TEXT("[GetLipFacialExpressions] Lip FacialTracker is XR_NULL_HANDLE."));
		return false;
	}

	double currentTime = FApp::GetDeltaTime();
	if (FApp::GetDeltaTime() == LipLastUpdateFrame)
	{
		return LipLastUpdateResult == EError::WORK;
	}
	else LipLastUpdateFrame = FApp::GetDeltaTime();

	XrFacialExpressionsHTC facialExpressions;
	facialExpressions.type = XR_TYPE_FACIAL_EXPRESSIONS_HTC;
	facialExpressions.next = NULL;
	facialExpressions.expressionCount = XR_FACIAL_EXPRESSION_LIP_COUNT_HTC;
	TArray<float> expressionWeightings_;
	expressionWeightings_.SetNum(facialExpressions.expressionCount);
	facialExpressions.expressionWeightings = expressionWeightings_.GetData();
	XrResult result = FT_ext.xrGetFacialExpressionsHTC(m_FacialTracker_Lip, &facialExpressions);
	XR_ENSURE(result);

	if (result != XR_SUCCESS)
	{
		UE_LOG(LogViveOpenXRFacialTracking, Error, TEXT("[GetLipFacialExpressions] Lip xrGetFacialExpressionsHTC: %s"), OpenXRResultToString(result));
		LipLastUpdateResult = EError::FAILED;
		return false;
	}

	if (facialExpressions.isActive == 1)
	{
		isActive = true;
		blendShapes.SetNum(facialExpressions.expressionCount);
		FMemory::Memcpy(blendShapes.GetData(), facialExpressions.expressionWeightings, facialExpressions.expressionCount * sizeof(float));
		LipLastUpdateResult = EError::WORK;
	}
	else if (facialExpressions.isActive == 0)
	{
		isActive = false;
		LipLastUpdateResult = EError::FAILED;
	}

	if (isActive)
	{
		if (blendShapes.Num() > 0)
		{
			for (int i = 0; i < lipExpressionCount; i++)
			{
				lipWeightings[static_cast<XrLipExpressionHTC>(i)] = blendShapes[i];
			}

			for (auto elem : lipWeightings)
			{
				lipShapes.Add(*lipShapeMap.FindKey(elem.Key), elem.Value);
			}
		}
	}
	return LipLastUpdateResult == EError::WORK;
}

#undef LOCTEXT_NAMESPACE


