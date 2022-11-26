#ifndef INTERSECT_UTILS_H
#define INTERSECT_UTILS_H
#include "FBoundingBox.h"
#include "FPositionKey.h"
#include <unordered_set>
#include <unordered_map>
#include "tri_tri_intersect.h"
typedef std::pair<FPositionKey, int> DetectPair;
namespace std {

	template<>
	struct hash<DetectPair> {
		size_t operator ()(const DetectPair& x) const {
			return hash<FPositionKey>()(x.first) ^ hash<int>()(x.second);
		}
	};
}


class IntersectUtils {
public:

	static bool TrianglesIntersect(FTriangle& triangleA, FTriangle& triangleB, std::pair<FVertex, FVertex>*& edge) {
		int coplanar = 0;
		edge = nullptr;
		double pA[3], pB[3];
		double p1[3] = { triangleA.i().position.X,triangleA.i().position.Y,triangleA.i().position.Z };
		double p2[3] = { triangleA.j().position.X,triangleA.j().position.Y,triangleA.j().position.Z };
		double p3[3] = { triangleA.k().position.X,triangleA.k().position.Y,triangleA.k().position.Z };		
		
		double q1[3] = { triangleB.i().position.X,triangleB.i().position.Y,triangleB.i().position.Z };
		double q2[3] = { triangleB.j().position.X,triangleB.j().position.Y,triangleB.j().position.Z };
		double q3[3] = { triangleB.k().position.X,triangleB.k().position.Y,triangleB.k().position.Z };
		if (!tri_tri_intersection_test_3d(p1,p2,p3,q1,q2,q3,&coplanar,pA,pB)) {
			return false;
		}
		if (coplanar)
			return false;

		FVec3 posA={ (float)pA[0] ,(float)pA[1] ,(float)pA[2] };
		FVec3 posB = { (float)pB[0],(float)pB[1],(float)pB[2]};
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
		if (std::abs(d) < FLOAT_EPSILON)
			return false;
		auto s = n / d;
		if (s < 0 || s> 1 || std::isnan(s) || std::isinf(s))
		//if (s < -1 || s> 1  || std::isnan(s) || std::isinf(s))
			return false;
		if (intersection!=nullptr)
			*intersection = segmentPoint0 + u * s;
		return true;

	}

	static bool IntersectSegmentAndTriangle(FVec3& segmentPoint0, FVec3& segmentPoint1,
		FVec3* triangle)
	{
		FVec3 P1, P2;
		P1 = segmentPoint0;
		P2 = segmentPoint1;

		FVec3 p1, p2, p3;
		p1 = triangle[0];
		p2 = triangle[0];
		p3 = triangle[0];

		FVec3 v1 = p1 - p2;
		FVec3 v2 = p3 - p2;

		float a, b, c, d;

		a = v1.Y * v2.Z - v1.Z * v2.Y;
		b = -(v1.X * v2.Z - v1.Z * v2.X);
		c = v1.X * v2.Y - v1.Y * v2.X;
		d = -(a * p1.X + b * p1.Y + c * p1.Z);

		FVec3 O = P1;
		FVec3 V = P2 - P1;

		float t;

		t = -(a * O.X + b * O.Y + c * O.Z + d) / (a * V.X + b * V.Y + c * V.Z);

		FVec3 p = O + V * t;

		float xmin = std::min(P1.X, P2.X);
		float ymin = std::min(P1.Y, P2.Y);
		float zmin = std::min(P1.Z, P2.Z);

		float xmax = std::max(P1.X, P2.X);
		float ymax = std::max(P1.Y, P2.Y);
		float zmax = std::max(P1.Z, P2.Z);


		if (p.X >= xmin && p.X <= xmax && p.Y >= ymin && p.Y <= ymax && p.Z >= zmin && p.Z <= zmax) {
			//*result = p.Length();
			return true;
		}
		return false;
	}

	static bool IsInMesh(/*FMeshData&meshdata, */std::unordered_set<FTriangle>& triangles,FVec3 &point, const FVec3& testAxis)
	{
		//std::vector<FTriangle>& triangles = meshdata.m_Triangles;
		FVec3 testEnd = point + testAxis;
		bool inside = false;
		FBoundingBox box;
		box.Include(point);
		box.Include(testEnd);
		std::unordered_set<DetectPair> hits;
		int count = 0;
		for (const auto& triangle : triangles) {
			if (!WeakBoundingBoxIntersection(box, triangle.box))
				continue;
			std::vector<FVec3> trianglePositions = {
				triangle.i().position,
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
					hits.emplace(std::make_pair(FPositionKey(intersection), Float::isEqual(dir,1)));
				}
			}

			//if (IntersectUtils::IntersectSegmentAndTriangle(point, testEnd,
			//	trianglePositions.data())) {
			//		float dir = normal.Dot(testAxis);
			//		auto it=hits.emplace(std::make_pair(FPositionKey(intersection), dir));
			//		if (it.second)
			//			count++;
			//}

		}
		int time = 0;
		for (auto hit : hits) {
			if (hit.second>0)
				time++;
		}
		inside = (2*time > hits.size());
		//inside = (hits.size() % 2!=0);
		//inside = (count % 2 != 0);
		return inside;
	}
};
#endif // INTERSECT_UTILS_H
