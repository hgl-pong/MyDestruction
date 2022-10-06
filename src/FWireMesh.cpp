#include "FWireMesh.h"
#include "Utils.h"
FWireMesh::FWireMesh(BoundingBox& box)
	:m_pVoronoi3D(nullptr),
	m_pCellInfo(nullptr),
	m_Box(box)
{
	FVoronoi3D::BBox bbox;
	bbox.Max = { box.Center.x + box.Extents.x,box.Center.y + box.Extents.y ,box.Center.z + box.Extents.z };
	bbox.Min = { box.Center.x - box.Extents.x,box.Center.y - box.Extents.y ,box.Center.z - box.Extents.z };
	m_pVoronoi3D = new FVoronoi3D(bbox);

	m_pVoronoi3D->AddSites(100);
	m_pVoronoi3D->ComputeCellEdges();
	m_pCellInfo = m_pVoronoi3D->GetAllCells();
}

FWireMesh::~FWireMesh()
{
	Release();
}

bool FWireMesh::LoadMeshData()
{
	Graphics::MeshData* newMesh;
	vertices.clear();
	colors.clear();
	indices.clear();
	uint32_t vCount = 0;

	FASSERT(m_pCellInfo);
	newMesh = new Graphics::MeshData();

	for (int i = 0; i < m_pVoronoi3D->Size(); i++) {
		for (auto pos : m_pCellInfo[i].Vertices) {
			vertices.push_back({ (float)pos.X,(float)pos.Y,(float)pos.Z });
			colors.push_back({ 1.0f,1.0f,0.5f,1.0f });
		}
		for (auto edge : m_pCellInfo[i].Edges) {
			indices.push_back(edge.s+vCount);
			indices.push_back(edge.e+vCount);
		}
		vCount += m_pCellInfo[i].Vertices.size();
	}

	return true;
Exit0:
	return false;
}

bool FWireMesh::Release()
{
	FDELETE(m_pVoronoi3D);
	FDELETE(m_pCellInfo);
	vertices.clear();
	colors.clear();
	indices.clear();
	return true;
}

bool FWireMesh::VoronoiFracture(std::vector<FVec3>& sites)
{
	
	return false;
}
