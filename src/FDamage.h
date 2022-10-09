#pragma once
#include "vec.h"
class FDamage
{

};

class FSphereDamage {
public:
	FSphereDamage()=default;

	FVec3 center;
	float radius;

	float damage;
};

