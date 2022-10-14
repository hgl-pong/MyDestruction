#ifndef FCHUNK_MANAGER_H
#define FCHUNK_MANAGER_H
#include "PxPhysicsAPI.h"
#include <unordered_map>
using namespace physx;
class FActor;
class FChunk;
class FChunkCluster;
class FDamage;
class FChunkManager {
public:
	FChunkManager(FActor*actor);
	~FChunkManager();
	bool Release();
	bool Insert(FChunk* chunk);
	bool Insert(FChunkCluster* chunkCluster);
	
	bool RemoveChunk(FChunk* chunk);
	bool RemoveChunkCluster(FChunkCluster* chunkCluster);
	FChunk* GetFChunk(PxRigidDynamic* actor);
	FChunkCluster* GetFChunkCluster(PxRigidDynamic* actor);
	bool UpdateChunks();
	bool ApplyDamage(FDamage* damage);

private:
	friend class FRenderMesh;
	FActor* m_pActor;
	std::unordered_map<PxRigidDynamic*, FChunk*> m_ChunksMap;
	std::unordered_map<PxRigidDynamic*, FChunkCluster*> m_ChunkClustersMap;
};
#endif // FCHUNK_MANAGER_H
