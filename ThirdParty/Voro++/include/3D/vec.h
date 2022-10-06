#ifndef VEC_H
#define VEC_H

class FVec3
{
public:

	double X,Y,Z;

public:
	FVec3() = default;
	FVec3( double x,  double y,  double z);
	~FVec3() = default;
	// special functions
	double Length() const;				// length of a vec3
	double SqrLength() const;			// squared length of a vec3
	FVec3& Normalize();					// normalize a vec3 in place
	FVec3 Cross(const FVec3 &v) const;			// cross product: self cross v
};

#endif // VEC_H
