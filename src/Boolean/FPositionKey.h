#ifndef FPOSITION_KEY_H
#define FPOSITION_KEY_H
#include "vec.h"

class FPositionKey
{
public:
    FPositionKey(const FVec3& v);
    FPositionKey(float x, float y, float z);
    bool operator<(const FPositionKey& right) const;
    bool operator==(const FPositionKey& right) const;

public:
    long m_intX;
    long m_intY;
    long m_intZ;

    static long m_toIntFactor;
};

namespace std {
	template<>
	struct hash<FPositionKey>
	{
		size_t operator ()(const FPositionKey& x) const {
			return hash<long>()(x.m_intX) ^ hash<long>()(x.m_intY) ^ hash < long >()(x.m_intZ);
		}
	};

	template<>
	struct equal_to<FPositionKey> {
		bool operator ()(const FPositionKey& a, const FPositionKey& b)const {
			return a == b;
		}
	};
}

#endif
