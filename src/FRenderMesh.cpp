#include "FRenderMesh.h"
#include "FActor.h"
#include "renderer\Renderer.h"
#include "Utils.h"
#include "Common\MaterialManager.h"
#include"FChunk.h"
#include"FChunkCluster.h"
#include"FChunkManager.h"
FRenderMesh::FRenderMesh(FActor* actor)
	:m_pActor(actor),
	m_pMaterial(nullptr),
	m_pMeshData(nullptr)
{
	Geometry::MeshData box = Geometry::CreateBox(2.5, 2.5, 2.5);
	m_Vertices.resize(box.vertices.size());
	m_Normals.resize(box.vertices.size());
	m_UVs.resize(box.vertices.size());
	m_Indices.resize(box.indices32.size());
	memcpy_s(m_Vertices.data(), sizeof(FVec3) * m_Vertices.size(), box.vertices.data(), sizeof(XMFLOAT3) * m_Vertices.size());
	memcpy_s(m_Normals.data(), sizeof(FVec3) * m_Vertices.size(), box.normals.data(), sizeof(XMFLOAT3) * m_Vertices.size());
	memcpy_s(m_UVs.data(), sizeof(FVec2) * m_Vertices.size(), box.texcoords.data(), sizeof(XMFLOAT2) * m_Vertices.size());

	memcpy_s(m_Indices.data(), sizeof(uint32_t) * m_Indices.size(), box.indices32.data(), sizeof(uint32_t) * m_Indices.size());
	CreateRenderData();
}

void FRenderMesh::Release() {
	FDELETE(m_pMeshData);
}

void FRenderMesh::UpdateMeshData() {
	if (m_pActor->m_RebuildRenderMesh) {
		m_Vertices.clear();
		m_Normals.clear();
		m_UVs.clear();
		m_Indices.clear();
		int vpos = 0;
		int ipos = 0;
		for (auto chunk : m_pActor->m_pChunkManager->m_ChunksMap) {
			int vsize = chunk.second->m_Vertices.size();
			int isize = chunk.second->m_Indices.size();
			m_Vertices.resize(vpos+vsize);
			memcpy_s(m_Vertices.data() + vpos, sizeof(FVec3) * vsize, chunk.second->m_Vertices.data(), sizeof(FVec3) * vsize);

			m_Normals.resize(vpos + vsize);
			memcpy_s(m_Normals.data() + vpos, sizeof(FVec3) * vsize, chunk.second->m_Normals.data(), sizeof(FVec3) * vsize);

			m_Indices.resize(ipos + isize);
			memcpy_s(m_Indices.data() + ipos, sizeof(uint32_t) * isize, chunk.second->m_Indices.data(), sizeof(uint32_t) * isize);

			std::for_each(m_Indices.begin()+ipos, m_Indices.end(),
				[&vpos](uint32_t& i)
			{ i = i + vpos; });
			chunk.second->m_VBStartPos = vpos;
			vpos += vsize;
			ipos += isize;
		}
		for (auto chunkCluster : m_pActor->m_pChunkManager->m_ChunkClustersMap) {
			for (auto chunk : chunkCluster.second->m_Chunks) {
				int vsize = chunk->m_Vertices.size();
				int isize = chunk->m_Indices.size();
				m_Vertices.resize(vpos + vsize);
				memcpy_s(m_Vertices.data() + vpos, sizeof(FVec3) * vsize, chunk->m_Vertices.data(), sizeof(FVec3) * vsize);

				m_Normals.resize(vpos + vsize);
				memcpy_s(m_Normals.data() + vpos, sizeof(FVec3) * vsize, chunk->m_Normals.data(), sizeof(FVec3) * vsize);

				m_Indices.resize(ipos + isize);
				memcpy_s(m_Indices.data() + ipos, sizeof(uint32_t) * isize, chunk->m_Indices.data(), sizeof(uint32_t) * isize);
				std::for_each(m_Indices.begin() + ipos, m_Indices.end(),
					[&vpos](uint32_t& i)
				{ i = i + vpos; });
				chunk->m_VBStartPos = vpos;
				vpos += vsize;
				ipos += isize;
			}
		}
		m_UVs = std::vector<FVec2>(vpos, FVec2(0, 1));
		if(vpos)
		CreateRenderData();
		m_pActor->m_RebuildRenderMesh = FALSE;
	}
	else {
		for (auto chunk : m_pActor->m_pChunkManager->m_ChunksMap) {
			int vsize = chunk.second->m_Vertices.size();

			if (!chunk.second->m_IsSleeping) {
				memcpy_s(m_Vertices.data() + chunk.second->m_VBStartPos, sizeof(FVec3) * vsize, chunk.second->m_Vertices.data(), sizeof(FVec3) * vsize);
			}

		}
		for (auto chunkCluster : m_pActor->m_pChunkManager->m_ChunkClustersMap) {
			if(!chunkCluster.second->m_IsSleeping)
			for (auto chunk : chunkCluster.second->m_Chunks) {
				int vsize = chunk->m_Vertices.size();
				memcpy_s(m_Vertices.data() + +chunk->m_VBStartPos, sizeof(FVec3) * vsize, chunk->m_Vertices.data(), sizeof(FVec3) * vsize);
			}
		}
		Graphics::Renderer::Get()->UpdateVerticesData(m_pMeshData, m_Vertices);
	}

}

void FRenderMesh::CreateRenderData() {
	Graphics::Renderer::Get()->RemoveRenderMesh(m_pMeshData);
	FDELETE(m_pMeshData);
	m_pMeshData = Graphics::Renderer::Get()->CreateRenderMeshData(this);
	m_pMeshData->m_pMaterial = MaterialManager::Get().createMaterial("..\\Texture\\stone.dds");
	m_pMeshData->m_Transform = m_pActor->m_Transform;
	BoundingBox::CreateFromPoints(m_pMeshData->m_BoundingBox, m_Vertices.size(),
		(XMFLOAT3*)m_Vertices.data(), sizeof(XMFLOAT3));
	Graphics::Renderer::Get()->AddRenderMesh(m_pMeshData);
}