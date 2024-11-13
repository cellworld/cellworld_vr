// Copyright HTC Corporation. All Rights Reserved.


#include "ViveOpenXRPlaneDetectionFunctionLibrary.h"
#include "ViveOpenXRPlaneDetectionModule.h"

#include "OpenXRCore.h"

bool UViveOpenXRPlaneDetectionFunctionLibrary::IsPlaneDetectionSupported()
{
	auto mod = FViveOpenXRPlaneDetection::Instance();
	if (!mod || !mod->IsSupportPlaneDetection()) return false;
	return true;
}

bool UViveOpenXRPlaneDetectionFunctionLibrary::CreatePlaneDetector(int64& planeDetector)
{
	auto mod = FViveOpenXRPlaneDetection::Instance();
	if (!mod) return false;
	XrPlaneDetectorCreateInfoEXT createInfo{};
	createInfo.type = XR_TYPE_PLANE_DETECTOR_CREATE_INFO_EXT;
	createInfo.next = nullptr;
	createInfo.flags = XR_PLANE_DETECTOR_ENABLE_CONTOUR_BIT_EXT;
	XrPlaneDetectorEXT planeDetectorExt{};
	planeDetector = 0;
	if (!mod->CreatePlaneDetector(&createInfo, &planeDetectorExt)) return false;
	planeDetector = (int64)planeDetectorExt;
	return true;
}

bool UViveOpenXRPlaneDetectionFunctionLibrary::DestroyPlaneDetector(int64 planeDetector)
{
	auto mod = FViveOpenXRPlaneDetection::Instance();
	if (!mod) return false;
	return mod->DestroyPlaneDetector((XrPlaneDetectorEXT)planeDetector);
}

bool UViveOpenXRPlaneDetectionFunctionLibrary::BeginPlaneDetection(int64 planeDetector)
{
	auto mod = FViveOpenXRPlaneDetection::Instance();
	auto hmd = FViveOpenXRPlaneDetection::HMD();
	if (!mod || !hmd) return false;

	XrPlaneDetectorBeginInfoEXT beginInfoExt{};
	beginInfoExt.type = XR_TYPE_PLANE_DETECTOR_BEGIN_INFO_EXT;
	beginInfoExt.next = nullptr;
	beginInfoExt.baseSpace = hmd->GetTrackingSpace();
	beginInfoExt.time = hmd->GetDisplayTime();
	beginInfoExt.orientationCount = 4;
	const XrPlaneDetectorOrientationEXT orientations[4] = {
		XR_PLANE_DETECTOR_ORIENTATION_HORIZONTAL_UPWARD_EXT,
		XR_PLANE_DETECTOR_ORIENTATION_HORIZONTAL_DOWNWARD_EXT,
		XR_PLANE_DETECTOR_ORIENTATION_VERTICAL_EXT,
		XR_PLANE_DETECTOR_ORIENTATION_ARBITRARY_EXT,
	};
	beginInfoExt.orientationCount = sizeof(orientations);
	beginInfoExt.orientations = orientations;

	const XrPlaneDetectorSemanticTypeEXT semanticTypes[5] = {
		XR_PLANE_DETECTOR_SEMANTIC_TYPE_UNDEFINED_EXT,
		XR_PLANE_DETECTOR_SEMANTIC_TYPE_CEILING_EXT,
		XR_PLANE_DETECTOR_SEMANTIC_TYPE_FLOOR_EXT,
		XR_PLANE_DETECTOR_SEMANTIC_TYPE_WALL_EXT,
		XR_PLANE_DETECTOR_SEMANTIC_TYPE_PLATFORM_EXT,
	};
	beginInfoExt.semanticTypeCount = sizeof(semanticTypes);
	beginInfoExt.semanticTypes = semanticTypes;

	beginInfoExt.maxPlanes = 100;
	beginInfoExt.minArea = 0.01f;
	beginInfoExt.boundingBoxPose = { { 0, 0, 0, 1 }, { 0, 0, 0 } };
	beginInfoExt.boundingBoxExtent = { 100, 100, 100 };
	return mod->BeginPlaneDetection((XrPlaneDetectorEXT)planeDetector, &beginInfoExt);
}

bool UViveOpenXRPlaneDetectionFunctionLibrary::GetPlaneDetectionState(int64 planeDetector, EViveOpenXRPlaneDetectionState& state)
{
	auto mod = FViveOpenXRPlaneDetection::Instance();
	if (!mod) return false;
	XrPlaneDetectionStateEXT stateExt;
	if (!mod->GetPlaneDetectionState((XrPlaneDetectorEXT)planeDetector, &stateExt)) return false;
	state = (EViveOpenXRPlaneDetectionState)stateExt;
	return true;
}

bool UViveOpenXRPlaneDetectionFunctionLibrary::GetPlaneDetections(int64 planeDetector, TArray<FViveOpenXRPlaneDetectorLocation>& locations)
{
	auto mod = FViveOpenXRPlaneDetection::Instance();
	auto hmd = FViveOpenXRPlaneDetection::HMD();
	locations.Empty();
	if (!mod || !hmd) return false;

	XrPlaneDetectorGetInfoEXT infoExt{};
	infoExt.type = XR_TYPE_PLANE_DETECTOR_GET_INFO_EXT;
	infoExt.next = nullptr;
	infoExt.baseSpace = hmd->GetTrackingSpace();
	infoExt.time = hmd->GetDisplayTime();

	XrPlaneDetectorLocationsEXT locationsExt{};
	locationsExt.type = XR_TYPE_PLANE_DETECTOR_LOCATIONS_EXT;
	locationsExt.next = nullptr;
	locationsExt.planeLocationCapacityInput = 0;
	locationsExt.planeLocationCountOutput = 0;
	locationsExt.planeLocations = nullptr;
	if (!mod->GetPlaneDetections((XrPlaneDetectorEXT)planeDetector, &infoExt, &locationsExt)) return false;
	if (locationsExt.planeLocationCountOutput == 0) return true;
	locationsExt.planeLocations = new XrPlaneDetectorLocationEXT[locationsExt.planeLocationCountOutput];
	locationsExt.planeLocationCapacityInput = locationsExt.planeLocationCountOutput;
	if (!mod->GetPlaneDetections((XrPlaneDetectorEXT)planeDetector, &infoExt, &locationsExt))
	{
		delete[] locationsExt.planeLocations;
		return false;
	}
	locations.SetNum(locationsExt.planeLocationCountOutput);

	auto forwardRot = FQuat(FRotator(0, 180, 0));
	for (int i = 0; i < (int)locationsExt.planeLocationCountOutput; i++)
	{
		auto& locationExt = locationsExt.planeLocations[i];
		locations[i].planeId = locationExt.planeId;
		locations[i].locationFlags = locationExt.locationFlags;
		auto& rot = locationExt.pose.orientation;
		auto& pos = locationExt.pose.position;
		auto& extents = locationExt.extents;
		float w2m = hmd->GetWorldToMetersScale();
		locations[i].transform = FTransform(ToFQuat(rot) * forwardRot, ToFVector(pos, w2m), FVector::OneVector);
		locations[i].size = FVector(extents.height * w2m, extents.width * w2m, 0);
		locations[i].orientation = (EViveOpenXRPlaneDetectorOrientation)locationExt.orientation;
		locations[i].semanticType = (EViveOpenXRPlaneDetectorSemanticType)locationExt.semanticType;
		locations[i].polygonBufferCount = locationExt.polygonBufferCount;
	}
	delete[] locationsExt.planeLocations;
	return true;
}

bool UViveOpenXRPlaneDetectionFunctionLibrary::GetPlanePolygonBufferRaw(int64 planeDetector, int64 planeId, int32 polygonBufferIndex, TArray<FVector>& vertices)
{
	auto mod = FViveOpenXRPlaneDetection::Instance();
	auto hmd = FViveOpenXRPlaneDetection::HMD();
	if (!mod || !hmd) return false;

	XrPlaneDetectorPolygonBufferEXT polygonBuffer{};
	polygonBuffer.type = XR_TYPE_PLANE_DETECTOR_POLYGON_BUFFER_EXT;
	polygonBuffer.next = nullptr;
	polygonBuffer.vertexCapacityInput = 0;
	polygonBuffer.vertexCountOutput = 0;
	polygonBuffer.vertices = nullptr;
	if (!mod->GetPlanePolygonBuffer((XrPlaneDetectorEXT)planeDetector, planeId, polygonBufferIndex, &polygonBuffer)) return false;
	// ViveOpenXR AIO only support quad
	if (polygonBuffer.vertexCountOutput != 4) return true;
	polygonBuffer.vertices = new XrVector2f[polygonBuffer.vertexCountOutput];
	polygonBuffer.vertexCapacityInput = polygonBuffer.vertexCountOutput;
	if (!mod->GetPlanePolygonBuffer((XrPlaneDetectorEXT)planeDetector, planeId, polygonBufferIndex, &polygonBuffer))
	{
		delete[] polygonBuffer.vertices;
		return false;
	}
	vertices.SetNum(polygonBuffer.vertexCountOutput);
	for (int i = 0; i < (int)polygonBuffer.vertexCountOutput; i++)
	{
		vertices[i] = FVector(0, polygonBuffer.vertices[i].x, polygonBuffer.vertices[i].y);
	}
	delete[] polygonBuffer.vertices;
	return true;
}

bool UViveOpenXRPlaneDetectionFunctionLibrary::GetPlanePolygon(int64 planeDetector, int64 planeId, int32 polygonBufferIndex, TArray<FVector>& vertices, TArray<int32>& indices, TArray<FVector>& normals, TArray<FVector2D>& uvs, TArray<FLinearColor>& colors)
{
	auto mod = FViveOpenXRPlaneDetection::Instance();
	auto hmd = FViveOpenXRPlaneDetection::HMD();
	if (!mod || !hmd) return false;

	XrPlaneDetectorPolygonBufferEXT polygonBuffer{};
	polygonBuffer.type = XR_TYPE_PLANE_DETECTOR_POLYGON_BUFFER_EXT;
	polygonBuffer.next = nullptr;
	polygonBuffer.vertexCapacityInput = 0;
	polygonBuffer.vertexCountOutput = 0;
	polygonBuffer.vertices = nullptr;
	if (!mod->GetPlanePolygonBuffer((XrPlaneDetectorEXT)planeDetector, planeId, polygonBufferIndex, &polygonBuffer)) return false;
	// ViveOpenXR AIO only support quad
	if (polygonBuffer.vertexCountOutput != 4) return true;
	polygonBuffer.vertices = new XrVector2f[polygonBuffer.vertexCountOutput];
	polygonBuffer.vertexCapacityInput = polygonBuffer.vertexCountOutput;
	if (!mod->GetPlanePolygonBuffer((XrPlaneDetectorEXT)planeDetector, planeId, polygonBufferIndex, &polygonBuffer))
	{
		delete [] polygonBuffer.vertices;
		return false;
	}

	float w2m = hmd->GetWorldToMetersScale();
	float minX = FLT_MAX;
	float maxX = -FLT_MAX;
	float minY = FLT_MAX;
	float maxY = -FLT_MAX;
	for (int i = 0; i < (int)polygonBuffer.vertexCountOutput; i++)
	{
		minX = FMath::Min(minX, polygonBuffer.vertices[i].x);
		maxX = FMath::Max(maxX, polygonBuffer.vertices[i].x);
		minY = FMath::Min(minY, polygonBuffer.vertices[i].y);
		maxY = FMath::Max(maxY, polygonBuffer.vertices[i].y);
	}
	delete[] polygonBuffer.vertices;

	// Output generated vertices not the orignal vertices
	vertices.SetNum(4);
	// In Unreal, plane actor is horizontal, and In this OpenXR extension, plane is also horizontal.
	// Thus, in Unreal, only the X and Y axis are used, and the Z axis is always 0.
	//                3-----2         3-----2                      3-----2  
	//    +Y         /     /   +X +Z   \     \               +Z   /     /   
	//     |        /     /      \|     \     \              |   /     /    
	//     *-->+X  0-----1        *-->+Y 0-----1        +Y<--*  0-----1     
	//    /        OpenXR                Unreal             /   Unreal      
	//  +Z         Model space           Tracking space   +X    Model space 
	//                                   With Rot 180           With Rot 0  

	// Because the pose will contained the 180 degree rotation, we should make the vertices in Unreal's tracking space.
	vertices[0] = FVector(-minY, minX, 0) * w2m;
	vertices[1] = FVector(-minY, maxX, 0) * w2m;
	vertices[2] = FVector(-maxY, maxX, 0) * w2m;
	vertices[3] = FVector(-maxY, minX, 0) * w2m;
	indices.SetNum(6);
	// In Unreal, the indices are clockwise.
	indices[0] = 0;
	indices[1] = 2;
	indices[2] = 1;
	indices[3] = 0;
	indices[4] = 3;
	indices[5] = 2;
	normals.SetNum(4);
	normals[0] = FVector(0, 0, 1);
	normals[1] = FVector(0, 0, 1);
	normals[2] = FVector(0, 0, 1);
	normals[3] = FVector(0, 0, 1);
	uvs.SetNum(4);
	// In Unreal, the UVs are lower-left origin.
	uvs[0] = FVector2D(0, 0);
	uvs[1] = FVector2D(1, 0);
	uvs[2] = FVector2D(1, 1);
	uvs[3] = FVector2D(0, 1);
	colors.SetNum(4);
	colors[0] = FLinearColor::Gray;
	colors[1] = FLinearColor::Red;
	colors[2] = FLinearColor::White;
	colors[3] = FLinearColor::Green;
	return true;
}

FViveOpenXRPlaneDetectorLocation::FViveOpenXRPlaneDetectorLocation()
	: planeId(0)
	, locationFlags(0)
	, transform(FRotator::ZeroRotator, FVector::ZeroVector, FVector::OneVector)
	, size(FVector::ZeroVector)
	, orientation(EViveOpenXRPlaneDetectorOrientation::Arbitrary)
	, semanticType(EViveOpenXRPlaneDetectorSemanticType::Undefined)
	, polygonBufferCount(0)
{
}
