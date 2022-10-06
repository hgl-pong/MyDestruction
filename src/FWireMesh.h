#ifndef FWIRE_MESH_H
#define FWIRE_MESH_H
#include<DirectXCollision.h>
#include "FVoronoi3D.h"
#include "Common/MeshData.h"
using namespace DirectX;
class FVoronoi3D;

class FWireMesh
{
public:
	FWireMesh(BoundingBox& box);
	~FWireMesh();

	bool LoadMeshData();
	bool Release();
	bool VoronoiFracture(std::vector<FVec3>& sites);
private:
	VoronoiCellInfo* m_pCellInfo;
	FVoronoi3D* m_pVoronoi3D;
	BoundingBox m_Box;
public:
	std::vector<XMFLOAT3>vertices;
	std::vector<XMFLOAT4>colors;
	std::vector<uint32_t>indices;
};

#endif // FWIRE_MESH_H
