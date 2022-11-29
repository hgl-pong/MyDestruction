#ifndef FGEOMETRY_COLLECTION_H
#define FGEOMETRY_COLLECTION_H
#include "FBoundingBox.h"
#include "FTriangulator.h"
#include <unordered_map>
#include <unordered_set>
enum CollectionType
{
	DIFF,
	INTERSECT,
	UNION

};

class TrianglePair {
public:
	TrianglePair(FTriangle& triangleA, FTriangle& triangleB)
		:m_TriangleA(triangleA),
		m_TriangleB(triangleB)
	{
	};
	~TrianglePair() = default;
	bool operator ==(const TrianglePair& pair)const {
		return (m_TriangleA == pair.m_TriangleA && m_TriangleB == pair.m_TriangleB);
	}
	FTriangle& first() const{
		return *const_cast<FTriangle*>(&m_TriangleA);
	}
	FTriangle& second()const {
		return *const_cast<FTriangle*>(&m_TriangleB);
	}
private:
	FTriangle m_TriangleA, m_TriangleB;
};

namespace std {
	template<>
	struct hash<TrianglePair>
	{
		size_t operator ()(const TrianglePair& x) const {
			return hash<FTriangle>()(x.first()) ^ hash<FTriangle>()(x.second());
		}
	};
}

static std::unordered_map<TrianglePair, IntersectEdge> m_IntersectMap;
static std::unordered_map<FTriangle, std::unordered_set<FTriangle>> m_IntersectNeighbors;

static const std::vector<FVec3> g_testAxisList = {
{FLOAT_MAX, 0, 0},
{0, FLOAT_MAX, 0},
{0, 0, FLOAT_MAX},

{FLOAT_MIN, 0, 0},
{0, FLOAT_MAX, 0},
{0, 0, FLOAT_MIN},
};
class FGeometryCollection
{
public:
	FGeometryCollection(FBoundingBox &box, FMeshData &meshdata);
	~FGeometryCollection();

	FMeshData FetchResult(CollectionType type);
	void SetMeshB(FMeshData& meshdata);
	bool CalculateIntersect();
	void Clean(CollectionType type);
	void Triangulate();
private:
	void _GetMeshAInMeshB(std::unordered_set<FTriangle>& meshA, std::unordered_set<FTriangle>& meshB);
private:
	std::unordered_set<FTriangle> m_MeshASet;
	std::unordered_set<FTriangle> m_MeshBSet;

	FMeshData m_MeshA;
	FMeshData m_MeshB;
	FMeshData m_Result;
	
	std::unordered_map<FTriangle, std::unordered_set<FVertex>> m_TrianglesIntersction;
	FBoundingBox m_Box;
};
#endif // FGEOMETRY_COLLECTION_H
