#include "FPositionKey.h"

//long FPositionKey::m_toIntFactor = 10000;

FPositionKey::FPositionKey(const FVec3& v) 
    :FPositionKey(v.X, v.Y, v.Z)
{
}

FPositionKey::FPositionKey(float x, float y, float z)
{
    m_intX = (long)(x * EPSILON);
    m_intY = (long)(y * EPSILON);
    m_intZ = (long)(z * EPSILON);
}

bool FPositionKey::operator<(const FPositionKey& right) const
{
    if (m_intX < right.m_intX)
        return true;
    if (m_intX > right.m_intX)
        return false;
    if (m_intY < right.m_intY)
        return true;
    if (m_intY > right.m_intY)
        return false;
    if (m_intZ < right.m_intZ)
        return true;
    if (m_intZ > right.m_intZ)
        return false;
    return false;
}

bool FPositionKey::operator==(const FPositionKey& right) const
{
    return m_intX == right.m_intX &&
        m_intY == right.m_intY &&
        m_intZ == right.m_intZ;
}
