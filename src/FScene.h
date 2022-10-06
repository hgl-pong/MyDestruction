#ifndef FSCENE_H
#define FSCENE_H
#include "PxPhysicsAPI.h"
#include <set>
using namespace physx;
class FActor;
class FScene
{
public:
	FScene();
	~FScene();
	
	bool Init();
	bool Release();
	bool ReInit();

	bool AddActor(FActor*actor);
	bool RemoveActor(FActor* actor);
	bool RemoveAllActors();
	bool Update();

	bool SetSimulateState(bool simulate);
	bool GetSimulateState();
private:
	PxScene* m_pScene;
	PxMaterial* m_pMaterial;
	bool m_Simulating;
	std::set<FActor*> m_Actors;
};
#endif // FSCENE_H


