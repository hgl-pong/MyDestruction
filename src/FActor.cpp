#include "FActor.h"
#include "FScene.h"
#include "Utils.h"
#include "FWireMesh.h"
#include "Common/Geometry.h"
#include "Importer/MeshImporter.h"
#include "Common/MaterialManager.h"
#include "renderer/Renderer.h"
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
	m_pWireMesh->LoadMeshData();
	m_pVoroMeshData = Graphics::Renderer::Get()->CreateVoroMeshData(m_pWireMesh);
	m_pVoroMeshData->m_Transform = m_pMeshData->m_Transform;
	m_pVoroMeshData->m_BoundingBox = m_pMeshData->m_BoundingBox;
	Graphics::Renderer::Get()->AddVoroMesh(m_pVoroMeshData);
	//Graphics::MeshData* test = Graphics::Renderer::Get()->createBoundingBoxMesh(&m_pMeshData->m_BoundingBox);
	//test->m_Transform.SetPosition(0, 26, 0);
	//Graphics::Renderer::Get()->AddVoroMesh(test);
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

bool FActor::SetRenderWireFrame()
{
	return false;
}
