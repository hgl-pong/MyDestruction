#include "FChunk.h"
#include "FPhysics.h"
#include "FActor.h"
#include "FChunkCluster.h"
#include "FSiteGenerator.h"
#include "FChunkCluster.h"
FChunk::FChunk(VoroCellInfo& cellInfo,FActor*actor)
	:m_IsSleeping(true),
	m_IsDestructable(false),
	m_pActor(actor),
	m_Damage(0,0)
{
	m_Center = cellInfo.Position;
	m_Volume = cellInfo.Volume;
	m_Vertices = cellInfo.Vertices;
	m_Normals = cellInfo.Normals;
	m_Indices = cellInfo.Indices;
	m_Neighbors = cellInfo.Neighbors;
	m_Areas = cellInfo.Areas;
	m_Id = cellInfo.Id;
	m_Life = 1000;
	m_ChunkHealth = actor->m_Material.hardness / m_Volume;

	InitPhyiscShape();
	InitBoundingBox();
}

FChunk::FChunk(Geometry::MeshData& meshData, FActor* actor)
	:m_IsSleeping(true),
	m_IsDestructable(false),
	m_pActor(actor),
	m_Damage(0,0),
	m_Id(0)
{


	m_Vertices.resize(meshData.vertices.size());
	m_Indices.resize(meshData.indices32.size());

	memcpy_s(m_Vertices.data(), sizeof(XMFLOAT3) * meshData.vertices.size(), meshData.vertices.data(), sizeof(XMFLOAT3) * meshData.vertices.size());
	memcpy_s(m_Indices.data(), sizeof(uint32_t) * meshData.indices32.size(), meshData.indices32.data(), sizeof(uint32_t) * meshData.indices32.size());

	InitBoundingBox();
	m_Volume = (m_BoundingBox.Extents.x * m_BoundingBox.Extents.x+ m_BoundingBox.Extents.y * m_BoundingBox.Extents.y+ m_BoundingBox.Extents.z * m_BoundingBox.Extents.z)*8;
	m_Life = 1000;
	m_ChunkHealth = actor->m_Material.hardness / (m_Volume*m_Volume);

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

	PxRigidBodyExt::updateMassAndInertia(*m_pRigidActor, m_pActor->m_Material.identity);

	m_pRigidActor->setAngularDamping(0.1f);

	m_pRigidActor->setSleepThreshold((1 - m_pActor->m_Material.material->getRestitution()) / 3);
	m_pRigidActor->attachShape(*m_pConvexMeshShape);
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

bool FChunk::Update()
{
	m_Transform = m_Transform*m_pRigidActor->getGlobalPose();
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

bool FChunk::VoronoiFracture(std::vector<FVec3>& sites, FChunkCluster*& chunkCluster)
{
	if (!m_IsDestructable)
		return false;
	FVoronoi3D::BBox bbox;
	bbox.Max = { m_BoundingBox.Center.x + m_BoundingBox.Extents.x,m_BoundingBox.Center.y + m_BoundingBox.Extents.y ,m_BoundingBox.Center.z + m_BoundingBox.Extents.z };
	bbox.Min = { m_BoundingBox.Center.x - m_BoundingBox.Extents.x,m_BoundingBox.Center.y - m_BoundingBox.Extents.y ,m_BoundingBox.Center.z - m_BoundingBox.Extents.z };
	FVoronoi3D diagram(bbox);
	VoroCellInfo* cellInfo = nullptr;
	std::unordered_map<int,FChunk*> chunks;
	FChunkCluster* newChunkCluster;
	FASSERT(m_ChunkHealth<0);
	FASSERT(!sites.empty());
	diagram.AddSites(sites,0,FVec3(-m_Transform.p.x,-m_Transform.p.y, -m_Transform.p.z));
	diagram.ComputeAllCells();
	cellInfo = diagram.GetAllCells();
	for (int i = 0; i < diagram.Size(); i++) {
		if (cellInfo[i].Vertices.empty())
			continue;
		FChunk* newChunk = new FChunk(cellInfo[i],m_pActor);
		newChunk->m_Transform = m_Transform;
		chunks.emplace(i,newChunk);
		
	}
	if (chunks.size() <2)
		return false;
	newChunkCluster = new FChunkCluster(m_pActor);
	newChunkCluster->Init(chunks, m_pActor,m_Transform);
	m_pActor->RemoveChunk(this);
	m_pActor->m_ChunkClusters.emplace(newChunkCluster);
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
	FChunkCluster* newChunkCluster=nullptr;
	VoronoiFracture(sites,newChunkCluster);
	return false;
}

physx::PxRigidDynamic* FChunk::GetPhysicsActor()
{
	return m_pRigidActor;
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