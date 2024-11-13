// Copyright HTC Corporation. All Rights Reserved.

#pragma once
#include "OpenXRCore.h"
#include "IOpenXRARModule.h"
#include "IOpenXRARModule.h"
#include "IOpenXRARTrackedGeometryHolder.h"

namespace ViveOpenXR
{
	class TrackedMeshCollision
	{
	public:
		TrackedMeshCollision(TArray<FVector> vertices, TArray<MRMESH_INDEX_TYPE> indices);
		~TrackedMeshCollision();

		bool Collides(const FVector Start, const FVector End, const FTransform MeshToWorld, FVector& OutHitPoint, FVector& OutHitNormal, float& OutHitDistance);
	
	private:
		TArray<FVector> m_Vertices;
		TArray<MRMESH_INDEX_TYPE> m_Indices;
		FBox m_BoundingBox;
	};

}// namespace ViveOpenXR
