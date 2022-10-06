#include "FScene.h"
#include "PxPhysicsAPI.h"
#include "FPhysics.h"
FScene::FScene():
	m_pScene(nullptr),
	m_pMaterial(nullptr),
	m_Simulating(false)
{

}

FScene::FScene()
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
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;

	m_pScene = FPhysics::Get()->m_pPhysics->createScene(sceneDesc);
	FASSERT(m_pScene);
	
	m_pMaterial = FPhysics::Get()->CreateMaterial(STONE);

	groundPlane = PxCreatePlane(*FPhysics::Get()->m_pPhysics, PxPlane(0, 1, 0, 0), *m_pMaterial);
	m_pScene->addActor(*groundPlane);
	return true;
Exit0:
	return false;
}

bool FScene::Release()
{
	PHYSX_RELEASE(m_pScene);
	PHYSX_RELEASE(m_pMaterial);
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
	m_pScene->simulate(1.0f / 60.0f);
	m_pScene->fetchResults(true);
Exit0:
	return false;
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
