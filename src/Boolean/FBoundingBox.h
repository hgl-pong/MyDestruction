#ifndef FBOUNDING_BOX_H
#define FBOUNDING_BOX_H
#include <vector>
#include "vec.h"

typedef uint32_t FIndex;

struct FVertex {
    //FIndex index;
    FVec3 position;
    FVec3 normals;
    FVec2 uv;
    FVec4 tangent;
    uint32_t diffuse;

    bool operator==(const FVertex& v) const { 
        return position == v.position && uv == v.uv;
    }

	bool operator<(const FVertex& v) const {
		return position < v.position && uv < v.uv;
	}

};

struct FEdge
{
    FIndex v[2];

    FIndex& start() {
        return v[0];
    }
    const FIndex& start() const {
        return v[0];
    }

    FIndex& end() {
        return v[1];
    }
    const FIndex& end() const { return v[1]; }

    FEdge()
    {
        v[0] = v[1] = -1;
    }
};

class FBoundingBox
{
public:
    FBoundingBox();
    FBoundingBox(const std::vector<FVec3>& points);
    FBoundingBox(const std::vector<FVertex>& points);
    FBoundingBox(FVec3& v0, FVec3& v1, FVec3& v2);
    ~FBoundingBox();

    bool Intersection(FBoundingBox& box);
    void Merge(FBoundingBox& box);
    void Include(FVec3& v);

    FBoundingBox operator* (const float& d){
        m_Min *= d;
        m_Max *= d;
        m_Size *= d;
        return *this;
    }
public:
    FVec3 m_Min;
    FVec3 m_Max;
    FVec3 m_Size;
};

struct FTriangle
{
    FVertex v[3];
    FBoundingBox box;

    const FVertex& i() const { return v[0]; }
    const FVertex& j() const { return v[1]; }
    const FVertex& k() const { return v[2]; }

    FTriangle() = default;
	FTriangle(FVertex& x, FVertex& y, FVertex& z) {
		v[0] = x;
		v[1] = y;
		v[2] = z;
		box = FBoundingBox(x.position, y.position, z.position);
	}

	FTriangle(FVec3& x, FVec3& y, FVec3& z) {
		v[0].position = x;
		v[1].position = y;
		v[2].position = z;
		box = FBoundingBox(x, y, z);
	}

    bool operator ==(const FTriangle& triangle) const{
        return v[0] == triangle.v[0] && v[1] == triangle.v[1] && v[2] == triangle.v[2];
    }

	bool operator <(const FTriangle& triangle) const {
        return v[0] < triangle.v[0] && v[1] == triangle.v[1] && v[2] == triangle.v[2];
	}

    bool IsOnEdge(FVec3& p) {
		bool a = p.IsOnSegment(v[0].position, v[1].position);
		bool b = p.IsOnSegment(v[0].position, v[2].position);
		bool c = p.IsOnSegment(v[1].position, v[2].position);
        return a || b || c;
    }
};

struct FMeshData {
	std::vector<FTriangle> m_Triangles;
	std::vector<FVertex> m_Vertices;
};


static bool WeakBoundingBoxIntersection(const FBoundingBox& aBox, const FBoundingBox& bBox)
{
	if (std::max(aBox.m_Min.X, bBox.m_Min.X) > std::min(aBox.m_Max.X, bBox.m_Max.X) + FLOAT_EPSILON)
		return false;
	if (std::max(aBox.m_Min.Y, bBox.m_Min.Y) > std::min(aBox.m_Max.Y, bBox.m_Max.Y) + FLOAT_EPSILON)
		return false;
	if (std::max(aBox.m_Min.Z, bBox.m_Min.Z) > std::min(aBox.m_Max.Z, bBox.m_Max.Z) + FLOAT_EPSILON)
		return false;
	return true;
}

static FVec3 Normal(FTriangle& triangle) {
    return Normal(triangle.i().position, triangle.j().position, triangle.k().position);
}

static float CalCulateVolume(FMeshData& meshdata) {
    float volume = 0;
    for (auto& tri : meshdata.m_Triangles)
    {
        const FVec3& a = tri.i().position;
        const FVec3& b = tri.j().position;
        const FVec3& c = tri.k().position;

        volume += (a.X * b.Y * c.Z - a.X * b.Z * c.Y - a.Y * b.X * c.Z + a.Y * b.Z * c.X + a.Z * b.X * c.Y - a.Z * b.Y * c.X);
    }
    return (1.0f / 6.0f) * std::abs(volume);
}

namespace std {

    template<>
    struct hash<FVertex> {
		size_t operator ()(const FVertex& x) const{
			//return hash<float>()(x.position.X) ^ hash<float>()(x.position.Y) ^ hash<float>()(x.position.Z) ^ hash<float>()(x.uv.X) ^ hash<float>()(x.uv.Y);
			return hash<FVec3>()(x.position) ^ hash<FVec2>()(x.uv) ;
		}
    };

    template<>
    struct equal_to<FVertex> {
        bool operator ()(const FVertex& a,const FVertex&b)const {
            return a == b;
        }
    };

	template<>
	struct hash<FTriangle> {
		size_t operator ()(const FTriangle& x)const {
			return hash<FVertex>()(x.i()) ^ hash<FVertex>()(x.j()) ^ hash<FVertex>()(x.k());
		}
	};

	template<>
	struct equal_to<FTriangle> {
		bool operator ()(const FTriangle& a, const FTriangle& b)const {
			return a == b;
		}
	};
}

#endif // FBOUNDING_BOX_H
