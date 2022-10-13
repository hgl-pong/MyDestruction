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
	auto s1= m_ChunksMap.emplace(chunk->GetPhysicsActor(), chunk).second;
	auto s2= m_ChunkShapesMap.emplace(chunk->GetPhysicsShape(), chunk).second;
	m_pActor->m_RebuildRenderMesh = true;
	return s1 && s2;
}

bool FChunkManager::Insert(FChunkCluster* chunkCluster) {
	m_pActor->m_RebuildRenderMesh = true;
	return m_ChunkClustersMap.emplace(chunkCluster->GetPhysicsActor(), chunkCluster).second;
}

bool FChunkManager::RemoveChunk(FChunk* chunk) {
	auto it1 = find_if(m_ChunksMap.begin(), m_ChunksMap.end(), [&](const std::pair<PxRigidDynamic*, FChunk*>& pair) {return pair.second == chunk; });
	auto it2 = find_if(m_ChunkShapesMap.begin(), m_ChunkShapesMap.end(), [&](const std::pair<PxShape*, FChunk*>& pair) {return pair.second == chunk; });
	if (it1 != m_ChunksMap.end()&&it2 != m_ChunkShapesMap.end()) {
		m_ChunksMap.erase(it1);
		m_ChunkShapesMap.erase(it2);
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

FChunk* FChunkManager::GetFChunk(PxShape* shape) {
	auto it = m_ChunkShapesMap.find(shape);
	if (it != m_ChunkShapesMap.end())
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

	for (auto it = damage->m_DamagingChunkClusters.begin(); it != damage->m_DamagingChunkClusters.end();) {
		(*it)->Intersection(damage);
		if ((*it)->Size() == 0) {
			m_pActor->RemoveChunkCluser(*it++);
			continue;
		}
		it++;
	}

	for (auto chunk : damage->m_DamagingChunks) {
		FChunkCluster* newChunkCluster = nullptr;
		if (chunk->VoronoiFracture(damage->m_Sites, newChunkCluster)) {
			newChunkCluster->Intersection(damage);
			m_pActor->RemoveChunk(chunk);
		}
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

