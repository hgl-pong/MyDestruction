#ifndef FSCENE_H
#define FSCENE_H
#include "PxPhysicsAPI.h"
#include "Common/Collision.h"
#include"FPhysics.h"
#include <set>
using namespace physx;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Overlap
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//class FOverlapCallback : public PxOverlapCallback
//{
//public:
//    FOverlapCallback(std::set<FChunk*>& actorBuffer)
//        :  m_actorBuffer(actorBuffer), PxOverlapCallback(m_hitBuffer, sizeof(m_hitBuffer) / sizeof(m_hitBuffer[0])) {}
//
//    PxAgain processTouches(const PxOverlapHit* buffer, PxU32 nbHits)
//    {
//        for (PxU32 i = 0; i < nbHits; ++i)
//        {
//            PxRigidDynamic* rigidDynamic = buffer[i].actor->is<PxRigidDynamic>();
//            if (rigidDynamic)
//            {
//                FChunk* actor = m_pxManager.getActorFromPhysXActor(*rigidDynamic);
//                if (actor != nullptr)
//                {
//                    m_actorBuffer.insert(actor);
//                }
//            }
//        }
//        return true;
//    }
//
//private:
//    std::set<FChunk*>& m_actorBuffer;
//    PxOverlapHit m_hitBuffer[1000];
//};
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


