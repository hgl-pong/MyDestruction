#include "FChunkCluster.h"
#include "FChunk.h"
#include "FActor.h"
#include <unordered_map>
#include "FChunkManager.h"
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
	for (auto chunk : m_HitChunks) {
		damage->Damage(chunk);
	}
	m_HitChunks.clear();

	for (auto chunk : m_Chunks) {
		damage->Intersection(chunk);
		chunk->m_IsDestructable = true;
	}

	for (std::unordered_set<GraphEdge*>::iterator edge = m_Edges.begin(); edge != m_Edges.end();) {
		if ((* edge)->chunkA->m_Damage.X == 0 && (*edge)->chunkB->m_Damage.X == 0) {
			edge++;
			continue;
		}
		float dam  = (*edge)->connectHealth - ((*edge)->chunkA->m_Damage.X + (*edge)->chunkB->m_Damage.X);
		if (dam< 0) {
			(*edge)->chunkA->m_Damage.Y += dam / 2;
			(*edge)->chunkB->m_Damage.Y += dam / 2;
			m_Edges.erase(edge++);
		}
		else {
			(*edge)->connectHealth = dam;
			edge++;
		}
	}
	if (m_ChunkCount == 1) {
		auto chunk = m_Chunks.begin();
		(* chunk)->m_Damage.Y = -(*chunk)->m_Damage.X;
	}

	for (auto edge : m_Edges) {
		(*edge).chunkA->m_IsDestructable = false;
		(*edge).chunkB->m_IsDestructable = false;
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
	chunk->m_Transform = m_pRigidActor->getGlobalPose();
	//chunk->m_Volocity = m_pRigidActor->getLinearVelocity();
	chunk->InitUniquePhysicsActor();
	m_pActor->m_pChunkManager->Insert(chunk);

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

	//m_pRigidActor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
	//m_pRigidActor->setSleepThreshold((1 - m_pActor->m_Material.material->getRestitution()) / 3);
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
	FASSERT(m_pRigidActor);
	m_IsSleeping = m_pRigidActor->isSleeping();
	for (auto chunk : m_Chunks) {
		for (int i = 0; i < chunk->m_Vertices.size(); i++) {
			PxVec3 temp(chunk->m_Vertices2[i].X, chunk->m_Vertices2[i].Y, chunk->m_Vertices2[i].Z);
			temp = m_pRigidActor->getGlobalPose().transform(temp);
			chunk->m_Vertices[i] = FVec3(temp.x, temp.y, temp.z);
		}
		for (int i = 0; i < chunk->m_Normals.size(); i++) {
			PxVec3 temp(chunk->m_Normals2[i].X, chunk->m_Normals2[i].Y, chunk->m_Normals2[i].Z);
			temp = m_pRigidActor->getGlobalPose().rotate(temp);

			chunk->m_Normals[i] = FVec3(temp.x, temp.y, temp.z);
		}
	}
	return true;
Exit0:
	return false;
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
			GraphEdge* newEdge=new GraphEdge();
			newEdge->chunkA = chunk.second;
			newEdge->chunkB = chunks[chunk.second->m_Neighbors[i]];
			newEdge->connectHealth = chunk.second->m_Areas[i] * actor->m_Material.hardness;
			m_Edges.emplace(newEdge);
		}
		chunk.second->m_pRigidActor = m_pRigidActor;
		m_Chunks.emplace(chunk.second);
		chunk.second->Attach(m_pRigidActor);
		m_ChunkCount++;
	}
	Update();
	return true;
}

bool FChunkCluster::AddHitChunk(FChunk* chunk) {
	FASSERT(chunk);
	m_HitChunks.push_back(chunk);
	return true;
Exit0:
	return false;
}
