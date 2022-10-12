#ifndef VEC_H
#define VEC_H
struct FVec2
{
	float X, Y;
	FVec2() = default;
	FVec2(float x, float y):X(x),Y(y) {

	}
};
struct FVec4
{
	float X, Y, Z, W;
};
class FVec3
{
public:

	float X,Y,Z;

public:
	FVec3() = default;
	FVec3(float x, float y, float z);
	FVec3(const FVec3&);
	~FVec3() = default;
	// Assignment operators
	FVec3& operator	= (const FVec3& v);	    // assignment of a FVec3
	FVec3& operator += (const FVec3& v);	    // incrementation by a FVec3
	FVec3& operator -= (const FVec3& v);	    // decrementation by a FVec3
	FVec3& operator *= (const float d);	    // multiplication by a constant
	FVec3& operator /= (const float d);	    // division by a constant


	// special functions
	float Length() const;				// length of a FVec3
	float SqrLength() const;			// squared length of a FVec3
	FVec3& Normalize();					// normalize a FVec3 in place
	FVec3 Cross(const FVec3& v) const;			// cross product: self cross v

	 FVec3 operator - (const FVec3& v);					// -v1
	 FVec3 operator + (const FVec3&v);	// v1 + v2
	 FVec3 operator * ( const float d);	// v1 * 3.0
	 float operator * (const FVec3&v); // dot product
	 FVec3 operator / (const float d);	// v1 / 3.0
	 FVec3 operator ^ (const FVec3&v);	// cross product
	 int operator == (const FVec3&v);	// v1 == v2 ?
	 FVec3 Prod(const FVec3&v);		    // term by term *
	 float Dot(const FVec3&v);			// dot product
	 float Distance(const FVec3&v);  // distance
	 float DistanceSqr(const FVec3&v);  // distance sqr
};

#endif // VEC_H
