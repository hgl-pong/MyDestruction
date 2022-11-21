#include "FChunkCluster.h"
#include "FChunk.h"
#include "FActor.h"
#include <unordered_map>
#include "FChunkManager.h"
FChunkCluster::FChunkCluster(FActor* actor)
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
	for (auto chunk : m_ConnectGraph.GetNodes())
		chunk->Release();
	return false;
}

bool FChunkCluster::UpdateClusterHealth(FDamage* damage)
{
	std::set<GraphEdge<FChunk*>> buffer;
	for (auto node : m_ConnectGraph.m_Container) {
		for (auto edge : node.second) {
			if (buffer.find(edge) != buffer.end())
				continue;
			float dam = edge.m_ConnectHealth - (node.first->m_Damage.X + edge.m_Node->m_Damage.X);
			if (dam < 0) {
				node.first->m_Damage.Y += dam / 2;
				edge.m_Node->m_Damage.Y += dam / 2;
				m_ConnectGraph.RemoveEdge(node.first, edge.m_Node);
			}
			else {
				m_ConnectGraph.ChangeHealth(node.first, edge.m_Node, dam);
			}
			buffer.emplace(edge);
		}
	}

	for (auto chunk : m_ConnectGraph.CollectAllIsolatedNode()) {
		damage->Damage(chunk,true);
		chunk->m_IsDestructable = true;
		Seperate(chunk);	
		m_ConnectGraph.RemoveNode(chunk);
		chunk->CalculateChunkHealth(damage);
	}
	if (!m_ChunkCount)
		return false;
	std::vector<std::map<FChunk*, std::set<GraphEdge<FChunk*>>>> clusters = m_ConnectGraph.CollectAllCluster();
	m_ConnectGraph = FConnectGraph(clusters[0]);

	for (int i = 1; i < clusters.size(); i++) {
		FChunkCluster* newChunkCluster = new FChunkCluster();
		newChunkCluster->m_ConnectGraph = FConnectGraph(clusters[i]);
		newChunkCluster->m_ChunkCount = newChunkCluster->m_ConnectGraph.NodeSize();
		Seperate(newChunkCluster);
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
	PxRigidBodyExt::updateMassAndInertia(*m_pRigidActor, m_pActor->m_Material.identity);

	return false;
}

bool FChunkCluster::Seperate(FChunkCluster* chunkCluster)
{	
	chunkCluster->m_Transform = m_pRigidActor->getGlobalPose();
	//chunk->m_Volocity = m_pRigidActor->getLinearVelocity();
	chunkCluster->InitSharedPhysicsActor();
	std::vector<FChunk*>buffer = chunkCluster->m_ConnectGraph.GetNodes();
	for (auto chunk : buffer) {
		m_pRigidActor->detachShape(*chunk->m_pConvexMeshShape);
		chunk->Attach(chunkCluster->GetPhysicsActor());
		chunk->m_pRigidActor = chunkCluster->GetPhysicsActor();
	}
	PxRigidBodyExt::updateMassAndInertia(*chunkCluster->GetPhysicsActor(), m_pActor->m_Material.identity);
	m_pActor->m_pChunkManager->Insert(chunkCluster);
	chunkCluster->m_pActor = m_pActor;
	m_pActor->AddPhysicsActorToScene(chunkCluster->GetPhysicsActor());
	m_ChunkCount-= buffer.size();
	PxRigidBodyExt::updateMassAndInertia(*m_pRigidActor, m_pActor->m_Material.identity);
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
	for (auto chunk : m_ConnectGraph.GetNodes()) {
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

bool FChunkCluster::Init(std::unordered_map<int, FChunk*>& chunks, FActor* actor, PxTransform& tran)
{
	m_Transform = tran;
	if (!InitSharedPhysicsActor())
		return false;

	for (auto chunk : chunks) {
		m_ConnectGraph.AddNode(chunk.second);
		for (int i = 0; i < chunk.second->m_Neighbors.size(); i++)
		{
			if (chunk.second->m_Neighbors[i] < 0)
				continue;

			FChunk* chunkA = chunk.second;
			FChunk* chunkB = chunks[chunk.second->m_Neighbors[i]];
			float connectHealth = 0/*chunk.second->m_Areas[i] * actor->m_Material.hardness*/;
			m_ConnectGraph.AddEdge(chunkA, chunkB, connectHealth);
		}
		chunk.second->m_pRigidActor = m_pRigidActor;
		chunk.second->Attach(m_pRigidActor);
		m_ChunkCount++;
	}
	PxRigidBodyExt::updateMassAndInertia(*m_pRigidActor, m_pActor->m_Material.identity);
	Update();
	return true;
}

