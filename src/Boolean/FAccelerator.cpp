#include "FAccelerator.h"
#include "IntersectUtils.h"
#include <set>
FBoxAccelerator::FBoxAccelerator(FBoundingBox& box, FMeshData& mesh, int level/*=10*/)
	:m_Box(box),
	m_MeshData(mesh)
{
	int size = level * level * level;
	m_Map.resize(size);
	m_Grids.resize(size);
	int32_t currentCell = 0;
	FVec3 incr = (m_Box.m_Max - m_Box.m_Min) * (1.0f / level);
	for (int32_t z = 0; z < level; z++)
	{
		for (int32_t y = 0; y < level; y++)
		{
			for (int32_t x = 0; x < level; x++)
			{
				m_Grids[currentCell].m_Min.X = m_Box.m_Min.X + x * incr.X;
				m_Grids[currentCell].m_Min.Y = m_Box.m_Min.Y + y * incr.Y;
				m_Grids[currentCell].m_Min.Z = m_Box.m_Min.Z + z * incr.Z;

				m_Grids[currentCell].m_Max.X = m_Box.m_Min.X + (x + 1) * incr.X;
				m_Grids[currentCell].m_Max.Y = m_Box.m_Min.Y + (y + 1) * incr.Y;
				m_Grids[currentCell].m_Max.Z = m_Box.m_Min.Z + (z + 1) * incr.Z;

				m_Grids[currentCell].m_Size = incr;
				currentCell++;
			}
		}
	}

	_BuildBox(mesh);
}

FBoxAccelerator::~FBoxAccelerator()
{

}

FMeshData FBoxAccelerator::CollecteTriangles(FBoundingBox& box)
{
	FMeshData meshdata;
	std::unordered_set<FTriangle> triangles;
	std::unordered_set<FVertex> vertices;
	for (uint32_t i = 0; i < m_Grids.size(); i++)
	{
		if (WeakBoundingBoxIntersection(m_Grids[i], box))
		{
			for (auto& tri : m_Map[i]) {
				if (WeakBoundingBoxIntersection(m_MeshData.m_Triangles[tri].box,box)) {
					triangles.emplace(m_MeshData.m_Triangles[tri]);
					vertices.emplace(m_MeshData.m_Triangles[tri].i());
					vertices.emplace(m_MeshData.m_Triangles[tri].j());
					vertices.emplace(m_MeshData.m_Triangles[tri].k());
				}
			}
		}
	}
	for (auto& triangle : triangles) {
		meshdata.m_Triangles.push_back(triangle);
	}
	for (auto& v : vertices) {
		meshdata.m_Vertices.push_back(v);
	}
	return meshdata;
}

void FBoxAccelerator::_BuildBox(FMeshData& mesh)
{
	for (int32_t facet = 0; facet < mesh.m_Triangles.size(); facet++)
	{
		FBoundingBox& bBox=mesh.m_Triangles[facet].box;

		for (uint32_t i = 0; i < m_Grids.size(); i++)
		{
			if (WeakBoundingBoxIntersection(m_Grids[i], bBox))
			{
				m_Map[i].push_back(facet);
			}
		}
	}
}
