#ifndef INTERSECT_UTILS_H
#define INTERSECT_UTILS_H
#include "FBoundingBox.h"
#include <unordered_set>
#include <unordered_map>
#include "tri_tri_intersect.h"
typedef std::pair<FVec3, int> DetectPair;
namespace std {

	template<>
	struct hash<DetectPair> {
		size_t operator ()(const DetectPair& x) const {
			return hash<FVec3>()(x.first) ^ hash<int>()(x.second);
		}
	};
}


class IntersectUtils {
public:

	static bool TrianglesIntersect(FTriangle& triangleA, FTriangle& triangleB, std::pair<FVertex, FVertex>*& edge) {
		int coplanar = 0;
		edge = nullptr;
		FVec3 posA, posB;
		if (!tri_tri_intersection_test_3d(
			(float*)(&triangleA.i().position),
			(float*)(&triangleA.j().position),
			(float*)(&triangleA.k().position), 
			(float*)(&triangleB.i().position),
			(float*)(&triangleB.j().position),
			(float*)(&triangleB.k().position),
			&coplanar,
			(float*)(&posA),
			(float*)(&posB))) {
			return false;
		}
		if (coplanar)
			return false;
		if (posA == posB)
			return false;
		edge = new std::pair<FVertex, FVertex>[2];
		edge[0].first.position = posA;
		edge[0].second.position = posB;

		edge[1].first.position = posA;
		edge[1].second.position = posB;

		if(!(triangleA.i().uv==FVec2(0,0)&&triangleA.j().uv == FVec2(0, 0)&&triangleA.k().uv == FVec2(0, 0)))
		CalcluateUV(triangleA, edge[0]);
		if (!(triangleB.i().uv == FVec2(0, 0) && triangleB.j().uv == FVec2(0, 0) && triangleB.k().uv == FVec2(0, 0)))
		CalcluateUV(triangleB, edge[1]);
	}


	static void CalcluateUV(FTriangle& triangle, std::pair<FVertex, FVertex>& edge) {
		float a, b, c;
		a = (-(edge.first.position.X - triangle.j().position.X) * (triangle.k().position.Y - triangle.j().position.Y) + (edge.first.position.Y - triangle.j().position.Y) * (triangle.k().position.X - triangle.j().position.X)) /
			(-(triangle.i().position.X - triangle.j().position.X) * (triangle.k().position.Y - triangle.j().position.Y) + (triangle.i().position.Y - triangle.j().position.Y) * (triangle.k().position.X - triangle.j().position.X));
		b = (-(edge.first.position.X - triangle.k().position.X) * (triangle.i().position.Y - triangle.k().position.Y) + (edge.first.position.Y - triangle.k().position.Y) * (triangle.i().position.X - triangle.k().position.X)) /
			(-(triangle.i().position.X - triangle.k().position.X) * (triangle.i().position.Y - triangle.k().position.Y) + (triangle.j().position.Y - triangle.k().position.Y) * (triangle.i().position.X - triangle.k().position.X));
		c = 1 - a - b;
		edge.first.uv = FVec2(a*triangle.i().uv.X+b*triangle.j().uv.X+c*triangle.k().uv.X, a * triangle.i().uv.Y + b * triangle.j().uv.Y+c * triangle.k().uv.Y);

		a = (-(edge.second.position.X - triangle.j().position.X) * (triangle.k().position.Y - triangle.j().position.Y) + (edge.second.position.Y - triangle.j().position.Y) * (triangle.k().position.X - triangle.j().position.X)) /
			(-(triangle.i().position.X - triangle.j().position.X) * (triangle.k().position.Y - triangle.j().position.Y) + (triangle.i().position.Y - triangle.j().position.Y) * (triangle.k().position.X - triangle.j().position.X));
		b = (-(edge.second.position.X - triangle.k().position.X) * (triangle.i().position.Y - triangle.k().position.Y) + (edge.second.position.Y - triangle.k().position.Y) * (triangle.i().position.X - triangle.k().position.X)) /
			(-(triangle.i().position.X - triangle.k().position.X) * (triangle.i().position.Y - triangle.k().position.Y) + (triangle.j().position.Y - triangle.k().position.Y) * (triangle.i().position.X - triangle.k().position.X));
		c = 1 - a - b;
		edge.second.uv = FVec2(a * triangle.i().uv.X + b * triangle.j().uv.X + c * triangle.k().uv.X, a * triangle.i().uv.Y + b * triangle.j().uv.Y + c * triangle.k().uv.Y);
	}

	static bool IntersectSegmentAndPlane( FVec3& segmentPoint0,  FVec3& segmentPoint1,
		FVec3& pointOnPlane, FVec3& planeNormal,
		FVec3* intersection)
	{
		FVec3 u = segmentPoint1 - segmentPoint0;
		FVec3 w = pointOnPlane - segmentPoint0;
		float d = planeNormal.Dot(u);
		float n = planeNormal.Dot(w);
		//
		
		if (std::abs(d) <= FLOAT_EPSILON)
		if (Float::isWeakZero(d))
			return false;
		auto s = n / d;
		if (s < 0 || s> 1 || std::isnan(s) || std::isinf(s))
			return false;
		if (intersection!=nullptr)
			*intersection = segmentPoint0 + u * s;
		return true;

	}

	static bool IsInMesh(/*FMeshData&meshdata, */std::unordered_set<FTriangle>& triangles,FVec3 point, const FVec3& testAxis)
	{
		//std::vector<FTriangle>& triangles = meshdata.m_Triangles;
		//point = point * SCALE;
		FVec3 testEnd = point + testAxis;
		bool inside = false;
		FBoundingBox box;
		box.Include(point);
		box.Include(testEnd);
		std::unordered_set<DetectPair> hits;
		int time = 0;
		for (const auto triangle : triangles) {
			//triangle.box *= SCALE ;
			if (!WeakBoundingBoxIntersection(box, triangle.box))
				continue;
			std::vector<FVec3> trianglePositions = {
				triangle.i().position ,
				triangle.j().position,
				triangle.k().position
			};
			FVec3 intersection;
			FVec3 normal = Normal(triangle.i().position, triangle.j().position, triangle.k().position);

			if (IsInTriangle(point, trianglePositions.data())) {
				return true;
			}
			if (IntersectUtils::IntersectSegmentAndPlane(point, testEnd,
				trianglePositions[0],
				normal,
				&intersection)) {
				if (IsInTriangle(intersection, trianglePositions.data())) {
					float dir = normal.Dot((testAxis/FLOAT_MAX).Normalize());
					bool sameDir = dir > 0;
					if (hits.emplace(std::make_pair(intersection,sameDir)).second&&sameDir)
						time++;
				}
			}
		}
		inside = (2*time > hits.size());
		return inside;
	}
};
#endif // INTERSECT_UTILS_H
