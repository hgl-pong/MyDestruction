#ifndef FDAMAGE_H
#define FDAMAGE_H
#include "vec.h"
#include "Common/Collision.h"
#include "FVoronoi3D.h"
#include "FPhysics.h"
#include "FSiteGenerator.h"
#define FPI 3.1415926
class FChunk;
class FDamage
{
public:
	FDamage() = default;
	virtual ~FDamage(){};
	virtual void Intersection(FChunk *chunk) = 0;
	virtual void ResetSites()
	{
		m_Sites.clear();
	}
	virtual void GenerateSites(FMaterial &material, RandomType type) = 0;
	virtual std::set<FChunk*> GetDamagingChunks()
	{
		return m_DamagingChunks;
	};

public:
	FVec3 m_Position;
	std::set<FChunk*> m_DamagingChunks;
	float m_Damage;
	std::vector<FVec3> m_Sites;
};

class FSphereDamage : public FDamage
{
public:
	FSphereDamage(FVec3 &pos, float radius, float damage);
	~FSphereDamage();
	void Intersection(FChunk *chunk) override;
	void GenerateSites(FMaterial &material, RandomType type) override;

public:
	float m_Radius;
};

#endif // FDAMAGE_H
