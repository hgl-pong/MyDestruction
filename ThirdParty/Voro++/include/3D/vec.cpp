#include "vec.h"
#include <cmath>

// CONSTRUCTORS

FVec3::FVec3(const double x, const double y, const double z)
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

FVec3& FVec3::operator *= (const double d)
{
	X *= d; Y *= d; Z *= d; return *this;
}

FVec3& FVec3::operator /= (const double d)
{
	double d_inv = 1.0f / d; X *= d_inv; Y *= d_inv; Z *= d_inv;
	return *this;
}

// SPECIAL FUNCTIONS

double FVec3::Length() const
{
	return sqrt(SqrLength());
}

double FVec3::SqrLength() const
{
	return X * X + Y * Y + Z * Z;
}

FVec3& FVec3::Normalize() // it is up to caller to avoid divide-by-zero
{
	double len = Length();
	if (len > 0.000001) *this /= Length();
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


FVec3 FVec3:: operator * (const double d)
{
	return FVec3(d * X, d * Y, d * Z);
}


double FVec3::operator * (const FVec3& a)
{
	return Dot(a);
}

FVec3 FVec3::operator / (const double d)
{
	double d_inv = 1.0f / d;
	return FVec3(X * d_inv, Y * d_inv, Z * d_inv);
}

FVec3 FVec3::operator ^ (const FVec3& a)
{
	return FVec3(Y * a.Z - Z * a.Y,
		Z * a.X - X * a.Z,
		X *a. Y - Y * a.X);
}

int FVec3::operator == (const FVec3& a)
{
	return (a.X == X) && (a.Y == Y) && (a.Z == Z);
}


FVec3 FVec3::Prod(const FVec3& a)
{
	return FVec3(a.X * X, a.Y * Y, a.Z * Z);
}

double FVec3::Dot(const FVec3& a)
{
	return a.X * X + a.Y * Y + a.Z * Z;
}


double FVec3::Distance(const FVec3& a)  // distance
{
	return std::sqrt(DistanceSqr(a));
}

double FVec3::DistanceSqr(const FVec3& a)  // distance
{
	return ((X - a.X) * (X - a.X) +
		(Y - a.Y) * (Y - a.Y) +
		(Z - a.Z) * (Z - a.Z));
}
