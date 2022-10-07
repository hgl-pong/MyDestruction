#ifndef VEC_H
#define VEC_H

class FVec3
{
public:

	double X,Y,Z;

public:
	FVec3() = default;
	FVec3(double x, double y, double z);
	FVec3(const FVec3&);
	~FVec3() = default;
	// Assignment operators
	FVec3& operator	= (const FVec3& v);	    // assignment of a FVec3
	FVec3& operator += (const FVec3& v);	    // incrementation by a FVec3
	FVec3& operator -= (const FVec3& v);	    // decrementation by a FVec3
	FVec3& operator *= (const double d);	    // multiplication by a constant
	FVec3& operator /= (const double d);	    // division by a constant


	// special functions
	double Length() const;				// length of a FVec3
	double SqrLength() const;			// squared length of a FVec3
	FVec3& Normalize();					// normalize a FVec3 in place
	FVec3 Cross(const FVec3& v) const;			// cross product: self cross v

	 FVec3 operator - (const FVec3& v);					// -v1
	 FVec3 operator + (const FVec3&v);	// v1 + v2
	 FVec3 operator * ( const double d);	// v1 * 3.0
	 double operator * (const FVec3&v); // dot product
	 FVec3 operator / (const double d);	// v1 / 3.0
	 FVec3 operator ^ (const FVec3&v);	// cross product
	 int operator == (const FVec3&v);	// v1 == v2 ?
	 FVec3 Prod(const FVec3&v);		    // term by term *
	 double Dot(const FVec3&v);			// dot product
	 double Distance(const FVec3&v);  // distance
	 double DistanceSqr(const FVec3&v);  // distance sqr
};

#endif // VEC_H
