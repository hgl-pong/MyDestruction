#include "FSiteGenerator.h"
#include <random>
#include <time.h>
void FSiteGenerator::ImpactDamage(FVec3& pos, FVec3&transform,float radius, int num, std::vector<FVec3> &output, RandomType type)
{
	FVec3 min(pos.X - radius, pos.Y - radius, pos.Z - radius);
	FVec3 max(pos.X + radius, pos.Y + radius, pos.Z + radius);
	FVec3 newSite;
	for (int i = 0; i < num; i++) {

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
		}
	}
}

