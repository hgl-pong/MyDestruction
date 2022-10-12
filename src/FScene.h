#ifndef FSCENE_H
#define FSCENE_H
#include "PxPhysicsAPI.h"
#include "Common/Collision.h"
#include"FPhysics.h"
#include <set>
using namespace physx;
class FActor;
class FChunk;
class FChunkManager;
class FDamage;
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Overlap
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class FOverlapCallback : public PxOverlapCallback
{
public:
	FOverlapCallback(FChunkManager* manager, FDamage* damage)
		: m_Damage(damage),
		m_ChunkManager(manager),
		PxOverlapCallback(m_hitBuffer, sizeof(m_hitBuffer) / sizeof(m_hitBuffer[0])) {}
	PxAgain processTouches(const PxOverlapHit* buffer, PxU32 nbHits);
private:
	FDamage* m_Damage;
	FChunkManager* m_ChunkManager;
	PxOverlapHit m_hitBuffer[1000];
};
#endif // FSCENE_H


