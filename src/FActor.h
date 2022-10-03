#ifndef FACTOR_H
#define FACTOR_H
#include "PxPhysicsAPI.h"
using namespace physx;
struct FChunk {
	float m_Volume;
	PxVec3 m_Center;
	PxRigidDynamic* m_pPxRigidActor;
	float m_Life;
	bool m_IsSleeping;
};
class FActor
{
public:
	FActor();
	~FActor();

	bool Init();
	bool Release();
	bool ReInit();

	bool AddActor();
	bool RemoveActor();
	bool RemoveAllActors();
	bool Update();
	
	bool OnEnterScene();
	bool OnLeaveScene();
	
	bool SetRenderWireFrame();



};
#endif // FACTOR_H


