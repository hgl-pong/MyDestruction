#include "vec.h"
#include <cmath>
#include <algorithm>

bool FVec2::IsInPolygon(const std::vector<FVec2>& polygon) {
	bool inside = false;
	int i, j;
	for (i = 0, j = polygon.size() - 1; i < polygon.size(); j = i++) {
		if (((polygon[i].Y > Y) != (polygon[j].Y > Y)) &&
			(X < (polygon[j].X - polygon[i].X) * (Y - polygon[i].Y) / (polygon[j].Y - polygon[i].Y) + polygon[i].X)) {
			inside = !inside;
		}
	}
	return inside;
}

bool FVec2::IsInPolygon(const std::vector<FVec2>& polygonVertices, const std::vector<uint32_t>& polygonIndices)
{
	bool inside = false;
	int i, j;
	for (i = 0, j = polygonIndices.size() - 1; i < polygonIndices.size(); j = i++) {
		if (((polygonVertices[polygonIndices[i]].Y > Y) != (polygonVertices[polygonIndices[j]].Y > Y)) &&
			(X < (polygonVertices[polygonIndices[j]].X - polygonVertices[polygonIndices[i]].X) * (Y - polygonVertices[polygonIndices[i]].Y) / (polygonVertices[polygonIndices[j]].Y - polygonVertices[polygonIndices[i]].Y) + polygonVertices[polygonIndices[i]].X)) {
			inside = !inside;
		}
	}
	return inside;
}

FVec2 BarycentricCoordinates( FVec2& a,  FVec2& b,  FVec2& c,  FVec2& point)
{
	auto v0 = c - a;
	auto v1 = b - a;
	auto v2 = point - a;

	auto dot00 = v0.Dot(v0);
	auto dot01 = v0.Dot(v1);
	auto dot02 = v0.Dot(v2);
	auto dot11 = v1.Dot(v1);
	auto dot12 = v1.Dot(v2);

	auto invDenom = 1.0 / (dot00 * dot11 - dot01 * dot01);
	auto alpha = (dot11 * dot02 - dot01 * dot12) * invDenom;
	auto beta = (dot00 * dot12 - dot01 * dot02) * invDenom;

	return FVec2(alpha, beta);
}

bool FVec2::IsInTriangle(FVec2& a, FVec2& b, FVec2& c) {
	auto alphaAndBeta = BarycentricCoordinates(a, b, c, *this);
	if (alphaAndBeta.X < 0)
		return false;
	if (alphaAndBeta.Y < 0)
		return false;
	if ((1.0 - (alphaAndBeta.X + alphaAndBeta.Y)) < 0)
		return false;
	return true;
}
bool FVec2::IsOnLine( FVec2& a, FVec2& b)
{
	//FVec2 p1 = a - *this;
	//FVec2 p2 = b - *this;
	//float l1 = p1.Distance();
	//float l2 = p2.Distance();
	//return std::abs(p1.Dot(p2)+l1*l2)<= FLOAT_EPSILON*2;

	return std::abs((Y - a.Y) * (b.X - a.X) - (X - a.X) * (b.Y - a.Y))<= FLOAT_EPSILON;
}

// CONSTRUCTORS

FVec3::FVec3(const float x, const float y, const float z)
{
	X = x; Y = y; Z = z;
}

FVec3::FVec3(const FVec3& v)
{
	X = v.X; Y = v.Y; Z = v.Z;
}

// ASSIGNMENT OPERATORS

FVec3& FVec3::operator = (const FVec3& v)
{
	X = v.X; Y = v.Y; Z = v.Z; return *this;
}

 FVec3& FVec3::operator += (const FVec3& v)
{
	X += v.X; Y += v.Y; Z += v.Z; return *this;
}

 FVec3& FVec3::operator -= (const FVec3& v)
{
	X -= v.X; Y -= v.Y; Z -= v.Z; return *this;
}

 FVec3& FVec3::operator *= (const float d)
{
	X *= d; Y *= d; Z *= d; return *this;
}

 FVec3 FVec3::operator*(const float d) const
 {
	 return FVec3(X*d,Y*d,Z*d);
 }

 FVec3& FVec3::operator /= (const float d)
{
	float d_inv = 1.0f / d; X *= d_inv; Y *= d_inv; Z *= d_inv;
	return *this;
}

// SPECIAL FUNCTIONS

 float FVec3::Length() const
{
	return sqrt(LengthSqr());
}

 float FVec3::LengthSqr() const
{
	return X * X + Y * Y + Z * Z;
}

 FVec3& FVec3::Normalize() // it is up to caller to avoid divide-by-zero
{
	float len = Length();
	if (len > FLOAT_EPSILON) *this /= Length();
	return *this;
}

 FVec3 FVec3::Cross(const FVec3& v) const
{
	FVec3 tmp;
	tmp.X = Y * v.Z - Z* v.Y;
	tmp.Y = Z * v.X - X* v.Z;
	tmp.Z = X * v.Y - Y* v.X;
	return tmp;
}


// FRIENDS

 FVec3  FVec3::operator - (const FVec3& a)
{
	return FVec3(X-a.X, Y-a.Y, Z-a.Z);
}

 FVec3 FVec3::operator + (const FVec3& a)
{
	return FVec3(a.X + X, a.Y + Y, a.Z + Z);
}


 FVec3 FVec3:: operator * (const float d)
{
	return FVec3(d * X, d * Y, d * Z);
}


 FVec3 FVec3::operator * (const FVec3& a)
{
	return FVec3(a.X * X, a.Y * Y, a.Z * Z);
}

 FVec3 FVec3::operator / (const float d)const 
{
	float d_inv = 1.0f / d;
	return FVec3(X * d_inv, Y * d_inv, Z * d_inv);
}

 FVec3 FVec3::operator/(const FVec3& a)
 {
	 return FVec3(X / a.X, Y / a.Y, Z / a.Z);
 }

 FVec3 FVec3::operator ^ (const FVec3& a)
{
	return FVec3(Y * a.Z - Z * a.Y,
		Z * a.X - X * a.Z,
		X *a. Y - Y * a.X);
}

 int FVec3::operator == (const FVec3& a)const
{
	return Float::isEqual(a.X,X) && Float::isEqual(a.Y,Y) && Float::isEqual(a.Z ,Z);
}



 FVec3 FVec3::Prod(const FVec3& a)
{
	return FVec3(a.X * X, a.Y * Y, a.Z * Z);
}

 float FVec3::Dot(const FVec3& a)
{
	return a.X * X + a.Y * Y + a.Z * Z;
}


 float FVec3::Distance(const FVec3& a)  // distance
{
	return std::sqrt(DistanceSqr(a));
}

 float FVec3::DistanceSqr(const FVec3& a)  // distance
{
	return ((X - a.X) * (X - a.X) +
		(Y - a.Y) * (Y - a.Y) +
		(Z - a.Z) * (Z - a.Z));
}

 FVec3 FVec3::Ceiling(const FVec3& v) {
	return FVec3(std::max(X, v.X), std::max(Y, v.Y), std::max(Z, v.Z));
}

 FVec3 FVec3::Floor(const FVec3& v) {
	return FVec3(std::min(X, v.X), std::min(Y, v.Y), std::min(Z, v.Z));
}

 bool FVec3::IsOnSegment(FVec3& p1, FVec3& p2)
 {
	 return Float::isWeakZero(((X - p1.X) * (p1.Y - p2.Y))- ((p1.X - p2.X) * (Y - p1.Y)))
		 && (X >= std::min(p1.X, p2.X) && X <= std::max(p1.X, p2.X))
		 && ((Y >= std::min(p1.Y, p2.Y)) && (Y <= std::max(p1.Y, p2.Y)));
 }
