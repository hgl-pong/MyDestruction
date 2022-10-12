#ifndef FWIRE_MESH_H
#define FWIRE_MESH_H
#include<DirectXCollision.h>
#include "FVoronoi3D.h"
#include "Common/MeshData.h"
using namespace DirectX;
class FVoronoi3D;

class FScene;
class FWireMesh
{
public:
	FWireMesh(BoundingBox& box);
	~FWireMesh();

	bool LoadMeshData();
	bool Release();
	bool VoronoiFracture(std::vector<FVec3>& sites);

	bool AddToScene(FScene* scene);
	bool CreaePhysicsActor(Transform& trans);
private:


	BoundingBox m_Box;
public:
	std::vector<XMFLOAT3>vertices;
	std::vector<XMFLOAT4>colors;
	std::vector<uint32_t>indices;
	FVoronoi3D* m_pVoronoi3D;
	VoroCellInfo* m_pCellInfo;

};

#endif // FWIRE_MESH_H
