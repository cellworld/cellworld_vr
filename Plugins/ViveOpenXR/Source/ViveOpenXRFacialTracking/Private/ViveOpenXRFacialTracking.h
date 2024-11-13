// Copyright HTC Corporation. All Rights Reserved.

#pragma once

#include "OpenXRCommon.h"
#include "CoreMinimal.h"
#include "ViveFacialExpressionEnums.h"
#include "IViveOpenXRFacialTrackingModule.h"
#include "ILiveLinkSource.h"
#include "Roles/LiveLinkAnimationTypes.h"
#include "Tickable.h"

DECLARE_LOG_CATEGORY_EXTERN(LogViveOpenXRFacialTracking, Log, All);

enum class EError
{
	FAILED = -1,
	WORK = 0
};

struct FFacialTrackingExtesionDispatchTable
{
	PFN_xrGetSystemProperties xrGetSystemProperties;
	PFN_xrCreateFacialTrackerHTC xrCreateFacialTrackerHTC;
	PFN_xrDestroyFacialTrackerHTC xrDestroyFacialTrackerHTC;
	PFN_xrGetFacialExpressionsHTC xrGetFacialExpressionsHTC;
};

class FViveOpenXRFacialTracking : 
	public FTickableGameObject,
	public IOpenXRExtensionPlugin,
	public ILiveLinkSource
{
public:
	FViveOpenXRFacialTracking();
	~FViveOpenXRFacialTracking();

	void Register();
	void Unregister();

	// FTickableGameObject interface
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override { return true; }
	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FViveOpenXRFacialTracking, STATGROUP_Tickables);
	}
	virtual bool IsTickableInEditor() const override { return true; }
	virtual bool IsTickableWhenPaused() const override { return true; }
	// End FTickableGameObject

	/** IOpenXRExtensionPlugin */
	virtual FString GetDisplayName() override
	{
		return FString(TEXT("ViveOpenXRFacialTracking"));
	}

	/** IOpenXRExtensionPlugin implementation */
	virtual const void* OnCreateSession(XrInstance InInstance, XrSystemId InSystem, const void* InNext) override;
	virtual const void* OnBeginSession(XrSession InSession, const void* InNext) override;
	virtual bool GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions) override;
	virtual void UpdateDeviceLocations(XrSession InSession, XrTime DisplayTime, XrSpace TrackingSpace) override;
	virtual void PostGetSystem(XrInstance InInstance, XrSystemId InSystem) override;
	virtual void PostCreateInstance(XrInstance InInstance) override;
	virtual void PostCreateSession(XrSession InSession) override;
	virtual void OnDestroySession(XrSession InSession) override;

	/** ILiveLinkSource interface */
	virtual void ReceiveClient(ILiveLinkClient* InClient, FGuid InSourceGuid) override;
	virtual bool IsSourceStillValid() const override;
	virtual bool RequestSourceShutdown() override;
	virtual FText GetSourceMachineName() const override;
	virtual FText GetSourceStatus() const override;
	virtual FText GetSourceType() const override;
	// End ILiveLinkSource

private:
	void UpdateStaticData();
	void UpdateLiveLinkBlendShapes();

public:
	bool m_bEnableFacialTracking = false;

	bool CreateFacialTracker(XrFacialTrackingTypeHTC facialTrckingType);
	bool DestroyFacialTracker(XrFacialTrackingTypeHTC facialTrckingType);
	bool GetEyeExpressions(bool& isActive);
	bool GetLipExpressions(bool& isActive);

	TMap<EEyeShape, float> eyeShapes;
	TMap<ELipShape, float> lipShapes;

	XrFacialTrackerHTC m_FacialTracker_Eye = XR_NULL_HANDLE;
	XrFacialTrackerHTC m_FacialTracker_Lip = XR_NULL_HANDLE;
private:
	XrInstance m_XrInstance;
	XrSystemId m_XrSystemId;
	XrSession m_Session = XR_NULL_HANDLE;
	FFacialTrackingExtesionDispatchTable FT_ext{};

	const int eyeExpressionCount = ((int)EEyeShape::Max - 1); // (Max - 1) is because SRanipalAvatar have 15 shapes, and OpenXR EyeExpression shape count only 14.
	TMap<XrEyeExpressionHTC, float> eyeWeightings;
	TMap<EEyeShape, XrEyeExpressionHTC> eyeShapeMap;

	const int lipExpressionCount = (int)ELipShape::Max;
	TMap<XrLipExpressionHTC, float> lipWeightings;
	TMap<ELipShape, XrLipExpressionHTC> lipShapeMap;

	EError EyeLastUpdateResult = EError::FAILED;
	EError LipLastUpdateResult = EError::FAILED;
	XrFacialTrackingTypeHTC m_FacialTrackingType;

	double EyeLastUpdateFrame = -1;
	double LipLastUpdateFrame = -1;

	// Map OpenXR eye shapes to Avater eye blendshapes.
	void MapEyeShapes();
	void MapLipShapes();

	bool isEyeSupported = false;
	bool isLipSupported = false;

	// LiveLink Data
	/** The local client to push data updates to */
	ILiveLinkClient* LiveLinkClient = nullptr;
	/** Our identifier in LiveLink */
	FGuid LiveLinkSourceGuid;

	static FLiveLinkSubjectName LiveLinkEyeTrackingSubjectName;
	static FLiveLinkSubjectName LiveLinkLipTrackingSubjectName;
	FLiveLinkSubjectKey LiveLinkEyeTrackingSubjectKey;
	FLiveLinkSubjectKey LiveLinkLipTrackingSubjectKey;
	bool bNewLiveLinkClient = false;
	FLiveLinkStaticDataStruct LiveLinkSkeletonStaticData;

public:
	/** Parses the enum name removing the prefix */
	static FName ParseEnumName(FName EnumName)
	{
		// "EEyeShape::" and "ELipShape::" both length are 11.
		static int32 EnumNameLength = 11;
		FString EnumString = EnumName.ToString();
		return FName(*EnumString.Right(EnumString.Len() - EnumNameLength));
	}

};

class FViveOpenXRFacialTrackingModule : public IViveOpenXRFacialTrackingModule
{
public:
	FViveOpenXRFacialTrackingModule() : FTSource(nullptr)
	{}

	/************************************************************************/
	/* IModuleInterface                                                     */
	/************************************************************************/
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/************************************************************************/
	/* LiveLink			                                                    */
	/************************************************************************/
	virtual TSharedPtr<ILiveLinkSource> CreateLiveLinkSource() override;
	virtual TSharedPtr<ILiveLinkSource> GetLiveLinkSource() override;
	virtual bool IsLiveLinkSourceValid() const override;
	//FViveOpenXRFacialTracking FacialTracking;
	TSharedPtr<FViveOpenXRFacialTracking> FTSource = nullptr;

private:
};

inline FViveOpenXRFacialTrackingModule& GetViveOpenXRFacialTrackingModule()
{
	return FModuleManager::Get().GetModuleChecked<FViveOpenXRFacialTrackingModule>("ViveOpenXRFacialTracking");
}