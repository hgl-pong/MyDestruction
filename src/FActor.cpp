#include "FActor.h"
#include "FScene.h"
#include "Utils.h"
#include "FWireMesh.h"
#include "Common/Geometry.h"
#include "Importer/MeshImporter.h"
#include "Common/MaterialManager.h"
#include "renderer/Renderer.h"
#include "FSiteGenerator.h"
#include "FDamage.h"
#include "FPhysics.h"
FActor::FActor()
	:m_pWireMesh(nullptr),
	m_pRenderMesh(nullptr),
	m_pVoroMeshData(nullptr),
	m_pMeshData(nullptr)
{
}

FActor::~FActor()
{
	Release();
}

bool FActor::Init(char*name)
{
	m_Name = name;
	Geometry::MeshData box = Geometry::CreateBox(50, 50, 50);
	m_pMeshData = new Graphics::MeshData();;
	*m_pMeshData = MeshImporter::Get().CreateFromGeometry("box", box)->meshData;
	m_pMeshData->m_Transform.SetPosition(0, 26, 0);
	BoundingBox::CreateFromPoints(m_pMeshData->m_BoundingBox, box.vertices.size(),
		box.vertices.data(), sizeof(XMFLOAT3));

	m_pWireMesh = new FWireMesh(m_pMeshData->m_BoundingBox);
	if (m_pWireMesh->LoadMeshData()) {
		m_pVoroMeshData = Graphics::Renderer::Get()->CreateVoroMeshData(m_pWireMesh);
		m_pVoroMeshData->m_Transform = m_pMeshData->m_Transform;
		m_pVoroMeshData->m_BoundingBox = m_pMeshData->m_BoundingBox;
		Graphics::Renderer::Get()->AddVoroMesh(m_pVoroMeshData);
	}
	else {
		m_pVoroMeshData = Graphics::Renderer::Get()->createBoundingBoxMesh(&m_pMeshData->m_BoundingBox);
		m_pVoroMeshData->m_Transform = m_pMeshData->m_Transform;
		m_pVoroMeshData->m_BoundingBox = m_pMeshData->m_BoundingBox;
		Graphics::Renderer::Get()->AddVoroMesh(m_pVoroMeshData);
	}
	return true;
}

bool FActor::Release()
{
	FRELEASE(m_pWireMesh);

	return true;
}

bool FActor::ReInit()
{
	return false;
}

bool FActor::AddActor()
{
	return false;
}

bool FActor::RemoveActor()
{
	return false;
}

bool FActor::RemoveAllActors()
{
	return false;
}

bool FActor::Update()
{
	return false;
}

bool FActor::OnEnterScene(FScene* scene)
{
	scene->AddActor(this);
	return true;
}

bool FActor::OnLeaveScene(FScene* scene)
{
	scene->RemoveActor(this);
	return true;
}

bool FActor::Intersection(Ray& ray,FScene*scene)
{
	float dis = 0; 
	//FVec3 HitPoint;
	DirectX::BoundingBox box = m_pMeshData->m_BoundingBox;
	box.Center = { box.Center.x + m_pMeshData->m_Transform.GetPosition().x,
		box.Center.y + m_pMeshData->m_Transform.GetPosition().y,
		box.Center.z + m_pMeshData->m_Transform.GetPosition().z };

	//bool hit = Hit(ray,box, HitPoint);
	bool hit = ray.Hit(box, &dis);
	FVec3 HitPoint(ray.origin.x + dis * ray.direction.x,
		ray.origin.y + dis * ray.direction.y,
		ray.origin.z + dis * ray.direction.z);
	if (!hit)
		return hit;
	FSphereDamage damage;
	damage.center = HitPoint;
	damage.radius = 20;

	std::vector<FVec3>sites;
	FVec3 transform(m_pMeshData->m_Transform.GetPosition().x,
		m_pMeshData->m_Transform.GetPosition().y,
		m_pMeshData->m_Transform.GetPosition().z);
	FSiteGenerator::ImpactDamage(damage.center,transform , damage.radius, 500, sites,RandomType::GAUSSION);
	//FVec3 normal(ray.direction.x, ray.direction.y, ray.direction.z);
	//FSiteGenerator::PlaneImpactDamage(m_pMeshData->m_BoundingBox,damage.center,normal, transform ,500, sites);
	Graphics::MeshData* newMesh;

	Transform trans;
	trans.SetPosition(0, 100, 0);
	trans.Translate(XMFLOAT3(0, 0, 1), 100 * m_HitTime);
	m_HitTime++;

	FASSERT(m_pWireMesh->VoronoiFracture(sites));
	m_pWireMesh->LoadMeshData();
	

	m_pWireMesh->CreaePhysicsActor(trans);

	FASSERT(!m_pWireMesh->vertices.empty());

	newMesh=Graphics::Renderer::Get()->CreateVoroMeshData(m_pWireMesh);
	*m_pVoroMeshData = *newMesh;
	delete newMesh;
	m_pVoroMeshData->m_Transform = m_pMeshData->m_Transform;
	m_pVoroMeshData->m_BoundingBox = m_pMeshData->m_BoundingBox;

	Graphics::Renderer::Get()->createHitPosSphere(damage.center,damage.radius);
	//Graphics::Renderer::Get()->m_pHitPos->m_Transform = m_pMeshData->m_Transform;
	Graphics::Renderer::Get()->AddVoroMesh(Graphics::Renderer::Get()->m_pHitPos);

	m_pWireMesh->AddToScene(scene);
	return hit;
Exit0:
	return hit;
}

bool FActor::SetRenderWireFrame()
{
	return false;
}
