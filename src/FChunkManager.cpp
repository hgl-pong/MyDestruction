#include "FChunkManager.h"
#include"FChunk.h"
#include "FChunkCluster.h"
#include "FDamage.h"
#include "FActor.h"
FChunkManager::FChunkManager(FActor* actor)
	:m_pActor(actor)
{
}

FChunkManager::~FChunkManager() {
	Release();
}

bool FChunkManager::Release() {
	for (auto chunk : m_ChunksMap)
		chunk.second->Release();

	for (auto chunkCluster : m_ChunkClustersMap)
		chunkCluster.second->Release();
	return true;
}

bool FChunkManager::Insert(FChunk* chunk) {
	auto result= m_ChunksMap.emplace(chunk->GetPhysicsActor(), chunk).second;
	m_pActor->m_RebuildRenderMesh = true;
	return result;
}

bool FChunkManager::Insert(FChunkCluster* chunkCluster) {
	m_pActor->m_RebuildRenderMesh = true;
	return m_ChunkClustersMap.emplace(chunkCluster->GetPhysicsActor(), chunkCluster).second;
}

bool FChunkManager::RemoveChunk(FChunk* chunk) {
	auto it = find_if(m_ChunksMap.begin(), m_ChunksMap.end(), [&](const std::pair<PxRigidDynamic*, FChunk*>& pair) {return pair.second == chunk; });
	if (it != m_ChunksMap.end()) {
		m_ChunksMap.erase(it);
		m_pActor->m_RebuildRenderMesh = true;
		FRELEASE(chunk);
		return true;
	}
	return false;
}

bool FChunkManager::RemoveChunkCluster(FChunkCluster* chunkCluster) {
	auto it = find_if(m_ChunkClustersMap.begin(), m_ChunkClustersMap.end(), [&](const std::pair<PxRigidDynamic*, FChunkCluster*>& pair) {return pair.second == chunkCluster; });
	if (it != m_ChunkClustersMap.end()) {
		m_ChunkClustersMap.erase(it);
		m_pActor->m_RebuildRenderMesh = true;
		FRELEASE(chunkCluster);
		return true;
	}
	return false;
}

FChunk* FChunkManager::GetFChunk(PxRigidDynamic* actor) {
	auto it = m_ChunksMap.find(actor);
	if (it != m_ChunksMap.end())
		return it->second;
	return nullptr;
}

FChunkCluster* FChunkManager::GetFChunkCluster(PxRigidDynamic* actor){
	auto it = m_ChunkClustersMap.find(actor);
	if (it != m_ChunkClustersMap.end())
		return it->second;
	return nullptr;
}

bool FChunkManager::ApplyDamage(FDamage* damage) {

	for (auto chunk = damage->m_DamagingChunks.begin(); chunk != damage->m_DamagingChunks.end();) {
		damage->Damage(*chunk);
		(*chunk)->CalculateChunkHealth(damage);
		chunk++;
	}

	for (auto chunkCluster = damage->m_DamagingChunkClusters.begin(); chunkCluster != damage->m_DamagingChunkClusters.end();) {
		damage->Damage(*chunkCluster);
		(* chunkCluster)->UpdateClusterHealth(damage);
		chunkCluster++;
	}


	return true;
}

bool FChunkManager::UpdateChunks() {
	for (auto chunk : m_ChunksMap)
		chunk.second->Update();
	for (auto chunkCluster : m_ChunkClustersMap)
		chunkCluster.second->Update();
	return true;
}

