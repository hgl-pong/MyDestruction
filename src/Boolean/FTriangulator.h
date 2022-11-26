#ifndef FTRIANGULAR_H
#define FTRIANGULAR_H
#include <unordered_map>
#include <unordered_set>
#include "FBoundingBox.h"
typedef std::pair<FVertex, FVertex> IntersectEdge;

class FTriangulator
{
public:
	static bool Triangulating(FTriangle& triangle, std::vector<FVertex>& points,std::unordered_map<FIndex, std::unordered_set<FIndex>>& neighborMapFrom3,std::vector<FTriangle>&triangles);
private:
	FTriangulator(std::vector<FVertex>& points);
	FTriangulator(FTriangle& triangles);
public:
	void SetEdges(std::vector<FVertex>& points,
		std::unordered_map<FIndex, std::unordered_set<FIndex>>* neighborMapFrom3);
	bool ReTriangulate();
	const std::vector<std::vector<FIndex>>& GetPolygons() const;
	const std::vector<std::vector<FIndex>>& GetTriangles() const;
private:
	FVec3 m_projectAxis;
	FVec3 m_projectOrigin;
	FVec3 m_projectNormal;
	std::vector<FVec2> m_points;
	const std::unordered_map<FIndex, std::unordered_set<FIndex>>* m_neighborMapFrom3 = nullptr;
	std::vector<std::vector<FIndex>> m_polylines;
	std::vector<std::vector<FIndex>> m_innerPolygons;
	std::vector<std::vector<FIndex>> m_polygons;
	std::unordered_map<FIndex, std::unordered_set<FIndex>> m_innerParentsMap;
	std::unordered_map<FIndex, std::unordered_set<FIndex>> m_innerChildrenMap;
	std::unordered_map<FIndex, std::vector<FIndex>> m_polygonHoles;
	std::vector<std::vector<FIndex>> m_triangles;

	void LookupPolylinesFromNeighborMap(const std::unordered_map<FIndex, std::unordered_set<FIndex>>& neighborMap);
	int AttachPointToTriangleEdge( FVec2& point);
	bool BuildPolygons();
	void BuildPolygonHierarchy();
	void Triangulate();
};

inline std::vector<FVec2> Project(std::vector<FVertex>polygon, FVec3& normal, FVec3& axis, FVec3& origin) {
	std::vector<FVec2>result;

	FVec3 perpendicularAxis = normal.Cross(axis);
	for (auto it:polygon) {
		FVec3 direction = it.position - origin;
		result.push_back(FVec2(direction.Dot(axis), direction.Dot(perpendicularAxis)));
	}

	return result;
}

inline std::vector<FVec2> Project(FVertex*polygon, FVec3& normal, FVec3& axis, FVec3& origin) {
	std::vector<FVec2>result;

	FVec3 perpendicularAxis = normal.Cross(axis);
	for (int i=0; i < 3;i++) {
		FVec3 direction = polygon[i].position - origin;
		result.push_back(FVec2(direction.Dot(axis), direction.Dot(perpendicularAxis)));
	}

	return result;
}


#endif // FTRIANGULAR_H



