#include "FChunkCluster.h"
#include "FChunk.h"
#include "FActor.h"
#include <unordered_map>
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
	for (auto chunk : m_Chunks)
		chunk->Release();
	m_Edges.clear();
	return false;
}

bool FChunkCluster::Intersection(FDamage* damage)
{	
	for (auto chunk : m_Chunks) {
		damage->Intersection(chunk);
		chunk->m_IsDestructable = true;
	}

	for (std::unordered_set<GraphEdge>::iterator edge = m_Edges.begin(); edge != m_Edges.end();) {
		if (edge->chunkA->m_Damage.X == 0 && edge->chunkB->m_Damage.X == 0) {
			edge++;
			continue;
		}
		float dam  = edge->connectHealth - (edge->chunkA->m_Damage.X + edge->chunkB->m_Damage.X);
		if (dam< 0) {
			edge->chunkA->m_Damage.Y += dam / 2;
			edge->chunkB->m_Damage.Y += dam / 2;
			m_Edges.erase(edge++);
		}
		else {
			m_Edges.insert(*edge);
			edge++;
		}
	}
	if (m_ChunkCount == 1) {
		auto chunk = m_Chunks.begin();
		(* chunk)->m_Damage.Y = -(*chunk)->m_Damage.X;
	}

	for (auto edge : m_Edges) {
		edge.chunkA->m_IsDestructable = false;
		edge.chunkB->m_IsDestructable = false;
	}

	for (auto chunk = m_Chunks.begin(); chunk != m_Chunks.end();) {
		(*chunk)->m_ChunkHealth += (*chunk)->m_Damage.Y;
		(*chunk)->m_Damage = FVec2(0, 0);
		if ((*chunk)->m_IsDestructable) {

			(*chunk)->m_IsDestructable = false;
			Seperate(*chunk);	
			m_Chunks.erase(chunk++);

			continue;
		}
		chunk++;
	}
	return false;
}


bool FChunkCluster::Seperate(FChunk* chunk)
{
	m_pRigidActor->detachShape(*chunk->m_pConvexMeshShape);
	chunk->m_Transform = m_Transform;
	//chunk->m_Volocity = m_pRigidActor->getLinearVelocity();
	chunk->InitUniquePhysicsActor();
	m_pActor->m_Chunks.emplace(chunk);

	m_pActor->AddPhysicsActorToScene(chunk->GetPhysicsActor());
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

	m_pRigidActor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
	m_pRigidActor->setSleepThreshold((1 - m_pActor->m_Material.material->getRestitution()) / 3);
	return true;
Exit0:
	return false;
}

PxRigidDynamic* FChunkCluster::GetPhysicsActor()
{
	return m_pRigidActor;
}

bool FChunkCluster::Update()
{
	m_Transform = m_Transform * m_pRigidActor->getGlobalPose();
	return true;
}

bool FChunkCluster::Init(std::unordered_map<int,FChunk*>& chunks, FActor* actor,PxTransform&tran)
{
	m_Transform = tran;
	if (!InitSharedPhysicsActor())
		return false;

	for (auto chunk : chunks) {

		for (int i = 0; i < chunk.second->m_Neighbors.size(); i++)
		{
			if (chunk.second->m_Neighbors[i] < 0)
				continue;
			GraphEdge newEdge;
			newEdge.chunkA = chunk.second;
			newEdge.chunkB = chunks[chunk.second->m_Neighbors[i]];
			newEdge.connectHealth = chunk.second->m_Areas[i] * actor->m_Material.hardness;
			m_Edges.emplace(newEdge);
		}
		chunk.second->m_pRigidActor = m_pRigidActor;
		m_Chunks.emplace(chunk.second);
		chunk.second->Attach(m_pRigidActor);
		m_ChunkCount++;
	}
	return true;
}
