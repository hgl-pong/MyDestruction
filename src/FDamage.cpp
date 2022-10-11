#include "FDamage.h"
#include <DirectXCollision.h>
#include "FChunk.h"
FSphereDamage::FSphereDamage(FVec3& pos, float radius, float damage) {
	m_Position = pos;
	m_Radius = radius;
	m_Damage = damage;
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
		float dis = chunk->GetPosition().Distance(m_Position);
		if(chunk->m_Neighbors.empty())
			chunk->m_Damage.X = m_Damage / (dis * dis);
		else
			chunk->m_Damage.X = m_Damage / (dis * dis)/chunk->m_Neighbors.size();
		m_DamagingChunks.emplace(chunk);
	}
}

void FSphereDamage::GenerateSites(FMaterial& material, RandomType type) {
	int sitecount = 10 * 4 * FPI * m_Radius * m_Radius * m_Radius * m_Damage / material.hardness;
	FSiteGenerator::ImpactSphereDamage(m_Position, m_Radius, sitecount, m_Sites, type);
};
