#ifndef FACTTACK_H
#define FACTTACK_H
#include "PxPhysicsAPI.h"

enum FAttackType
{
	SPHERE = (0 << 0),
	CUBE = (0 << 1),
	LINE = (0 << 2),
	PLANE = (0 << 3),
};

class FAttack {
public:

private:
	FAttackType m_AttackType;
	float m_impulse;
};

#endif//FACTTACK_H
