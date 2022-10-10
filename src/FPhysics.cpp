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

	STONE = *CreateMaterial(0.6, 0.6, 0.5, 2000, 1000);
	PLASTIC = *CreateMaterial(0.4, 0.4, 0.3, 500, 500);
	GLASS = *CreateMaterial(0.1, 0.1, 0.1, 1000, 1000);

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


FMaterial* FPhysics::CreateMaterial(float staticFriction, float dynamicFriction, float restitution, float identity, float hardness)
{
	FMaterial* newMaterial;
	FASSERT(m_pPhysics);
	newMaterial = new FMaterial();
	newMaterial->material=m_pPhysics->createMaterial(staticFriction, dynamicFriction,restitution);
	newMaterial->identity = identity;
	newMaterial->hardness = hardness;
	return newMaterial;
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

bool FPhysics::AddToScene(PxRigidDynamic*actor, FScene* scene)
{
	FASSERT(actor);
	scene->GetPhysicsScene()->addActor(*actor);
	return true;
	Exit0:
	return false;
}

bool FPhysics::RemoveFromScene(PxRigidDynamic*actor, FScene* scene)
{
	scene->GetPhysicsScene()->removeActor(*actor);
	return true;
}

PxRigidDynamic* FPhysics::CreatePhysicActor(VoroCellInfo& cellInfo, FMaterial& material, PxTransform& tran)
{
	const PxU32 numVertices = cellInfo.Vertices.size();
	const PxU32 numTriangles = cellInfo.Faces.size() / 3;

	PxVec3* vertices = new PxVec3[numVertices];
	PxU32* indices = cellInfo.Faces.data();

	// 加载顶点
	memcpy_s(vertices, sizeof(PxVec3) * numVertices, cellInfo.Vertices.data(), sizeof(FVec3) * numVertices);

	PxConvexMeshDesc meshDesc1;
	meshDesc1.points.count = numVertices;
	meshDesc1.points.data = vertices;
	meshDesc1.points.stride = sizeof(PxVec3);

	meshDesc1.indices.count = numTriangles;
	meshDesc1.indices.data = indices;
	meshDesc1.indices.stride = sizeof(PxU32) * 3;

	meshDesc1.flags = PxConvexFlag::eCOMPUTE_CONVEX;

	PxConvexMesh *convexMesh = m_pCooking->createConvexMesh(meshDesc1,
															m_pPhysics->getPhysicsInsertionCallback());

	PxConvexMeshGeometry geom(convexMesh);
	// 创建网格面
	PxRigidDynamic* convexActor = m_pPhysics->createRigidDynamic(tran);

	// 创建三角网格形状 *gMaterial
	PxShape* shape = m_pPhysics->createShape(geom, *material.material);


	convexActor->attachShape(*shape);

	PxRigidBodyExt::updateMassAndInertia(*convexActor,material.identity);	//convexActor->setMass(1000.0f);
	shape->release();

	convexActor->setAngularDamping(0.1f);
	return convexActor;
}

PxShape* FPhysics::CreateConvexShape(std::vector<FVec3>&Vertices,std::vector<uint32_t>&Indices, FMaterial& material)
{
	const PxU32 numVertices = Vertices.size();
	const PxU32 numTriangles = Indices.size() / 3;

	PxVec3* vertices = new PxVec3[numVertices];
	PxU32* indices = Indices.data();

	// 加载顶点
	memcpy_s(vertices, sizeof(PxVec3) * numVertices, Vertices.data(), sizeof(FVec3) * numVertices);

	PxConvexMeshDesc meshDesc1;
	meshDesc1.points.count = numVertices;
	meshDesc1.points.data = vertices;
	meshDesc1.points.stride = sizeof(PxVec3);

	meshDesc1.indices.count = numTriangles;
	meshDesc1.indices.data = indices;
	meshDesc1.indices.stride = sizeof(PxU32) * 3;

	meshDesc1.flags = PxConvexFlag::eCOMPUTE_CONVEX;

	PxConvexMesh* convexMesh = m_pCooking->createConvexMesh(meshDesc1,
		m_pPhysics->getPhysicsInsertionCallback());

	PxConvexMeshGeometry geom(convexMesh);

	// 创建三角网格形状 *gMaterial
	PxShape* shape = m_pPhysics->createShape(geom, *material.material);
	return shape;
}

bool FPhysics::Update()
{
	for (auto scene : m_SimulatingScenes)
		scene->Update();
	return true;
}

