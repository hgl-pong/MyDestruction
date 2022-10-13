#ifndef FCHUNK_H
#define FCHUNK_H
#include "PxPhysicsAPI.h"
#include "Fracture.h"
#include <vector>
#include <vector>
#include "FVoronoi3D.h"
#include "Common/Collision.h"
#include "vec.h"
#include "DirectXCollision.h"
#include "FDamage.h"
#include "Common/Geometry.h"
using namespace physx;
using namespace DirectX;

class FActor;
class FChunkCluster;
class FChunk
{
public:
	FChunk() = default;
	FChunk(VoroCellInfo& cellInfo, FActor* actor);
	FChunk(Geometry::MeshData & meshData, FActor* actor);

	~FChunk();
	bool Release();
	bool InitUniquePhysicsActor();
	bool Attach(PxRigidDynamic *actor);
	bool Update();
	bool IsDestructable();
	bool VoronoiFracture(std::vector<FVec3>& sites,FChunkCluster*& chunkCluster);

	bool Intersection(Ray &ray);
	bool InitPhyiscShape();
	bool InitBoundingBox();
	bool Intersection(FDamage* damage);
	FVec3 GetPosition() {
		return m_Center;
	}
	PxRigidDynamic* GetPhysicsActor();
	PxShape *GetPhysicsShape();
	bool CostChunkHealth(float damage);
	BoundingBox GetBoundingBox() {
		return m_BoundingBox;
	}


private:
	friend class FChunkCluster;
	friend class FActor;
	friend class FRenderMesh;
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
	FVec2 m_Damage;
	int m_Id;

	std::vector<FVec3> m_Vertices;
	std::vector<FVec3> m_Vertices2;
	std::vector<FVec3> m_Normals;
	std::vector<FVec3> m_Normals2;
	std::vector<uint32_t> m_Diffuse;
	std::vector<FVec4> m_Tangents;
	std::vector<FVec2> m_UVs;
	std::vector<uint32_t> m_Indices;
	std::vector<uint32_t> m_Faces;

	std::vector<int> m_Neighbors;
	std::vector<float> m_Areas;

};
#endif // FCHUNK_H
