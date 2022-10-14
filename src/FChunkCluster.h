#ifndef FFRACTURE_GRAPH_H
#define FFRACTURE_GRAPH_H
#include"FDamage.h"
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include "PxPhysicsAPI.h"
#include "Utils.h"
#include "FConnectGraph.h"
using namespace physx;
class FChunk;
class FActor;

class FChunkCluster {
public:
	FChunkCluster() = default;
	FChunkCluster(FActor* actor);
	~FChunkCluster();
	bool Release();
	bool UpdateClusterHealth(FDamage* damage);
	bool Seperate(FChunk* chunk);
	bool Seperate(FChunkCluster* chunk);
	int Size();
	bool InitSharedPhysicsActor();
	PxRigidDynamic* GetPhysicsActor();
	bool Update();
	bool Init(std::unordered_map<int,FChunk*>& chunks, FActor* actor, PxTransform& tran);
	const FConnectGraph<FChunk*>* GetConnectGraph()const {
		return &m_ConnectGraph;
	}
private:
	friend class FRenderMesh;
	FActor* m_pActor;
	bool m_IsSleeping;
	PxRigidDynamic* m_pRigidActor;
	PxTransform m_Transform;

	int m_ChunkCount;

	FConnectGraph<FChunk*> m_ConnectGraph;
	
};


#endif//FFRACTURE_GRAPH_H