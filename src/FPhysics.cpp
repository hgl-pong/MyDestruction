#include "FPhysics.h"
#include "PxPhysicsAPI.h"
#include "FScene.h"
namespace {
	FPhysics* m_Physics_Instance = nullptr;
}
FPhysics::FPhysics() :
	m_pPhysics(nullptr),
	m_pFoundation(nullptr),
	m_pCooking(nullptr),
	m_pPhysxCPUDispatcher(nullptr),
	m_pPvd(nullptr)
{
	if (m_Physics_Instance)
		throw std::exception("MeshImporter is a singleton!");
	m_Physics_Instance = this;
}

FPhysics::~FPhysics()
{
	Release();
}

bool FPhysics::Init()
{
	bool nResult = false;
	const unsigned uNumTreads = 4;
	physx::PxTolerancesScale scale;	
	physx::PxCookingParams cookingParams(scale);
	PxPvdTransport* transport;
	m_pFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_DefaultAllocator, m_DefaultErrorCallBack);
	FASSERT(m_pFoundation);
	transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	m_pPvd = PxCreatePvd(*m_pFoundation);
	FASSERT(m_pPvd);
	m_pPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

	m_pPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_pFoundation, scale, true,m_pPvd);
	FASSERT(m_pPhysics);

	cookingParams.buildGPUData = true;
	m_pCooking = PxCreateCooking(PX_PHYSICS_VERSION, m_pPhysics->getFoundation(), cookingParams);
	FASSERT(m_pCooking);

	m_pPhysxCPUDispatcher = PxDefaultCpuDispatcherCreate(uNumTreads);
	FASSERT(m_pPhysxCPUDispatcher);
	nResult = true;
	return nResult;
	Exit0:
	return nResult;
}

bool FPhysics::Release()
{
	PHYSX_RELEASE(m_pPhysics);
	PHYSX_RELEASE(m_pFoundation);
	PHYSX_RELEASE(m_pCooking);
	PHYSX_RELEASE(m_pPhysxCPUDispatcher);
	if (m_pPvd)
	{
		PxPvdTransport* transport = m_pPvd->getTransport();
		m_pPvd->release();	m_pPvd = NULL;
		PHYSX_RELEASE(transport);
	}
	return false;
}

FPhysics* FPhysics::Get() {
	if (!m_Physics_Instance)
		throw std::exception("Physics needs an instance");
	return m_Physics_Instance;
}


physx::PxMaterial* FPhysics::CreateMaterial(FMaterial& material)
{
	FASSERT(m_pPhysics);
	return m_pPhysics->createMaterial(material.staticFriction, material.dynamicFriction, material.restitution);
Exit0:
	return nullptr;
}

bool FPhysics::AddScene(FScene* scene)
{
	auto it = m_SimulatingScenes.emplace(scene);
	return it.second;
}

bool FPhysics::RemoveScene(FScene* scene)
{
	auto it = m_SimulatingScenes.find(scene);
	if (it != m_SimulatingScenes.end())
		return true;
	else
		return false;
}

bool FPhysics::Update()
{
	for (auto scene : m_SimulatingScenes)
		scene->Update();
	return true;
}

