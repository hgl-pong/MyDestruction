#include "FDamage.h"
#include <DirectXCollision.h>
#include "FChunk.h"
#include "renderer\Renderer.h"
#include "UI\UI.h"
#include "FChunkCluster.h"
#include "FConnectGraph.h"
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

void FSphereDamage::Damage(FChunk* chunk, bool cmpdis) {
	if (chunk)
		return;
	PxVec3 temp(chunk->GetPosition().X, chunk->GetPosition().Y, chunk->GetPosition().Z);
	temp= chunk->m_Transform.transform(temp);
	PxVec3 dir(temp.x- m_Position.X, temp.y - m_Position.Y, temp.z- m_Position.Z);
	float dis = dir.magnitude();	

	if (dis > m_Radius&&cmpdis)
		return;
	if (chunk->m_Neighbors.empty())
		chunk->m_Damage.X = m_Damage / (dis * dis);
	else
		chunk->m_Damage.X = m_Damage / (dis * dis) / chunk->m_Neighbors.size();

	dir.normalize();
	dir = dir * m_Damage / (dis * dis)*5;
	//dir = dir * 50;
	chunk->GetPhysicsActor()->addForce(dir, PxForceMode::eIMPULSE);

}

void FSphereDamage::Damage(FChunkCluster* chunkCluster) 
{
	for (auto chunk : const_cast<FConnectGraph<FChunk*>*>(chunkCluster->GetConnectGraph())->GetNodes()) {
		Damage(chunk);
	}
}

void FSphereDamage::GenerateSites(FMaterial& material, RandomType type) {
	int sitecount = 8 * FPI * m_Radius * m_Radius * m_Radius * m_Damage * material.hardness;
	if (sitecount > UIDrawer::Get()->m_MaxSiteCount)
		sitecount = UIDrawer::Get()->m_MaxSiteCount;
	FSiteGenerator::ImpactSphereDamage(m_Position, m_Radius, sitecount, m_Sites, type);
};
