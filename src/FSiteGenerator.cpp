#include "FSiteGenerator.h"
#include <random>
#include <time.h>
void FSiteGenerator::ImpactDamage(FVec3& pos, FVec3&transform,float radius, int num, std::vector<FVec3> &output)
{
	FVec3 min(pos.X - radius, pos.Y - radius, pos.Z - radius);
	FVec3 max(pos.X + radius, pos.Y + radius, pos.Z + radius);

	for (int i = 0; i < num; i++) {
		FVec3 newSite(_RandomNumber(min.X, max.X), _RandomNumber(min.Y, max.Y), _RandomNumber(min.Z, max.Z));
		//FVec3 newSite(generateGaussianNoise(pos.X, 5), generateGaussianNoise(pos.Y, 5), generateGaussianNoise(pos.Z, 5));
		double disq = newSite.DistanceSqr(pos);
		if (disq <= radius * radius) {
			//printf("%f,%f,%f\n", newSite.X, newSite.Y, newSite.Z);
			output.push_back(newSite-transform);
		}
	}
}

