#ifndef FCHUNK_H
#define FCHUNK_H
#include "PxPhysicsAPI.h"
#include <vector>
#include "FVoronoi3D.h"
#include "Common/Collision.h"
#include "vec.h"
#include "DirectXCollision.h"
#include "FDamage.h"
using namespace physx;
using namespace DirectX;
class FActor;
class FChunk
{
public:
	FChunk() = default;
	FChunk(VoroCellInfo &cellInfo, FActor*actor);
	~FChunk();
	bool Release();
	bool InitUniquePhysicsActor();
	bool Attach(PxRigidDynamic *actor);
	bool Tick();
	bool IsDestructable();
	bool Intersection(Ray &ray);
	bool InitPhyiscShape();
	bool InitBoundingBox();
	bool Intersection(FDamage &damage);
	PxRigidDynamic *GetPhysicsActor();

private:
	void _CalculateNormals(std::vector<Edge> &edges, std::vector<FVec3> &normals);

private:
	friend class FChunkCluster;
	float m_Volume;
	FActor* m_pActor;
	FVec3 m_Center;
	PxShape *m_pConvexMeshShape;
	PxRigidDynamic *m_pRigidActor;
	BoundingBox m_BoundingBox;

	float m_Life;
	bool m_IsSleeping;
	bool m_IsDestructable;
	float m_ChunkHealth;
public:
	PxTransform m_Transform;
	PxVec3 m_Volocity;
	uint32_t m_VBStartPos;
	uint32_t m_Offset;

	std::vector<FVec3> m_Vertices;
	std::vector<FVec3> m_Normals;
	std::vector<uint32_t> m_Diffuse;
	std::vector<FVec4> m_Tangents;
	std::vector<FVec2> m_UVs;
	std::vector<uint32_t> m_Indices;

	std::vector<uint32_t> m_Neighbors;
	std::vector<float> m_Areas;

};
#endif // FCHUNK_H
