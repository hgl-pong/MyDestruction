#include "FActor.h"
#include "FScene.h"
#include "Utils.h"
#include "FWireMesh.h"
#include "Common/Geometry.h"
#include "Importer/MeshImporter.h"
#include "Common/MaterialManager.h"
#include "renderer/Renderer.h"
#include "FSiteGenerator.h"
#include "FDamage.h"
#include "FPhysics.h"
#include "FChunk.h"
#include "FChunkCluster.h"
#include "FChunkManager.h"
#include "FRenderMesh.h"
FActor::FActor()
	: m_pWireMesh(nullptr),
	m_pRenderMesh(nullptr),
	m_pVoroMeshData(nullptr),
	m_pMeshData(nullptr),
	m_pChunkManager(new FChunkManager(this)),
	m_RebuildRenderMesh(false)
{
}

FActor::~FActor()
{
	Release();
}

bool FActor::Init(char *name)
{
	m_Name = name;
	Geometry::MeshData box = Geometry::CreateBox(2.5, 2.5, 2.5);
	m_pMeshData = new Graphics::MeshData();
	;
	*m_pMeshData = MeshImporter::Get().CreateFromGeometry("box", box)->meshData;
	m_pMeshData->m_Transform.SetPosition(0, 1.3, 0);
	m_Transform = m_pMeshData->m_Transform;
	BoundingBox::CreateFromPoints(m_pMeshData->m_BoundingBox, box.vertices.size(),
								  box.vertices.data(), sizeof(XMFLOAT3));
	m_pRenderMesh = new FRenderMesh(this);

	m_pWireMesh = new FWireMesh(m_pMeshData->m_BoundingBox);
	if (m_pWireMesh->LoadMeshData())
	{
		m_pVoroMeshData = Graphics::Renderer::Get()->CreateVoroMeshData(m_pWireMesh);
		m_pVoroMeshData->m_Transform = m_pMeshData->m_Transform;
		m_pVoroMeshData->m_BoundingBox = m_pMeshData->m_BoundingBox;
		Graphics::Renderer::Get()->AddVoroMesh(m_pVoroMeshData);
	}
	else
	{
		m_pVoroMeshData = Graphics::Renderer::Get()->createBoundingBoxMesh(&m_pMeshData->m_BoundingBox);
		m_pVoroMeshData->m_Transform = m_pMeshData->m_Transform;
		m_pVoroMeshData->m_BoundingBox = m_pMeshData->m_BoundingBox;
		Graphics::Renderer::Get()->AddVoroMesh(m_pVoroMeshData);
	}
	m_Material = FPhysics::Get()->STONE;
	FChunk *chunk = new FChunk(box, this);
	chunk->m_Transform = PxTransform(PxVec3(m_pMeshData->m_Transform.GetPosition().x, m_pMeshData->m_Transform.GetPosition().y, m_pMeshData->m_Transform.GetPosition().z));
	std::unordered_map<int, FChunk *> chunks;
	chunks.emplace(chunk->m_Id, chunk);
	FChunkCluster *chunkCluster = new FChunkCluster(this);
	chunkCluster->Init(chunks, this, chunk->m_Transform);
	// m_ChunkClusters.emplace(chunkCluster);
	chunk->InitUniquePhysicsActor();
	chunk->m_IsDestructable = true;

	m_pChunkManager->Insert(chunk);
	AddPhysicsActorToScene(chunk->GetPhysicsActor());

	// AddPhysicsActorToScene(chunkCluster->GetPhysicsActor());
	return true;
}

bool FActor::Release()
{
	FRELEASE(m_pWireMesh);
	FRELEASE(m_pRenderMesh);
	FRELEASE(m_pChunkManager);
	return true;
}

bool FActor::ReInit()
{
	return false;
}

bool FActor::Update()
{
	m_pChunkManager->UpdateChunks();
	m_pRenderMesh->UpdateMeshData();
	return false;
}

bool FActor::OnEnterScene(FScene *scene)
{
	scene->AddActor(this);
	m_Scenes.emplace(scene);

	return true;
}

bool FActor::OnLeaveScene(FScene *scene)
{
	scene->RemoveActor(this);
	auto it = m_Scenes.find(scene);
	if (it != m_Scenes.end())
		m_Scenes.erase(it);
	return true;
}

bool FActor::AddPhysicsActorToScene(PxRigidDynamic *actor)
{
	FASSERT(actor);
	for (auto scene : m_Scenes)
	{
		FPhysics::Get()->AddToScene(actor, scene);
	}
	return true;
Exit0:
	return false;
}

bool FActor::Intersection(FDamage *damage, FScene *scene)
{
	FOverlapCallback overlap(m_pChunkManager, damage);
	scene->GetPhysicsScene()->overlap(*damage->m_Shape, PxTransform(PxVec3(damage->m_Position.X, damage->m_Position.Y, damage->m_Position.Z)), overlap);
	Graphics::Renderer::Get()->createHitPosSphere(damage->m_Position, 1);
	// Graphics::Renderer::Get()->m_pHitPos->m_Transform = m_pMeshData->m_Transform;
	Graphics::Renderer::Get()->AddVoroMesh(Graphics::Renderer::Get()->m_pHitPos);
	m_pChunkManager->ApplyDamage(damage);

	// for (auto chunk : m_Chunks) {
	//	damage->Intersection(chunk);
	//	chunk->m_Damage.Y = -chunk->m_Damage.X;
	// }

	// for (auto it = m_ChunkClusters.begin(); it != m_ChunkClusters.end();) {
	//	(*it)->Intersection(damage);
	//	if ((*it)->Size() == 0) {
	//		RemoveChunkCluser(*it++);
	//		continue;
	//	}
	//	it++;
	// }

	// for (auto chunk : damage->m_DamagingChunks) {
	//	FChunkCluster* newChunkCluster=nullptr;
	//	if (chunk->VoronoiFracture(damage->m_Sites,newChunkCluster)) {
	//		newChunkCluster->Intersection(damage);
	//		chunk->Release();
	//	}
	// }

	//std::vector<FVec3> sites;
	FVec3 transform(m_pMeshData->m_Transform.GetPosition().x,
					m_pMeshData->m_Transform.GetPosition().y,
					m_pMeshData->m_Transform.GetPosition().z);
	//FSiteGenerator::ImpactSphereDamage(damage->m_Position, 1, 100, sites, RandomType::GAUSSION, transform);
	// FVec3 normal(ray.direction.x, ray.direction.y, ray.direction.z);
	// FSiteGenerator::PlaneImpactDamage(m_pMeshData->m_BoundingBox,damage.center,normal, transform ,100, sites);
	Graphics::MeshData *newMesh;

	Transform trans;
	trans.SetPosition(0, 10, 0);
	trans.Translate(XMFLOAT3(0, 0, 1), 5 * m_HitTime);
	m_HitTime++;

	PxVec3 pos(trans.GetPosition().x, trans.GetPosition().y, trans.GetPosition().z);
	PxTransform tran(pos);

	FASSERT(m_pWireMesh->VoronoiFracture(damage->m_Sites));
	m_pWireMesh->LoadMeshData();

	// m_pWireMesh->CreaePhysicsActor(trans);
	// for (int i = 0; i < m_pWireMesh->m_pVoronoi3D->Size(); i++) {
	//	//if (m_pWireMesh->m_pCellInfo[i].Vertices.empty())
	//	//	continue;
	//	//FChunk* newchunk = new FChunk(m_pWireMesh->m_pCellInfo[i], this);
	//	//newchunk->m_Transform = tran;
	//	//newchunk->InitUniquePhysicsActor();
	//	//FPhysics::Get()->AddToScene(newchunk->GetPhysicsActor(), scene);
	//	FPhysics::Get()->AddToScene(m_pWireMesh->m_pCellInfo[i].rigidDynamic, scene);

	//}

	FASSERT(!m_pWireMesh->vertices.empty());

	newMesh = Graphics::Renderer::Get()->CreateVoroMeshData(m_pWireMesh);
	*m_pVoroMeshData = *newMesh;
	delete newMesh;
	m_pVoroMeshData->m_Transform = m_pMeshData->m_Transform;
	m_pVoroMeshData->m_BoundingBox = m_pMeshData->m_BoundingBox;


	m_pWireMesh->AddToScene(scene);
	return true;
Exit0:
	return false;
}

bool FActor::RemoveChunk(FChunk *chunk)
{
	FASSERT(chunk);
	for (auto scene : m_Scenes)
	{
		if (!chunk->GetPhysicsActor())
			continue;
		FPhysics::Get()->RemoveFromScene(chunk->GetPhysicsActor(), scene);
	}
	m_pChunkManager->RemoveChunk(chunk);
	return true;
Exit0:
	return false;
}

bool FActor::RemoveChunkCluser(FChunkCluster *chunkCluster)
{
	FASSERT(chunkCluster);
	for (auto scene : m_Scenes)
	{
		if (!chunkCluster->GetPhysicsActor())
			continue;
		FPhysics::Get()->RemoveFromScene(chunkCluster->GetPhysicsActor(), scene);
	}
	m_pChunkManager->RemoveChunkCluster(chunkCluster);
	return true;
Exit0:
	return false;
}

bool FActor::SetRenderWireFrame(bool render)
{
	if (render)
	{
		Graphics::Renderer::Get()->AddVoroMesh(m_pVoroMeshData);
	}
	else
	{
		Graphics::Renderer::Get()->RemoveVoroMesh(m_pVoroMeshData);
	}
	return true;
}
