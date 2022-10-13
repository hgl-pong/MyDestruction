#include "FScene.h"
#include "PxPhysicsAPI.h"
#include "FPhysics.h"
#include "FActor.h"
#include "FDamage.h"
#include "FChunkManager.h"
#include "FDamage.h"
#include "UI/UI.h"
#include "FChunkCluster.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Overlap
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PxAgain FOverlapCallback::processTouches(const PxOverlapHit* buffer, PxU32 nbHits)
{
	for (PxU32 i = 0; i < nbHits; ++i)
	{
		PxRigidDynamic* rigidDynamic = buffer[i].actor->is<PxRigidDynamic>();
		if (rigidDynamic)
		{
			FChunk* chunk = m_ChunkManager->GetFChunk(rigidDynamic);
			if (chunk != nullptr)
			{
				m_Damage->m_DamagingChunks.emplace(chunk);
			}
			else {
				FChunkCluster* chunkCluster = m_ChunkManager->GetFChunkCluster(rigidDynamic);
				if (chunkCluster != nullptr)
				{
					m_Damage->m_DamagingChunkClusters.emplace(chunkCluster);
					FChunk* hitChunk = m_ChunkManager->GetFChunk(buffer[i].shape);
					chunkCluster->AddHitChunk(hitChunk);
				}
			}
		}
	}
	return true;
}


FScene::FScene() :
	m_pScene(nullptr),
	m_Simulating(false)
{

}

FScene::~FScene()
{
	Release();
}

bool FScene::Init()
{
	PxRigidStatic* groundPlane;
	PxSceneDesc sceneDesc(FPhysics::Get()->m_pPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	sceneDesc.cpuDispatcher = FPhysics::Get()->m_pPhysxCPUDispatcher;
	//sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	sceneDesc.filterShader = FilterShader;

	m_pScene = FPhysics::Get()->m_pPhysics->createScene(sceneDesc);
	FASSERT(m_pScene);

	m_Material = FPhysics::Get()->STONE;

	groundPlane = PxCreatePlane(*FPhysics::Get()->m_pPhysics, PxPlane(0, 1, 0, 0), *m_Material.material);
	m_pScene->addActor(*groundPlane);
	return true;
Exit0:
	return false;
}

bool FScene::Release()
{
	PHYSX_RELEASE(m_pScene);
	return false;
}

bool FScene::ReInit()
{
	Release();
	Init();
	return false;
}

bool FScene::AddActor(FActor* actor)
{
	return m_Actors.emplace(actor).second;
}

bool FScene::RemoveActor(FActor* actor)
{
	auto it = m_Actors.find(actor);
	if (it != m_Actors.end())
	{
		m_Actors.erase(actor);
		return true;
	}
	return false;
}

bool FScene::RemoveAllActors()
{
	m_Actors.clear();
	return true;
}

bool FScene::Update()
{
	FASSERT(m_Simulating);
	for (auto actor : m_Actors)
		actor->Update();
	m_pScene->simulate(1.0f / 60.0f);
	m_pScene->fetchResults(true);
Exit0:
	return false;
}

bool FScene::Intersection(Ray& ray)
{

	PxVec3 origin = PxVec3(ray.origin.x, ray.origin.y, ray.origin.z);
	PxVec3 unitDir = PxVec3(ray.direction.x, ray.direction.y, ray.direction.z);
	PxReal maxDistance = FLT_MAX;

	PxRaycastBuffer hit;
	bool status = m_pScene->raycast(origin, unitDir, maxDistance, hit);


	if (status) {
		FVec3 HitPoint(hit.block.position.x,
			hit.block.position.y,
			hit.block.position.z);
		FSphereDamage damage(HitPoint, UIDrawer::Get()->m_DamageRadius, UIDrawer::Get()->m_Damage);
		damage.GenerateSites(m_Material, NORMAL);
		for (auto actor : m_Actors)
			actor->Intersection(&damage, this);
	}


	return status;
}

bool FScene::SetSimulateState(bool simulate)
{
	m_Simulating = simulate;
	return true;
}

bool FScene::GetSimulateState()
{
	return m_Simulating;
}
