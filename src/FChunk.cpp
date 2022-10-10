#include "FChunk.h"
#include "FPhysics.h"
#include "FActor.h"
FChunk::FChunk(VoroCellInfo& cellInfo,FActor*actor)
	:m_IsSleeping(true),
	m_IsDestructable(false),
	m_pActor(actor)
{
	m_Center = cellInfo.Position;
	m_Volume = cellInfo.Volume;
	m_Vertices = cellInfo.Vertices;
	m_Normals = cellInfo.Normals;
	m_Indices = cellInfo.Indices;
	m_Neighbors = cellInfo.Neighbors;
	m_Areas = cellInfo.Areas;

	m_Life = 1000;
	m_ChunkHealth = actor->m_Material.hardness / m_Volume;

	InitPhyiscShape();
	InitBoundingBox();
}

FChunk::~FChunk()
{
	Release();
}

bool FChunk::Release()
{
	PHYSX_RELEASE(m_pConvexMeshShape);
	PHYSX_RELEASE(m_pRigidActor);
	return false;
}

bool FChunk::InitUniquePhysicsActor()
{
	m_pRigidActor = FPhysics::Get()->m_pPhysics->createRigidDynamic(m_Transform);

	PxRigidBodyExt::updateMassAndInertia(*m_pRigidActor, m_pActor->m_Material.identity);

	m_pRigidActor->setAngularDamping(0.1f);

	m_pRigidActor->setSleepThreshold((1 - m_pActor->m_Material.material->getRestitution()) / 3);
	return false;
}

bool FChunk::Attach(PxRigidDynamic* actor)
{
	FASSERT(actor);
	actor->attachShape(*m_pConvexMeshShape);
	return true;
Exit0:
	return false;
}

bool FChunk::Tick()
{
	if (!m_IsDestructable)
		return false;
	if (m_Life==0) {
		m_pActor->RemoveChunk(this);
		return false;
	}
	m_Life--;
	return true;
}

bool FChunk::IsDestructable()
{
	return m_IsDestructable;
}

bool FChunk::Intersection(Ray& ray)
{
	return false;
}

bool FChunk::Intersection(FDamage& damage)
{
	BoundingSphere sphere;
	sphere.Center = XMFLOAT3();
	sphere.Radius = 10;

	m_BoundingBox.Intersects(sphere);
	return false;
}

physx::PxRigidDynamic* FChunk::GetPhysicsActor()
{
	return m_pRigidActor;
}

bool FChunk::InitPhyiscShape()
{
	m_pConvexMeshShape = FPhysics::Get()->CreateConvexShape(m_Vertices,m_Indices,m_pActor->m_Material);
	FASSERT(m_pConvexMeshShape);
	return true;
Exit0:
	return false;
}

bool FChunk::InitBoundingBox()
{
	BoundingBox::CreateFromPoints(m_BoundingBox, m_Vertices.size(),
		(XMFLOAT3*)m_Vertices.data(), sizeof(XMFLOAT3));
	return true;
}

void FChunk::_CalculateNormals(std::vector<Edge>& edges, std::vector<FVec3>& normals) {

}