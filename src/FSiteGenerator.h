#ifndef FSITE_GENERATOR_H
#define FSITE_GENERATOR_H
#include "vec.h"
#include <vector>
#include <DirectXCollision.h>
enum RandomType
{
	NORMAL,
	GAUSSION,
};
class FSiteGenerator
{
public:
	static void ImpactDamage(FVec3& pos, FVec3& transform, float radius, int num, std::vector<FVec3>&sites,RandomType type=NORMAL);
	static void PlaneImpactDamage(DirectX::BoundingBox& box,FVec3& pos, FVec3& normals, FVec3& transform, int num, std::vector<FVec3>& sites);
private:
	static double _RandomNumber(float min, float max)
	{
		return min + double(rand()) / RAND_MAX * (max - min);
	}
	static double _GaussianRandom(double mu, double sigma)
	{
		const double epsilon = (std::numeric_limits<double>::min)();
		const double two_pi = 2.0 * 3.14159265358979323846;

		static double z0, z1;
		static bool generate;
		generate = !generate;

		if (!generate)
			return z1 * sigma + mu;

		double u1, u2;
		do
		{
			u1 = rand() * (1.0 / RAND_MAX);
			u2 = rand() * (1.0 / RAND_MAX);
		} while (u1 <= epsilon);

		z0 = sqrt(-2.0 * log(u1)) * cos(two_pi * u2);
		z1 = sqrt(-2.0 * log(u1)) * sin(two_pi * u2);
		return z0 * sigma + mu;
	}
};

#endif//FSITE_GENERATOR_H
