#include "vec.h"
#include <cmath>

FVec3::FVec3(const double x, const double y, const double z)
{
	X = x; Y = y; Z = z;
}


// SPECIAL FUNCTIONS

double FVec3::Length() const
{  
	return std::sqrt(SqrLength()); 
}

double FVec3::SqrLength() const
{  
	return X*X+Y*Y+Z*Z; 
}

FVec3& FVec3::Normalize() // it is up to caller to avoid divide-by-zero
{ 
   double len = Length();
   if (len > 0.000001) {
	   X /= len;
	   Y /= len;
	   Z /= len;
   };
   return *this; 
}

FVec3 FVec3::Cross(const FVec3 &v) const
{
	FVec3 tmp;
	tmp.X = Y * v.Z - Z * v.Y;
	tmp.Y = Z * v.X - X * v.Z;
	tmp.Z = X * v.Y - Y * v.X;
	return tmp;
}

