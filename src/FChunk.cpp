#include "FChunk.h"
#include "FPhysics.h"
#include "FActor.h"
#include "FChunkCluster.h"
#include "FSiteGenerator.h"
#include "FChunkCluster.h"
#include "FChunkManager.h"
FChunk::FChunk(VoroCellInfo& cellInfo, FActor* actor)
	:m_IsSleeping(true),
	m_IsDestructable(false),
	m_pActor(actor),
	m_Damage(0, 0)
{
	m_Center = cellInfo.Position;
	m_Volume = cellInfo.Volume;
	m_Vertices = cellInfo.Vertices;
	//m_Normals = cellInfo.Normals;
	m_Indices = cellInfo.Indices;
	m_Neighbors = cellInfo.Neighbors;
	m_Faces = cellInfo.Faces;
	m_Areas = cellInfo.Areas;
	m_Id = cellInfo.Id;
	m_Life = 1000;
	m_ChunkHealth = actor->m_Material.hardness / m_Volume;	
	m_Vertices2 = m_Vertices;
	Update();
	_CalculateNormals();
	InitPhyiscShape();
	InitBoundingBox();

}

FChunk::FChunk(Geometry::MeshData& meshData, FActor* actor)
	:m_IsSleeping(true),
	m_IsDestructable(false),
	m_pActor(actor),
	m_Damage(0, 0),
	m_Id(0)
{


	m_Vertices.resize(meshData.vertices.size());
	m_Normals.resize(meshData.normals.size());
	m_Indices.resize(meshData.indices32.size());

	memcpy_s(m_Vertices.data(), sizeof(XMFLOAT3) * meshData.vertices.size(), meshData.vertices.data(), sizeof(XMFLOAT3) * meshData.vertices.size());
	memcpy_s(m_Normals.data(), sizeof(XMFLOAT3) * meshData.vertices.size(), meshData.normals.data(), sizeof(XMFLOAT3) * meshData.vertices.size());
	memcpy_s(m_Indices.data(), sizeof(uint32_t) * meshData.indices32.size(), meshData.indices32.data(), sizeof(uint32_t) * meshData.indices32.size());

	InitBoundingBox();
	m_Volume = (m_BoundingBox.Extents.x * m_BoundingBox.Extents.x + m_BoundingBox.Extents.y * m_BoundingBox.Extents.y + m_BoundingBox.Extents.z * m_BoundingBox.Extents.z) * 8;
	m_Life = 1000;
	m_ChunkHealth = actor->m_Material.hardness / (m_Volume * m_Volume);
	m_Vertices2 = m_Vertices;

	InitPhyiscShape();
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



	m_pRigidActor->setAngularDamping(0.1f);

	m_pRigidActor->setSleepThreshold((1 - m_pActor->m_Material.material->getRestitution()) / 3);
	m_pRigidActor->setLinearVelocity(m_Volocity);

	PxFilterData simulationFilterData;
	//simulationFilterData.word3 = ChunkType::UNIQUE;
	//m_pConvexMeshShape->setSimulationFilterData(simulationFilterData);
	m_pRigidActor->attachShape(*m_pConvexMeshShape);
	PxRigidBodyExt::updateMassAndInertia(*m_pRigidActor, m_pActor->m_Material.identity);
	return false;
}

bool FChunk::Attach(PxRigidDynamic* actor)
{
	//PxFilterData simulationFilterData;
	FASSERT(actor);
	//simulationFilterData.word3 = ChunkType::INCLUSTER;
	//m_pConvexMeshShape->setSimulationFilterData(simulationFilterData);
	actor->attachShape(*m_pConvexMeshShape);
	return true;
Exit0:
	return false;
}

bool FChunk::Update()
{
	FASSERT(m_pRigidActor);
	m_IsSleeping = m_pRigidActor->isSleeping();
	FASSERT(!m_IsSleeping);
	for (int i = 0; i < m_Vertices.size();i++) {
		PxVec3 temp(m_Vertices2[i].X, m_Vertices2[i].Y, m_Vertices2[i].Z);
		temp = m_pRigidActor->getGlobalPose().transform(temp);
		
		m_Vertices[i] = FVec3(temp.x, temp.y, temp.z);
	}
	if (!m_IsDestructable)
		return false;
	//if (m_Life==0) {
	//	m_pActor->RemoveChunk(this);
	//	return false;
	//}
	//m_Life--;
	return true;
Exit0:
	return false;
}

bool FChunk::IsDestructable()
{
	return m_IsDestructable;
}

bool FChunk::VoronoiFracture(std::vector<FVec3>& sites, FChunkCluster*& chunkCluster)
{
	if (!m_IsDestructable)
		return false;
	FVoronoi3D::BBox bbox;
	bbox.Max = { m_BoundingBox.Center.x + m_BoundingBox.Extents.x,m_BoundingBox.Center.y + m_BoundingBox.Extents.y ,m_BoundingBox.Center.z + m_BoundingBox.Extents.z };
	bbox.Min = { m_BoundingBox.Center.x - m_BoundingBox.Extents.x,m_BoundingBox.Center.y - m_BoundingBox.Extents.y ,m_BoundingBox.Center.z - m_BoundingBox.Extents.z };
	FVoronoi3D diagram(bbox);
	VoroCellInfo* cellInfo = nullptr;
	std::unordered_map<int, FChunk*> chunks;
	FChunkCluster* newChunkCluster;
	FASSERT(m_ChunkHealth < 0);
	FASSERT(!sites.empty());
	diagram.AddSites(sites, 0, FVec3(-m_Transform.p.x, -m_Transform.p.y, -m_Transform.p.z));
	diagram.ComputeAllCells();
	cellInfo = diagram.GetAllCells();
	for (int i = 0; i < diagram.Size(); i++) {
		if (cellInfo[i].Vertices.empty())
			continue;
		FChunk* newChunk = new FChunk(cellInfo[i], m_pActor);
		newChunk->m_Transform = m_Transform;
		chunks.emplace(i, newChunk);
	}
	if (chunks.size() < 2)
		return false;
	newChunkCluster = new FChunkCluster(m_pActor);
	newChunkCluster->Init(chunks, m_pActor, m_Transform);
	m_pActor->RemoveChunk(this);
	m_pActor->m_pChunkManager->Insert(newChunkCluster);
	m_pActor->AddPhysicsActorToScene(newChunkCluster->GetPhysicsActor());
	chunkCluster = newChunkCluster;
	return true;
Exit0:
	return false;
}

bool FChunk::Intersection(Ray& ray)
{
	return false;
}

bool FChunk::Intersection(FDamage* damage)
{
	BoundingSphere sphere;
	sphere.Center = XMFLOAT3();
	sphere.Radius = 10;

	m_BoundingBox.Intersects(sphere);

	std::vector<FVec3>sites;

	//FSiteGenerator::ImpactDamage(damage.center, transform, damage.radius, 100, sites, RandomType::GAUSSION);
	m_pActor->RemoveChunk(this);
	FChunkCluster* newChunkCluster = nullptr;
	VoronoiFracture(sites, newChunkCluster);
	return false;
}

physx::PxRigidDynamic* FChunk::GetPhysicsActor()
{
	return m_pRigidActor;
}

physx::PxShape* FChunk::GetPhysicsShape()
{
	return m_pConvexMeshShape;
}

bool FChunk::CostChunkHealth(float damage)
{
	m_ChunkHealth -= damage;
	if (m_ChunkHealth >= 0)
		return true;
	//std::vector<FVec3>sites;

	////FSiteGenerator::ImpactDamage(damage.center, transform, damage.radius, 100, sites, RandomType::GAUSSION);
	//m_pActor->RemoveChunk(this);
	//VoronoiFracture(sites);

	return true;
}

bool FChunk::InitPhyiscShape()
{
	m_pConvexMeshShape = FPhysics::Get()->CreateConvexShape(m_Vertices, m_Indices, m_pActor->m_Material);

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

void FChunk::_CalculateNormals() {
	m_Normals = std::vector<FVec3>(m_Vertices.size(), FVec3(0, 0, 0));
	for (int i = 0; i < m_Indices.size() / 3-1; i++) {
		uint32_t p0, p1, p2;
		p0 = m_Indices[3 * i];
		p1 = m_Indices[3 * i+1];
		p2 = m_Indices[3 * i+2];
		FVec3 v01 = m_Vertices[p1] - m_Vertices[p0];
		FVec3 v02 = m_Vertices[p2] - m_Vertices[p0];
		FVec3 normal = v01.Cross(v02);
		normal/*.Normalize()*/;
		m_Normals[p0] = m_Normals[p0] + normal;
		m_Normals[p1] = m_Normals[p1] + normal;
		m_Normals[p2] = m_Normals[p2] + normal;
	}
}