#include "FChunkCluster.h"
#include "FChunk.h"
#include "FActor.h"
FChunkCluster::FChunkCluster( FActor* actor)
	:m_pActor(actor),
	m_ChunkCount(0),
	m_pRigidActor(nullptr)
{	
}

FChunkCluster::~FChunkCluster()
{
	Release();
}

bool FChunkCluster::Release()
{
	PHYSX_RELEASE(m_pRigidActor);
	return false;
}

bool FChunkCluster::Intersection(FDamage& damage)
{	

	if (m_ChunkCount == 0)
		m_pActor->RemoveChunkCluser(this);
	return false;
}


bool FChunkCluster::Seperate(FChunk* chunk)
{
	m_pRigidActor->detachShape(*chunk->m_pConvexMeshShape);
	chunk->m_Transform = m_Transform;
	chunk->m_Volocity = m_pRigidActor->getLinearVelocity();
	chunk->m_IsDestructable = true;
	m_ChunkCount--;
	return false;
}

int FChunkCluster::Size()
{
	return m_ChunkCount;
}

bool FChunkCluster::InitSharedPhysicsActor()
{
	m_pRigidActor = FPhysics::Get()->m_pPhysics->createRigidDynamic(m_Transform);
	FASSERT(m_pRigidActor);
	PxRigidBodyExt::updateMassAndInertia(*m_pRigidActor, m_pActor->m_Material.identity);
	m_pRigidActor->setAngularDamping(0.1f);
	m_pRigidActor->setSleepThreshold((1 - m_pActor->m_Material.material->getRestitution()) / 3);
	return true;
Exit0:
	return false;
}

PxRigidDynamic* FChunkCluster::GetPhysicsActor()
{
	return m_pRigidActor;
}

bool FChunkCluster::Tick()
{

	return true;
}

bool FChunkCluster::Init(std::vector<FChunk*>& chunks, FActor* actor)
{
	if (!InitSharedPhysicsActor())
		return false;
	for (auto chunk : chunks) {

		for (int i = 0; i < chunk->m_Neighbors.size(); i++)
		{
			if (chunk->m_Neighbors[i] < 0)
				continue;
			GraphEdge newEdge;
			newEdge.chunkA = chunk;
			newEdge.chunkB = chunks[chunk->m_Neighbors[i]];
			newEdge.connectHealth = chunk->m_Areas[chunk->m_Neighbors[i]] * actor->m_Material.hardness;
			m_Edges.emplace(newEdge);
		}
		m_pRigidActor->attachShape(*chunk->m_pConvexMeshShape);
		m_ChunkCount++;
	}
	m_Connecting = std::vector<bool>(chunks.size(), true);
	return true;
}
