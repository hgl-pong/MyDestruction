#include "FSiteGenerator.h"
#include <random>
#include <time.h>
void FSiteGenerator::ImpactDamage(FVec3& pos, FVec3&transform,float radius, int num, std::vector<FVec3> &output, RandomType type)
{
	FVec3 min(pos.X - radius, pos.Y - radius, pos.Z - radius);
	FVec3 max(pos.X + radius, pos.Y + radius, pos.Z + radius);
	FVec3 newSite;
	for (int i = 0; i < num; ) {

		switch (type)
		{
		case NORMAL:
			newSite=FVec3(_RandomNumber(min.X, max.X), _RandomNumber(min.Y, max.Y), _RandomNumber(min.Z, max.Z));
		case GAUSSION:
			newSite=FVec3(_GaussianRandom(pos.X, radius / 4), _GaussianRandom(pos.Y, radius / 4), _GaussianRandom(pos.Z, radius / 4));
		}
		
		
		double disq = newSite.DistanceSqr(pos);
		if (disq <= radius * radius) {
			//printf("%f,%f,%f\n", newSite.X, newSite.Y, newSite.Z);
			output.push_back(newSite-transform);
			i++;
		}
	}
}

void FSiteGenerator::PlaneImpactDamage(DirectX::BoundingBox&box,FVec3& pos, FVec3& normals, FVec3& transform, int num, std::vector<FVec3>& sites)
{
	FVec3 min(box.Center.x - box.Extents.x, box.Center.y - box.Extents.y, box.Center.z - box.Extents.z);
	FVec3 max(box.Center.x + box.Extents.x, box.Center.y + box.Extents.y, box.Center.z + box.Extents.z);

	FVec3 newSite;
	for (int i = 0; i < num; ) {

		newSite = FVec3(_RandomNumber(min.X, max.X), _RandomNumber(min.Y, max.Y), _RandomNumber(min.Z, max.Z));

		FVec3 temp = newSite - pos + transform;
		double dot = normals.Dot(temp);
		if (dot <= 0) {
			sites.push_back(newSite);
			i++;
		}

	}
}

