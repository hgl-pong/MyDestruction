#ifndef FPHYSICS_H
#define FPHYSICS_H
#include "PxPhysicsAPI.h"
#include "Fracture.h"
#include <set>
#include"FVoronoi3D.h"
#define PVD_HOST "127.0.0.1"
using namespace physx;
struct FMaterial
{
	float dynamicFriction;
	float staticFriction;
	float restitution;
	float identity;
};
static FMaterial STONE = { 0.8,0.8,0.2,1000 };

class FScene;
class FPhysics {
public:
	FPhysics();
	~FPhysics();

	bool Init();
	bool Release();
	static FPhysics* Get();

	PxMaterial* CreateMaterial(FMaterial& material);

	bool AddScene(FScene* scene);
	bool RemoveScene(FScene* scene);

	bool AddToScene(PxRigidDynamic*, FScene* scene);
	bool REmoveFromScene(PxRigidDynamic*, FScene* scene);
	PxRigidDynamic* CreatePhysicActor(VoronoiCellInfo& cellInfo,PxMaterial* material,PxTransform& tran);
	PxShape* CreateConvexShape(VoronoiCellInfo& cellInfo, PxMaterial* material);
	bool Update();
public:
	PxPhysics* m_pPhysics;
	PxFoundation* m_pFoundation;
	PxCooking* m_pCooking;
	PxDefaultCpuDispatcher* m_pPhysxCPUDispatcher;
	PxPvd* m_pPvd;
private:
	PxDefaultErrorCallback m_DefaultErrorCallBack;
	PxDefaultAllocator m_DefaultAllocator;

	std::set<FScene*> m_SimulatingScenes;

};
#endif //FPHYSICS_H
