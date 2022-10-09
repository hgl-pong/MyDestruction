#ifndef FCHUNK_H
#define FCHUNK_H
#include "PxPhysicsAPI.h"
#include<vector>
#include "FVoronoi3D.h"
#include "Common/Collision.h"
#include"vec.h"
#include "DirectXCollision.h"
#include "FDamage.h"
using namespace physx;
using namespace DirectX;
class FChunk {
public:
	FChunk()=default;
	FChunk(VoronoiCellInfo& cellInfo);
	~FChunk();
	bool Release();
	bool CreatePhysicsActor();
	bool Attach(PxRigidDynamic* actor);
	bool Tick();
	bool IsDestructable();
	bool Intersection(Ray& ray);

	bool Intersection(FDamage& damage);
private:
	void _CalculateNormals(std::vector<Edge>& edges, std::vector<FVec3>& normals);
private:
	float m_Volume;
	PxVec3 m_Center;
	PxShape* m_pConvexMesh;
	PxRigidDynamic* m_pPxRigidActor;
	BoundingBox box;

	float m_Life;
	bool m_IsSleeping;
	bool m_IsDestructable;
public:
	PxTransform m_Transform;
	PxVec3 m_Volocity;
	uint32_t start;

	std::vector<FVec3> m_Vertices;
	std::vector<FVec3> m_Normals;
	std::vector<uint32_t> m_Diffuse;
	std::vector<FVec4> m_Tangents;
	std::vector<FVec2>m_UVs;
	std::vector<uint32_t> m_Indices;

	std::vector<uint32_t> m_Neighbors;
};
#endif//FCHUNK_H
