#ifndef FPHYSICS_H
#define FPHYSICS_H
#include "PxPhysicsAPI.h"
#include "Fracture.h"
#include <set>
#include"FVoronoi3D.h"
#include "Fracture.h"
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
static physx::PxFilterFlags		FilterShader(
	physx::PxFilterObjectAttributes attributes0,
	physx::PxFilterData filterData0,
	physx::PxFilterObjectAttributes attributes1,
	physx::PxFilterData filterData1,
	physx::PxPairFlags& pairFlags,
	const void* constantBlock,
	uint32_t constantBlockSize)
{
	PX_UNUSED(constantBlock);
	PX_UNUSED(constantBlockSize);
	// let triggers through
	if (PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1))
	{
		pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
		return PxFilterFlags();
	}

	if ((PxFilterObjectIsKinematic(attributes0) || PxFilterObjectIsKinematic(attributes1)) &&
		(PxGetFilterObjectType(attributes0) == PxFilterObjectType::eRIGID_STATIC || PxGetFilterObjectType(attributes1) == PxFilterObjectType::eRIGID_STATIC))
	{
		return PxFilterFlag::eSUPPRESS;
	}

	// use a group-based mechanism if the first two filter data words are not 0
	uint32_t f0 = filterData0.word0 | filterData0.word1;
	uint32_t f1 = filterData1.word0 | filterData1.word1;
	if (f0 && f1 && !(filterData0.word0 & filterData1.word1 || filterData1.word0 & filterData0.word1))
		return PxFilterFlag::eSUPPRESS;

	// determine if we should suppress notification
	const bool suppressNotify = ((filterData0.word3 | filterData1.word3)&ChunkType::INCLUSTER) != 0;

	pairFlags = PxPairFlag::eCONTACT_DEFAULT;
	if (!suppressNotify)
	{
		pairFlags = pairFlags
			| PxPairFlag::eNOTIFY_CONTACT_POINTS
			| PxPairFlag::eNOTIFY_THRESHOLD_FORCE_PERSISTS
			| PxPairFlag::eNOTIFY_THRESHOLD_FORCE_FOUND
			| PxPairFlag::eNOTIFY_TOUCH_FOUND
			| PxPairFlag::eNOTIFY_TOUCH_PERSISTS;
	}

	// eSOLVE_CONTACT is invalid with kinematic pairs
	if (PxFilterObjectIsKinematic(attributes0) && PxFilterObjectIsKinematic(attributes1))
	{
		pairFlags &= ~PxPairFlag::eSOLVE_CONTACT;
	}

	return PxFilterFlags();
}

#endif //FPHYSICS_H
