// Copyright HTC Corporation. All Rights Reserved.

#include "TrackedMeshCollision.h"

namespace ViveOpenXR
{
	TrackedMeshCollision::TrackedMeshCollision(TArray<FVector> vertices, TArray<MRMESH_INDEX_TYPE> indices)
		:m_Vertices(std::move(vertices)),
		m_Indices(std::move(indices))
	{
		if (m_Vertices.Num() > 0)
		{
			m_BoundingBox = FBox(&m_Vertices[0], m_Vertices.Num());
		}
	}

	TrackedMeshCollision::~TrackedMeshCollision()
	{}

	bool TrackedMeshCollision::Collides(const FVector Start, const FVector End, const FTransform MeshToWorld, FVector & OutHitPoint, FVector & OutHitNormal, float& OutHitDistance)
	{
		if (MeshToWorld.GetScale3D().IsNearlyZero())
		{
			return false;
		}

		if (!FMath::LineBoxIntersection(m_BoundingBox.TransformBy(MeshToWorld), Start, End, End - Start))
		{
			return false;
		}

		for (int i = 0; i < m_Indices.Num(); i += 3)
		{
			if ((unsigned int)m_Indices[i] > (unsigned int) m_Vertices.Num() ||
				(unsigned int)m_Indices[i + 1] > (unsigned int)m_Vertices.Num() ||
				(unsigned int)m_Indices[i + 2] > (unsigned int)m_Vertices.Num())
			{
				continue;
			}

			if (FMath::SegmentTriangleIntersection(Start, End, MeshToWorld.TransformPosition(m_Vertices[m_Indices[i]]),
				MeshToWorld.TransformPosition(m_Vertices[m_Indices[i + 1]]),
				MeshToWorld.TransformPosition(m_Vertices[m_Indices[i + 2]]), OutHitPoint, OutHitNormal))
			{
				OutHitDistance = (OutHitPoint - Start).Size();
				return true;
			}
		}
		return false;
	}
}// namespace ViveOpenXR
