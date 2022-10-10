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
	PxMaterial* material;
	float identity;
	float hardness;
	~FMaterial() {
		PHYSX_RELEASE(material);
	}
};

class FScene;
class FPhysics {
public:
	FPhysics();
	~FPhysics();

	bool Init();
	bool Release();
	static FPhysics* Get();

	FMaterial* CreateMaterial(float staticFriction,float dynamicFriction,float restitution,float identity,float hardness);

	bool AddScene(FScene* scene);
	bool RemoveScene(FScene* scene);

	bool AddToScene(PxRigidDynamic*, FScene* scene);
	bool RemoveFromScene(PxRigidDynamic*, FScene* scene);
	PxRigidDynamic* CreatePhysicActor(VoroCellInfo& cellInfo,FMaterial& material,PxTransform& tran);
	PxShape* CreateConvexShape(std::vector<FVec3>& Vertices, std::vector<uint32_t>& Indices, FMaterial& material);
	bool Update();
public:
	PxPhysics* m_pPhysics;
	PxFoundation* m_pFoundation;
	PxCooking* m_pCooking;
	PxDefaultCpuDispatcher* m_pPhysxCPUDispatcher;
	PxPvd* m_pPvd;

	FMaterial STONE;
	FMaterial PLASTIC;
	FMaterial GLASS;
private:
	PxDefaultErrorCallback m_DefaultErrorCallBack;
	PxDefaultAllocator m_DefaultAllocator;

	std::set<FScene*> m_SimulatingScenes;



};


#endif //FPHYSICS_H
