#include "FWireMesh.h"
#include "Utils.h"
#include "FPhysics.h"
FWireMesh::FWireMesh(BoundingBox &box)
	: m_pVoronoi3D(nullptr),
	  m_pCellInfo(nullptr),
	  m_Box(box)
{
	FVoronoi3D::BBox bbox;
	bbox.Max = {box.Center.x + box.Extents.x, box.Center.y + box.Extents.y, box.Center.z + box.Extents.z};
	bbox.Min = {box.Center.x - box.Extents.x, box.Center.y - box.Extents.y, box.Center.z - box.Extents.z};
	m_pVoronoi3D = new FVoronoi3D(bbox);

	// m_pVoronoi3D->AddSites(100);
	// m_pVoronoi3D->ComputeCellEdges();
	// m_pVoronoi3D->ComputeCellEdgesSerial();
	// m_pVoronoi3D->ComputeAllCells();
	// m_pCellInfo = m_pVoronoi3D->GetAllCells();
}

FWireMesh::~FWireMesh()
{
	Release();
}

bool FWireMesh::LoadMeshData()
{

	Graphics::MeshData *newMesh;
	vertices.clear();
	colors.clear();
	indices.clear();
	uint32_t vCount = 0;

	if (!m_pVoronoi3D->Size())
	{
		return false;
	}
	FASSERT(m_pCellInfo);
	newMesh = new Graphics::MeshData();

	for (int i = 0; i < m_pVoronoi3D->Size(); i++)
	{
		for (auto pos : m_pCellInfo[i].Vertices)
		{
			vertices.push_back({(float)pos.X, (float)pos.Y, (float)pos.Z});
			colors.push_back({1.0f, 0.5f, 0.5f, 1.0f});
		}
		for (auto edge : m_pCellInfo[i].Edges)
		{
			indices.push_back(edge.s + vCount);
			indices.push_back(edge.e + vCount);
		}
		vCount += m_pCellInfo[i].Vertices.size();
	}

	return true;
Exit0:
	return false;
}

bool FWireMesh::Release()
{
	FDELETE(m_pVoronoi3D);
	FDELETE(m_pCellInfo);
	vertices.clear();
	colors.clear();
	indices.clear();
	return true;
}

bool FWireMesh::VoronoiFracture(std::vector<FVec3> &sites)
{
	m_pVoronoi3D->Clear();
	m_pCellInfo = nullptr;
	FASSERT(!sites.empty());
	m_pVoronoi3D->AddSites(sites);
	m_pVoronoi3D->ComputeAllCells();
	m_pCellInfo = m_pVoronoi3D->GetAllCells();
	return true;
Exit0:
	return false;
}

bool FWireMesh::AddToScene(FScene *scene)
{
	for (int i = 0; i < m_pVoronoi3D->Size(); i++)
	{
		PxRigidDynamic *actor = m_pCellInfo[i].rigidDynamic;
		if (!actor)
			continue;
		actor->setSleepThreshold(0.2);
		FPhysics::Get()->AddToScene(actor, scene);
	}
	return false;
}

bool FWireMesh::CreaePhysicsActor(Transform &trans)
{
	if (!m_pVoronoi3D->Size())
	{
		return false;
	}

	PxVec3 pos(trans.GetPosition().x, trans.GetPosition().y, trans.GetPosition().z);
	PxTransform tran(pos);

	//PxRigidDynamic *m_pRigidActor = FPhysics::Get()->m_pPhysics->createRigidDynamic(tran);

	//PxRigidBodyExt::updateMassAndInertia(*m_pRigidActor, 1000);

	//m_pRigidActor->setAngularDamping(0.1f);
	for (int i = 0; i < m_pVoronoi3D->Size(); i++)
	{
		if (!m_pCellInfo[i].Vertices.empty())
		{
			//PxShape *newshape = FPhysics::Get()->CreateConvexShape(m_pCellInfo[i].Vertices, m_pCellInfo[i].Faces, FPhysics::Get()->STONE);
			//m_pRigidActor->attachShape(*newshape);
			//m_pCellInfo[i].rigidDynamic = m_pRigidActor;
			m_pCellInfo[i].rigidDynamic = FPhysics::Get()->CreatePhysicActor(m_pCellInfo[i], FPhysics::Get()->STONE, tran);

		}
	}
	return true;
}
