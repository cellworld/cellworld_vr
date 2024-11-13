// Copyright HTC Corporation. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ViveOpenXRPlaneDetectionFunctionLibrary.generated.h"

UENUM(BlueprintType, Category = "ViveOpenXR|PlaneDetection", DisplayName = "Orientation")
enum class EViveOpenXRPlaneDetectorOrientation : uint8
{
	HorizontalUpward = 0,
	HorizontalDownward = 1,
	Vertical = 2,
	Arbitrary = 3,
};

UENUM(BlueprintType, Category = "ViveOpenXR|PlaneDetection", DisplayName = "SemanticType")
enum class EViveOpenXRPlaneDetectorSemanticType : uint8
{
	// The runtime was unable to classify this plane.
	Undefined = 0,

	// The plane is a ceiling.
	Ceiling = 1,

	// The plane is a floor.
	Floor = 2,

	// The plane is a wall.
	Wall = 3,

	// The plane is a platform.
	Platform = 4,
};


UENUM(BlueprintType, Category = "ViveOpenXR|PlaneDetection", DisplayName = "State")
enum class EViveOpenXRPlaneDetectionState : uint8
{

	// The plane detector is not actively looking for planes
	None = 0,

	// This plane detector is currently looking for planes but not yet ready with results
	Pending = 1,

	// This plane detector has finished and results may now be retrieved
	Done = 2,

	// An error occurred. The query may be tried again
	Error = 3,

	// An error occurred. The query must not be tried again
	Fatal = 4,
};

UENUM(BlueprintType, Category = "ViveOpenXR|PlaneDetection", DisplayName = "CapabilityBit")
enum class EViveOpenXRPlaneDetectionBit : uint8
{
	None = 0,
	PlaneDetection = 1,
	PlaneHoles = 2,
	SemanticCeiling = 3,
	SemanticFloor = 4,
	SemanticWall = 5,
	SemanticPlatform = 6,
	SemanticOrientation = 7,
};

//UENUM(BlueprintType, Category = "ViveOpenXR|PlaneDetection", DisplayName = "LocationFlags")
//enum class EViveOpenXRSpaceLocationFlags : uint8
//{
//	OrientationValidBit = 0x01,
//	PositionValidBit = 0x02,
//	OrientationTrackedBit = 0x04,
//	PositionTrackedBit = 0x08,
//};

// The informations of detected plane
USTRUCT(BlueprintType, Category = "ViveOpenXR|PlaneDetection", DisplayName = "Location")
struct FViveOpenXRPlaneDetectorLocation {
	GENERATED_USTRUCT_BODY()

	FViveOpenXRPlaneDetectorLocation();
	
	// planeId is a uint64_t unique identifier of the plane. This id is used by GetPlanePolygonBuffer.
	UPROPERTY(BlueprintReadOnly, Category = "ViveOpenXR|PlaneDetection")
	int64 planeId;

	// Used to check if the pose is valid or lost tracking.
	UPROPERTY(BlueprintReadOnly, Category = "ViveOpenXR|PlaneDetection")
	int64 locationFlags;

	// In the extension, the deliver values includes only "Translation" and "Rotation" components, with "Scale" being unused and set to 1 by default. The transfrom is an pose defining the position and orientation of the origin of a plane within the reference frame of the corresponding base space.
	UPROPERTY(BlueprintReadOnly, Category = "ViveOpenXR|PlaneDetection")
	FTransform transform;

	// Size is the extent of the plane along the y-axis (width) and x-axis (height) centered on the pose.
	UPROPERTY(BlueprintReadOnly, Category = "ViveOpenXR|PlaneDetection")
	FVector size;

	// The detected orientation of the plane.
	UPROPERTY(BlueprintReadOnly, Category = "ViveOpenXR|PlaneDetection")
	EViveOpenXRPlaneDetectorOrientation orientation;

	// The detected semantic type of the plane.
	UPROPERTY(BlueprintReadOnly, Category = "ViveOpenXR|PlaneDetection")
	EViveOpenXRPlaneDetectorSemanticType semanticType;

	// The number of polygon buffers for this plane.
	UPROPERTY(BlueprintReadOnly, Category = "ViveOpenXR|PlaneDetection")
	int polygonBufferCount;
};

/**
 * Blueprint function library for ViveOpenXR Plane Detection extension.
 */
UCLASS(ClassGroup = OpenXR)
class VIVEOPENXRPLANEDETECTION_API UViveOpenXRPlaneDetectionFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	* Check if Anchor is supported by requesting SystemProperties.
	*
	* @return (Boolean) true if plane detection is supported.
	*/
	UFUNCTION(BlueprintCallable, Category = "ViveOpenXR|PlaneDetection")
	static bool IsPlaneDetectionSupported();

	/**
	* Create a plane detector. This function implies applying only one flag in the extension.
	*
	* @param planeDetector		(out) Created plane detector.
	* @return (Boolean) false if API error.
	*/
	UFUNCTION(/*BlueprintCallable,*/ Category = "ViveOpenXR|PlaneDetection")
	static bool CreatePlaneDetector(int64& planeDetector);

	/**
	* Destroy the created plane detector.
	*
	* @param planeDetector		(in) The detector created by CreatePlaneDetector.
	* @return (Boolean) false if API error.
	*/
	UFUNCTION(/*BlueprintCallable,*/ Category = "ViveOpenXR|PlaneDetection")
	static bool DestroyPlaneDetector(int64 planeDetector);

	/**
	* Let the planeDetector begin the detection. Detecting planes in a scene is an asynchronous operation.
	* GetPlaneDetectionState can be used to determine if the query has finished. Once it has finished the 
	* results may be retrieved via xrGetPlaneDetectionsEXT. If a detection has already been started on a plane
	* detector handle, calling BeginPlaneDetection again on the same handle will cancel the operation in
	* progress and start a new detection with the new filter parameters.
	* This function implies applying all filters in the extension.
	* If you need to apply custom filters, you should directly use OpenXR API in C++.
	*
	* @param planeDetector		(in) The detector created by CreatePlaneDetector.
	* @return (Boolean) false if API error.
	*/
	UFUNCTION(/*BlueprintCallable,*/ Category = "ViveOpenXR|PlaneDetection")
	static bool BeginPlaneDetection(int64 planeDetector);

	/**
	* State is Pending if the plane detection has not yet finished, Done if the plane detection has finished,
	* and None if no plane detection was previously started.
	*
	* @param planeDetector		(in) The detector created by CreatePlaneDetector.
	* @param state				(out) The state.
	* @return (Boolean) false if API error.
	*/
	UFUNCTION(/*BlueprintCallable,*/ Category = "ViveOpenXR|PlaneDetection")
	static bool GetPlaneDetectionState(int64 planeDetector, EViveOpenXRPlaneDetectionState& state);

	/**
	* Return an array of currently detected planes's informations. The pose of a plane is in the current tracking space.
	* 
	* @param planeDetector		(in) The detector created by CreatePlaneDetector.
	* @param locations			(out) The array of detected planes' information.
	* @return (Boolean) false if API error.
	*/
	UFUNCTION(/*BlueprintCallable,*/ Category = "ViveOpenXR|PlaneDetection")
	static bool GetPlaneDetections(int64 planeDetector, TArray<FViveOpenXRPlaneDetectorLocation>& locations);


	/**
	* Retrieves the planes polygon buffer for the given planeId and polygonBufferIndex.
	* Outside contour polygon vertices must be ordered in counter clockwise order. Vertices 
	* of holes must be ordered in clockwise order. The right-hand rule is used to determine
	* the direction of the normal of this plane. The polygon contour data is relative to the
	* pose of the plane and coplanar with it.
	* 
	* This function only retrieves polygons, which means that it needs to be converted to a 
	* regular mesh to be rendered.
	*
	* @param planeDetector		(in) The detector created by CreatePlaneDetector.
	* @param planeId			(in) is in the PlaneDetectorLocation which is from GetPlaneDetections.
	* @param polygonBufferIndex	(in) The index of the polygon contour buffer to retrieve. This must be a number from 0 to polygonBufferCount - 1. Index 0 retrieves the outside contour, larger indexes retrieve holes in the plane.
	* @param vertices			(out) The array of vertices of the plane polygon.
	* @return (Boolean) false if API error.
	*/
	UFUNCTION(/*BlueprintCallable,*/ Category = "ViveOpenXR|PlaneDetection")
	static bool GetPlanePolygonBufferRaw(int64 planeDetector, int64 planeId, int32 polygonBufferIndex, TArray<FVector>& vertices);

	/**
	* return data for creating a mesh.  In this function, the plane's mesh is created by the size 
	* and tranform of the plane, not from the raw data.
	*
	* @param planeDetector		(in) The detector created by CreatePlaneDetector.
	* @param planeId			(in) is in the PlaneDetectorLocation which is from GetPlaneDetections.
	* @param polygonBufferIndex	(in) The index of the polygon contour buffer to retrieve. This must be a number from 0 to 
	*/
	UFUNCTION(/*BlueprintCallable,*/ Category = "ViveOpenXR|PlaneDetection")
	static bool GetPlanePolygon(int64 planeDetector, int64 planeId, int32 polygonBufferIndex, TArray<FVector>& vertices, TArray<int32>& indices, TArray<FVector>& normals, TArray<FVector2D>& uvs, TArray<FLinearColor>& colors);
};
