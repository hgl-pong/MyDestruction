#include "FDamage.h"
#include <DirectXCollision.h>
#include "FChunk.h"
#include "renderer\Renderer.h"
#include "UI\UI.h"
FSphereDamage::FSphereDamage(FVec3& pos, float radius, float damage) {
	m_Position = pos;
	m_Radius = radius;
	m_Damage = damage;
	m_Shape = new PxSphereGeometry(radius);

	Graphics::Renderer::Get()->createHitPosSphere(m_Position, m_Radius);
	Graphics::Renderer::Get()->AddVoroMesh(Graphics::Renderer::Get()->m_pHitPos);
}

FSphereDamage::~FSphereDamage() {

}

void FSphereDamage::Intersection(FChunk* chunk)
{
	if (m_DamagingChunks.find(chunk) != m_DamagingChunks.end())
		return;
	DirectX::BoundingSphere sphere;

	DirectX::BoundingBox box=chunk->GetBoundingBox();
	box.Center = { box.Center.x+chunk->m_Transform.p.x,box.Center.y + chunk->m_Transform.p.y,box.Center.z + chunk->m_Transform.p.z };
	sphere.Center = DirectX::XMFLOAT3(m_Position.X, m_Position.Y, m_Position.Z);
	sphere.Radius = m_Radius;
	if (box.Intersects(sphere)) {
		Damage(chunk);
		m_DamagingChunks.emplace(chunk);
	}
}

void FSphereDamage::Damage(FChunk* chunk) {
	PxVec3 dir(chunk->GetPosition().X - m_Position.X, chunk->GetPosition().Y - m_Position.Y, chunk->GetPosition().Z - m_Position.Z);
	float dis = dir.magnitude();
	if (chunk->m_Neighbors.empty())
		chunk->m_Damage.X = m_Damage / (dis * dis);
	else
		chunk->m_Damage.X = m_Damage / (dis * dis) / chunk->m_Neighbors.size();

	dir.normalize();
	dir = dir * m_Damage / (dis * dis);
	//dir = dir * 1000;
	chunk->GetPhysicsActor()->addForce(dir, PxForceMode::eIMPULSE);

}

void FSphereDamage::Intersection(FChunkCluster* chunkCluster) 
{

}

void FSphereDamage::GenerateSites(FMaterial& material, RandomType type) {
	int sitecount = 8 * FPI * m_Radius * m_Radius * m_Radius * m_Damage * material.hardness;
	if (sitecount > UIDrawer::Get()->m_MaxSiteCount)
		sitecount = UIDrawer::Get()->m_MaxSiteCount;
	FSiteGenerator::ImpactSphereDamage(m_Position, m_Radius, sitecount, m_Sites, type);
};
