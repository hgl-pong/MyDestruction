#ifndef VEC_H
#define VEC_H
#include <vector>
#include <cmath>
#include <limits>
#define FLOAT_EPSILON std::numeric_limits<float>::epsilon()*10
#define FLOAT_WEAK_EPSILON 1e-4f
#define FLOAT_MAX std::numeric_limits<float>::max()
#define FLOAT_MIN std::numeric_limits<float>::lowest()

#define SCALE 1e4
namespace Float
{

	inline bool isZero(float number)
	{
		return std::abs(number * number) <= FLOAT_EPSILON;
	}

	inline bool isWeakZero(float number)
	{
		return std::abs(number) <= FLOAT_WEAK_EPSILON;
	}

	inline bool isEqual(float a, float b)
	{
		return isZero(a - b);
	}

	inline bool isWeakEqual(float a, float b)
	{
		return isWeakZero(a - b);
	}

} //namespace float
class FVec2
{
public:
	float X, Y;
public:
	FVec2() { X = 0; Y = 0; }
	FVec2(float x, float y) :X(x), Y(y) {}
	FVec2& operator = (const FVec2& v) {
		X = v.X; Y = v.Y;
		return *this;
	}

	bool operator==(const FVec2& v)const {
		FVec2 d = *this - v;
		return Float::isWeakZero(d.LengthSqr());
		//return Float::isWeakEqual(v.X, X) && Float::isWeakEqual(v.Y, Y);
	}


	bool operator<(const FVec2& v)const {
		if (X < v.X)
			return true;
		if (X > v.X)
			return false;
		if (Y < v.Y)
			return true;
		if (Y > v.Y)
			return false;
		return false;
	}


	float Dot(const FVec2& v) {
		return X * v.X + Y * v.Y;
	}

	FVec2 operator - (const FVec2& v) const {
		return FVec2(X - v.X, Y - v.Y);
	}

	FVec2 operator / (const float& d) {
		return FVec2(X / d, Y / d);
	}

	bool IsInPolygon(const std::vector<FVec2>& polygon);

	bool IsInPolygon(const std::vector<FVec2>& polygonVertices, const std::vector<uint32_t>& polygonIndices);

	bool IsInTriangle(FVec2& a, FVec2& b, FVec2& c);

	bool IsOnLine(FVec2& a, FVec2& b);

	float Length() {
		return sqrt(LengthSqr());
	}

	float LengthSqr() {
		return Dot(*this);
	}
};

struct FVec4
{
	float X, Y, Z, W;
};
class FVec3
{
public:

	float X, Y, Z;

public:
	FVec3() { X = 0; Y = 0; Z = 0; };
	FVec3(float x, float y, float z);
	FVec3(const FVec3&);
	~FVec3() = default;

	FVec3& operator	= (const FVec3& v);
	FVec3& operator += (const FVec3& v);
	FVec3& operator -= (const FVec3& v);
	FVec3& operator *= (const float d);
	FVec3& operator /= (const float d);


	float Length() const;
	float LengthSqr() const;
	FVec3& Normalize();
	FVec3 Cross(const FVec3& v) const;

	FVec3 operator - (const FVec3& v)const;
	FVec3 operator + (const FVec3& v)const;
	FVec3 operator * (const float d);
	FVec3 operator * (const float d)const;
	FVec3 operator * (const FVec3& v);
	FVec3 operator / (const float d)const;
	FVec3 operator / (const FVec3& d);
	FVec3 operator ^ (const FVec3& v);
	int operator == (const FVec3& v)const;
	FVec3 Prod(const FVec3& v);
	float Dot(const FVec3& v);
	float Distance(const FVec3& v)const;
	float DistanceSqr(const FVec3& v)const;

	FVec3 Ceiling(const FVec3& v);
	FVec3 Floor(const FVec3& v);

	bool IsOnSegment(FVec3& a, FVec3& b);

	bool operator<(const FVec3& v)const {
		if (X < v.X)
			return true;
		if (X > v.X)
			return false;
		if (Y < v.Y)
			return true;
		if (Y > v.Y)
			return false;
		if (Z < v.Z)
			return true;
		if (Z > v.Z)
			return false;
		return false;
	}

};

static FVec3 Normal(const FVec3& a, const FVec3& b, const FVec3& c)
{
	float baX = b.X - a.X;
	float baY = b.Y - a.Y;
	float baZ = b.Z - a.Z;

	float caX = c.X - a.X;
	float caY = c.Y - a.Y;
	float caZ = c.Z - a.Z;

	FVec3 ba(baX, baY, baZ);
	FVec3 ca(caX, caY, caZ);
	FVec3 normal = ba.Cross(ca);

	float length = normal.Length();
	if (Float::isWeakZero(length))
		return FVec3();

	return FVec3(normal.X / length, normal.Y / length, normal.Z / length);
}

static bool IsInTriangle(FVec3& intersection, FVec3* trianglePositions) {
	//点在三角形内
	std::vector<FVec3> normals;
	for (size_t i = 0; i < 3; ++i) {
		size_t j = (i + 1) % 3;

		if (intersection == trianglePositions[i] || intersection.IsOnSegment(trianglePositions[i], trianglePositions[j]))
			return true;
		normals.push_back(Normal(intersection, trianglePositions[i], trianglePositions[j]));
	}
	float d1 = normals[0].Dot(normals[1]);
	float d2 = normals[0].Dot(normals[2]);
	float d3 = normals[1].Dot(normals[2]);
	//在三角形内或三角形边上时
	return (d1 + FLOAT_EPSILON >= 0 && d2 + FLOAT_EPSILON >= 0 && d3 + FLOAT_EPSILON >= 0);
	/*(d1 < 0 && d2 < 0 && d3 < 0 && std::abs(d1 + 1) <= FLOAT_EPSILON && std::abs(d2 + 1) <= FLOAT_EPSILON && std::abs(d3 + 1) <= FLOAT_EPSILON) ||
	(d1 >0 && d2 > 0&&d3>0&&std::abs(d1-1)<=FLOAT_EPSILON&& std::abs(d2-1) <= FLOAT_EPSILON&& std::abs(d3 - 1) <= FLOAT_EPSILON) ||
	(float::isZero(d1) || float::isZero(d2)|| float::isZero(d2));*/
}
namespace std {

	template<>
	struct hash<FVec3> {
		size_t operator ()(const FVec3& x) const {
			long vx = x.X * SCALE;
			long vy = x.Y * SCALE;
			long vz = x.Z * SCALE;
			return hash<long>()(vx) << 32 + hash<long>()(vy) << 32 + hash<long>()(vz);
		}
	};

	template<>
	struct hash<FVec2> {
		size_t operator ()(const FVec2& x) const {
			long vx = x.X * SCALE;
			long vy = x.Y * SCALE;
			return hash<long>()(vx) << 32 + hash<long>()(vy);
		}
	};
}

#endif // VEC_H
