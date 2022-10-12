#ifndef FRENDER_MESH_H
#define FRENDER_MESH_H
#include <vector>
#include <vec.h>
#include "Common\Material.h"
#include "Common\MeshData.h"
class FActor;
class FRenderMesh
{
public:
	FRenderMesh(FActor *actor);
	~FRenderMesh();
	void UpdateMeshData();
	void CreateRenderData();
	void Release();
public:
	FActor *m_pActor;
	std::vector<FVec3> m_Vertices;
	std::vector<FVec3> m_Normals;
	std::vector<FVec2> m_UVs;
	std::vector<uint32_t> m_Indices;

	Material *m_pMaterial;
	Graphics::MeshData *m_pMeshData;
};
#endif // FRENDER_MESH_H
