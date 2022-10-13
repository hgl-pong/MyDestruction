#ifndef FACTOR_H
#define FACTOR_H
#include "PxPhysicsAPI.h"
#include "Common/MeshData.h"
#include <string>
#include "Common/Collision.h"
#include "vec.h"
#include <algorithm>
#include"FPhysics.h"
using namespace physx;

class FRenderMesh;
class FWireMesh;
class FScene;
class FChunk;
class FChunkCluster;
class FDamage;
class FChunkManager;
class FActor
{
public:
	FActor();
	~FActor();

	bool Init(char * name);
	bool Release();
	bool ReInit();

	bool Update();
	
	bool OnEnterScene(FScene* scene);
	bool OnLeaveScene(FScene* scene);

	bool AddPhysicsActorToScene(PxRigidDynamic* actor);
	
	bool Intersection(FDamage*damage,FScene* scene);

	bool RemoveChunk(FChunk* chunk);
	bool RemoveChunkCluser(FChunkCluster* chunkCluster);
	bool SetRenderWireFrame(bool render);

private:
	friend class FChunk;
	friend class FChunkCluster;
	friend class FRenderMesh;
	friend class FChunkManager;
	FRenderMesh* m_pRenderMesh;
	FWireMesh* m_pWireMesh;
	bool m_RebuildRenderMesh;
	Transform m_Transform;

	std::set<FScene*> m_Scenes;
	Graphics::MeshData* m_pMeshData;
	Graphics::MeshData* m_pVoroMeshData;

	FChunkManager* m_pChunkManager;
	FMaterial m_Material;

	int m_HitTime = 0;
	char* m_Name;
};

static bool Hit(const Ray& ray,BoundingBox&box,FVec3& intersectionPoint)
{
	FVec3  mnear, mfar;
	FVec3 center (box.Center.x, box.Center.y, box.Center.z);
	FVec3 extents(box.Extents.x, box.Extents.y,box.Extents.z);
	mnear = FVec3(center.X - extents.X, center.Y - extents.Y, center.Z - extents.Z);
	mfar = FVec3(center.X + extents.X, center.Y + extents.Y, center.Z + extents.Z);

	FVec3 vertices[] =
	{
		{mnear},
		{FVec3(mnear.X, mfar.Y, mnear.Z)},
		{FVec3(mfar.X, mfar.Y, mnear.Z)},
		{FVec3(mfar.X, mnear.Y, mnear.Z)},

		{FVec3(mnear.X, mnear.Y, mfar.Z)},
		{FVec3(mnear.X, mfar.Y, mfar.Z)},
		{mfar},
		{FVec3(mfar.X, mnear.Y, mfar.Z)} };
	UINT indices[36] = {
		// 正面
		0, 1, 2,
		2, 3, 0,
		// 左面
		4, 5, 1,
		1, 0, 4,
		// 顶面
		1, 5, 6,
		6, 2, 1,
		// 背面
		7, 6, 5,
		5, 4, 7,
		// 右面
		3, 2, 6,
		6, 7, 3,
		// 底面
		4, 0, 3,
		3, 7, 4
	};
	bool output = false;
	float minTValue = 10000;
	FVec3 O(ray.origin.x, ray.origin.y, ray.origin.z);
	FVec3 D(ray.direction.x, ray.direction.y, ray.direction.z);
	FVec3 E1, E2, Q, P, T, check;
	for (int i = 0; i < 12; i++)
	{
		E1 = vertices[indices[3*i+1]] - vertices[indices[3 * i]];
		E2 = vertices[indices[3 * i + 2]] - vertices[indices[3 * i]];
		T = O - vertices[indices[3 * i]];
		P = D.Cross(E2);
		Q = T.Cross(E1);
		check = FVec3(Q.Dot(E2), P.Dot(T), Q.Dot(D))*(1 / (P.Dot(E1))) ;
		if (check.Y >= 0 && check.Z >= 0 && check.Y + check.Z <= 1)
		{
			if (check.X < minTValue) minTValue = check.X;
			output = true;
		}
	}
	intersectionPoint = O +  D*minTValue ;
	return output;
}
#endif // FACTOR_H


