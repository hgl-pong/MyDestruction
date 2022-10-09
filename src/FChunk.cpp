#include "FChunk.h"
#include "FPhysics.h"
FChunk::FChunk(VoronoiCellInfo& cellInfo,PxMaterial*material)
	:m_IsSleeping(true),
	m_IsDestructable(false),
	m_Life(1000)
{
	m_Center = cellInfo.Position;
	m_Volume = cellInfo.Volume;
	m_Vertices = cellInfo.Vertices;
	m_Normals = cellInfo.Normals;
	m_Indices = cellInfo.Faces;
	m_Neighbors = cellInfo.Neighbors;
	m_Areas = cellInfo.Areas;
	
	InitPhyiscShape(material);
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

bool FChunk::InitPhysicsActor(float identity)
{
	m_pRigidActor = FPhysics::Get()->m_pPhysics->createRigidDynamic(m_Transform);

	PxRigidBodyExt::updateMassAndInertia(*m_pRigidActor, identity);

	m_pRigidActor->setAngularDamping(0.1f);
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
		m_NeedToBeRemove = true;
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
	return false;
}

physx::PxRigidDynamic* FChunk::GetPhysicsActor()
{
	return m_pRigidActor;
}

bool FChunk::InitPhyiscShape(PxMaterial* material)
{
	m_pConvexMeshShape = FPhysics::Get()->CreateConvexShape(m_Vertices,m_Indices,material);
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