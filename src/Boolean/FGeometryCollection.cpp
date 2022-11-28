#include "FGeometryCollection.h"
#include "FTriangulator.h"
#include "IntersectUtils.h"
#include <queue>
#include <unordered_map>
FGeometryCollection::FGeometryCollection(FBoundingBox& box,FMeshData& meshdata)
	:m_Box(box),
	m_MeshA(meshdata)
{
}

FGeometryCollection::~FGeometryCollection()
{

}

FMeshData FGeometryCollection::FetchResult(CollectionType type)
{
	bool anyIntersect=CalculateIntersect();
	if(anyIntersect)
		Triangulate();
	else {
		FVertex& point = m_MeshA.m_Vertices[0];
		bool isIn = false;
		for (auto test : g_testAxisList) {
			if (IntersectUtils::IsInMesh(m_MeshASet, point.position, test)) {
				isIn = true;
				break;
			}
		}
		if (isIn)
			return m_MeshA;
		else
			return FMeshData();
	}
	Clean(type);

	m_Result.m_Triangles.clear();
	m_Result.m_Vertices.clear();

	std::unordered_set<FVertex> buffer;
	for (auto& triA : m_MeshASet) {
		m_Result.m_Triangles.push_back(triA);
		buffer.emplace(triA.i());
		buffer.emplace(triA.j());
		buffer.emplace(triA.k());
	}
	for (auto& triB : m_MeshBSet) {
		m_Result.m_Triangles.push_back(triB);
		buffer.emplace(triB.i());
		buffer.emplace(triB.j());
		buffer.emplace(triB.k());
	}
	for (auto& point : buffer) {
		m_Result.m_Vertices.push_back(point);
	}
	return m_Result;
}

void FGeometryCollection::SetMeshB(FMeshData& meshdata)
{
	m_MeshB = meshdata;
}

template <typename T>
void Emplace(std::unordered_set<T>& set, std::vector<T>& array) {
	for (auto& data : array) {
		set.emplace(data);
	}
}

template <typename T>
void Erase(std::unordered_set<T>& set, std::unordered_set<T>& array) {
	for (auto& data : array) {
		set.erase(data);
	}
}



bool FGeometryCollection::CalculateIntersect()
{
	bool anyIntersect = false;
	for (auto  triA:m_MeshA.m_Triangles) {
		for (auto triB:m_MeshB.m_Triangles) {
			if (!WeakBoundingBoxIntersection(triA.box, triB.box)) {
				continue;
			}
			IntersectEdge* edge=nullptr;
			TrianglePair newPair1(triA, triB);
			TrianglePair newPair2(triB, triA);
			auto it = m_IntersectMap.find(newPair1);
			if (it == m_IntersectMap.end())
				IntersectUtils::TrianglesIntersect(triA, triB, edge);
			else {
				edge = new IntersectEdge[2];
				edge[0] = it->second;
				edge[1] = m_IntersectMap.find(newPair2)->second;
			}
				
			if (edge) {
				m_IntersectMap.emplace(newPair1, edge[0]);
				m_IntersectMap.emplace(newPair2, edge[1]);

				auto it = m_IntersectNeighbors.find(triA);
				if (it != m_IntersectNeighbors.end())
					it->second.emplace(triB);
				else {
					std::unordered_set<FTriangle> set;
					set.emplace(triB);
					m_IntersectNeighbors.emplace(triA, set);
				}

				it = m_IntersectNeighbors.find(triB);
				if (it != m_IntersectNeighbors.end())
					it->second.emplace(triA);
				else {
					std::unordered_set<FTriangle> set;
					set.emplace(triA);
					m_IntersectNeighbors.emplace(triB, set);
				}
			}
			else {
			}
		}
	}

	for (auto tri : m_MeshA.m_Triangles) {
		if (m_IntersectNeighbors.find(tri) == m_IntersectNeighbors.end())
			m_MeshASet.emplace(tri);
	}
	for (auto tri : m_MeshB.m_Triangles) {
		if (m_IntersectNeighbors.find(tri) == m_IntersectNeighbors.end())
			m_MeshBSet.emplace(tri);
	}
	anyIntersect = (m_MeshASet.size()!=m_MeshA.m_Triangles.size());
	return anyIntersect;
}

void FGeometryCollection::_GetMeshAInMeshB(std::unordered_set<FTriangle>& meshA, std::unordered_set<FTriangle>& meshB) {
	std::unordered_set<FVec3> pointsOutOfMeshB;
	std::unordered_set<FVec3> pointsInMeshB;
	std::unordered_set<FTriangle> triangleToDelete;

	for (auto& triangleA : meshA) {
		auto center = (triangleA.i().position + triangleA.j().position + triangleA.k().position) / 3;
		std::vector<FVec3>points = {
			triangleA.i().position,
			triangleA.j().position,
			triangleA.k().position,
			center
		};
		for (auto& point : points) {
			auto it = pointsOutOfMeshB.find(point);
			if (it != pointsOutOfMeshB.end()) {
				triangleToDelete.emplace(triangleA);
				break;
			}
			else {
				bool isIn = false;
				for (auto test : g_testAxisList) {
					if (pointsInMeshB.find(point) != pointsInMeshB.end() || IntersectUtils::IsInMesh(meshB, point, test)) {
						isIn = true;
						break;
					}
				}
				if (!isIn) {
					pointsOutOfMeshB.emplace(point);
					triangleToDelete.emplace(triangleA);
					break;
				}
				else
					pointsInMeshB.emplace(point);
			}
		}
	}

	for (auto triangle : triangleToDelete)
		meshA.erase(triangle);
	triangleToDelete.clear();
}

void FGeometryCollection::Clean(CollectionType type)
{
	std::unordered_set<FTriangle>meshA = m_MeshASet;
	std::unordered_set<FTriangle>meshB = m_MeshBSet;
	_GetMeshAInMeshB(meshA, m_MeshBSet);
	_GetMeshAInMeshB(meshB, m_MeshASet);

	
	switch (type)
	{
	case DIFF: 
	{
		Erase(m_MeshASet, meshA);
		m_MeshBSet = meshB;
		break;
	}
	case INTERSECT:
	{
		m_MeshASet = meshA;
		m_MeshBSet = meshB;
		break;
	}
	case UNION:
	{
		Erase(m_MeshASet, meshA);
		Erase(m_MeshBSet, meshB);
		break;
	}
	default:
		break;
	}	
}


void FGeometryCollection::Triangulate()
{
	std::unordered_set<FTriangle> meshA;
	Emplace(meshA, m_MeshA.m_Triangles);
	for (auto triA : m_IntersectNeighbors) {
		std::unordered_map<FIndex, FVertex> pointsMap;
		std::vector<FVertex> points;
		std::unordered_map<FVertex, FIndex> indexMap;
		std::unordered_map<FIndex, std::unordered_set<FIndex>> edges;
		std::vector<FTriangle> triangles;
		for (auto triB : triA.second) {
			TrianglePair pair(*const_cast<FTriangle*>(&triA.first), triB);
			auto it = m_IntersectMap.find(pair);
			if (it == m_IntersectMap.end())
				continue;
			FVertex& keyA=it->second.first;
			FVertex& keyB=it->second.second;
			auto it1 = indexMap.find(keyA);
			auto it2 = indexMap.find(keyB);

			if (it1 == indexMap.end()) {
				indexMap.emplace(keyA, indexMap.size()+3);
				pointsMap.emplace(pointsMap.size()+3, it->second.first);
				//points.push_back(it->second.first);
			}

			if (it2 == indexMap.end()) {
				indexMap.emplace(keyB, indexMap.size()+3);
				pointsMap.emplace(pointsMap.size()+3, it->second.second);
				//points.push_back(it->second.second);
			}

			auto it3 = edges.find(indexMap[keyA]);
			if (it3 != edges.end())
				it3->second.emplace(indexMap[keyB]);
			else {
				std::unordered_set<FIndex> set;
				set.emplace(indexMap[keyB]);
				edges.emplace(indexMap[keyA], set);
			}

			auto it4 = edges.find(indexMap[keyB]);
			if (it4 != edges.end())
				it4->second.emplace(indexMap[keyA]);
			else {
				std::unordered_set<FIndex> set;
				set.emplace(indexMap[keyA]);
				edges.emplace(indexMap[keyB], set);
			}
		}
		points.resize(pointsMap.size());
		for (int i = 0; i < pointsMap.size(); i++)
			points[i]=pointsMap[i+3];
		FTriangulator::Triangulating(*const_cast<FTriangle*>(&triA.first), points, edges, triangles);
		if (meshA.find(triA.first) != meshA.end())
			Emplace(m_MeshASet, triangles);
		else
			Emplace(m_MeshBSet, triangles);
	}
}
//
//bool FGeometryCollection::_BuildPolygonsFromEdges(const std::unordered_map<int, std::unordered_set<int>>& edges,
//	std::vector<std::vector<int>>& polygons)
//{
//	std::unordered_set<size_t> visited;
//	for (const auto& edge : edges) {
//		const auto& startEndpoint = edge.first;
//		if (visited.find(startEndpoint) != visited.end())
//			continue;
//		std::queue<int> q;
//		q.push(startEndpoint);
//		std::vector<int> polyline;
//		while (!q.empty()) {
//			int loop = q.front();
//			visited.insert(loop);
//			polyline.push_back(loop);
//			q.pop();
//			auto neighborIt = edges.find(loop);
//			if (neighborIt == edges.end())
//				break;
//			for (const auto& it : neighborIt->second) {
//				if (visited.find(it) == visited.end()) {
//					q.push(it);
//					break;
//				}
//			}
//		}
//		if (polyline.size() <= 2) {
//			printf("buildPolygonsFromEdges failed, too short");
//			return false;
//		}
//
//		auto neighborOfLast = edges.find(polyline.back());
//		if (neighborOfLast->second.find(startEndpoint) == neighborOfLast->second.end()) {
//			printf("buildPolygonsFromEdges failed, could not form a ring");
//			return false;
//		}
//
//		polygons.push_back(polyline);
//	}
//
//	return true;
//}