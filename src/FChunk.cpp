#include "FChunk.h"
#include "FPhysics.h"
#include "FActor.h"
#include "FChunkCluster.h"
#include "FSiteGenerator.h"
#include "FChunkCluster.h"
#include "FChunkManager.h"
#include "FMeshBoolean.h"
FChunk::FChunk(VoroCellInfo& cellInfo, FActor* actor)
	:m_IsSleeping(true),
	m_IsDestructable(false),
	m_pActor(actor),
	m_Damage(0, 0)
{
	m_Center = cellInfo.Position;
	m_Volume = cellInfo.Volume;
	m_Vertices = cellInfo.Vertices;
	m_Normals = cellInfo.Normals;
	m_Indices = cellInfo.Indices;
	m_Neighbors = cellInfo.Neighbors;
	m_Faces = cellInfo.Faces;
	m_Areas = cellInfo.Areas;
	m_UVs = cellInfo.UVs;
	m_Id = cellInfo.Id;
	m_Life = 1000;
	m_ChunkHealth = actor->m_Material.hardness / m_Volume;	
	m_Normals2 = m_Normals;
	m_Vertices2 = m_Vertices;
	Update();
	InitPhyiscShape();
	InitBoundingBox();

	m_pMeshBoolean = new FMeshBoolean(this);
}

FChunk::FChunk(Geometry::MeshData& meshData, PxTransform transform, FActor* actor)
	:m_IsSleeping(true),
	m_IsDestructable(false),
	m_pActor(actor),
	m_Damage(0, 0),
	m_Id(0)
{
	m_Vertices.resize(meshData.vertices.size());
	m_Normals.resize(meshData.normals.size());
	m_UVs.resize(meshData.texcoords.size());
	m_Indices.resize(meshData.indices32.size());
	m_Transform = transform;
	memcpy_s(m_Vertices.data(), sizeof(XMFLOAT3) * meshData.vertices.size(), meshData.vertices.data(), sizeof(XMFLOAT3) * meshData.vertices.size());
	memcpy_s(m_Normals.data(), sizeof(XMFLOAT3) * meshData.vertices.size(), meshData.normals.data(), sizeof(XMFLOAT3) * meshData.vertices.size());
	memcpy_s(m_UVs.data(), sizeof(XMFLOAT2) * meshData.vertices.size(), meshData.texcoords.data(), sizeof(XMFLOAT2) * meshData.vertices.size());
	memcpy_s(m_Indices.data(), sizeof(uint32_t) * meshData.indices32.size(), meshData.indices32.data(), sizeof(uint32_t) * meshData.indices32.size());

	InitBoundingBox();
	m_Volume = (m_BoundingBox.Extents.x * m_BoundingBox.Extents.x + m_BoundingBox.Extents.y * m_BoundingBox.Extents.y + m_BoundingBox.Extents.z * m_BoundingBox.Extents.z) * 8;
	m_Life = 1000;
	m_ChunkHealth = actor->m_Material.hardness / (m_Volume * m_Volume);
	m_Vertices2 = m_Vertices;
	m_Normals2 = m_Normals;
	m_Center = FVec3(m_Transform.p.x, m_Transform.p.y, m_Transform.p.z);

	InitPhyiscShape();

	m_pMeshBoolean = new FMeshBoolean(this);
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

	//m_pRigidActor->setSleepThreshold((1 - m_pActor->m_Material.material->getRestitution()) / 3);
	m_pRigidActor->setLinearVelocity(m_Volocity);

	PxFilterData simulationFilterData;
	//simulationFilterData.word3 = ChunkType::UNIQUE;
	//m_pConvexMeshShape->setSimulationFilterData(simulationFilterData);
	FASSERT(m_pConvexMeshShape);
	m_pRigidActor->attachShape(*m_pConvexMeshShape);
	PxRigidBodyExt::updateMassAndInertia(*m_pRigidActor, m_pActor->m_Material.identity);
	return true;
Exit0:
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
	m_Transform = m_pRigidActor->getGlobalPose();
	for (int i = 0; i < m_Vertices.size();i++) {
		PxVec3 temp(m_Vertices2[i].X, m_Vertices2[i].Y, m_Vertices2[i].Z);
		temp = m_Transform.transform(temp);
		
		m_Vertices[i] = FVec3(temp.x, temp.y, temp.z);
	}
	for (int i = 0; i < m_Normals.size(); i++) {
		PxVec3 temp(m_Normals2[i].X, m_Normals2[i].Y, m_Normals2[i].Z);
		temp = m_Transform.rotate(temp);

		m_Normals[i] = FVec3(temp.x, temp.y, temp.z);
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

bool FChunk::VoronoiFracture(FDamage *damage)
{
	if (m_Volume < 0.5)
		return false;
	if (!m_IsDestructable)
		return false;
	FVoronoi3D::BBox bbox;
	bbox.Max = { m_BoundingBox.Center.x + m_BoundingBox.Extents.x,m_BoundingBox.Center.y + m_BoundingBox.Extents.y ,m_BoundingBox.Center.z + m_BoundingBox.Extents.z };
	bbox.Min = { m_BoundingBox.Center.x - m_BoundingBox.Extents.x,m_BoundingBox.Center.y - m_BoundingBox.Extents.y ,m_BoundingBox.Center.z - m_BoundingBox.Extents.z };
	FVoronoi3D diagram(bbox);
	VoroCellInfo* cellInfo = nullptr;
	std::unordered_map<int, FChunk*> chunks;
	FChunkCluster* newChunkCluster;
	FASSERT(!damage->m_Sites.empty());
	diagram.AddSites(damage->m_Sites, 0, FVec3(-m_Transform.p.x, -m_Transform.p.y, -m_Transform.p.z));
	diagram.ComputeAllCells();
	cellInfo = diagram.GetAllCells();
	for (int i = 0; i < diagram.Size(); i++) {
		if (cellInfo[i].Vertices.empty())
			continue;
		VoroCellInfo cell;
		m_pMeshBoolean->FetchBooleanResult(cellInfo[i], cell, INTERSEECTION);
		if (cell.Vertices.empty())
			continue;
		FChunk* newChunk = new FChunk(cell, m_pActor);
		//FChunk* newChunk = new FChunk(cellInfo[i], m_pActor);
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
	for(auto chunk: const_cast<FConnectGraph<FChunk*>*>(newChunkCluster->GetConnectGraph())->GetNodes())
		damage->Damage(chunk);
	newChunkCluster->UpdateClusterHealth(damage);
	return true;
Exit0:
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

bool FChunk::CalculateChunkHealth(FDamage* damage)
{
	m_ChunkHealth -= m_Damage.X;
	
	if (m_ChunkHealth >= 0) {
		m_Damage.X = 0;
		m_Damage.Y = 0;
		return true;
	}
	m_Damage.X = -m_ChunkHealth;
	VoronoiFracture(damage);
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

