#ifndef FSCENE_H
#define FSCENE_H
#include "PxPhysicsAPI.h"
#include "Common/Collision.h"
#include"FPhysics.h"
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


	bool Intersection(Ray& ray);
	bool SetSimulateState(bool simulate);
	bool GetSimulateState();

	PxScene* GetPhysicsScene()const {
		return m_pScene;
	}
private:
	PxScene* m_pScene;
	FMaterial m_Material;
	bool m_Simulating;
	std::set<FActor*> m_Actors;
};
#endif // FSCENE_H


