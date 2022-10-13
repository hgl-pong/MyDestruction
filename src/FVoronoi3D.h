#ifndef VORONOI3D_DIAGRAM_H
#define VORONOI3D_DIAGRAM_H
#define SMALL_NUMBER		(1.e-8f)
#define KINDA_SMALL_NUMBER	(1.e-4f)
#include <stdint.h>
#include <vector>
#include <voro++.hh>
#include "vec.h"
#include "PxPhysicsAPI.h"
#include "DirectXCollision.h"
struct Edge
{
	uint32_t s;
	uint32_t e;
};

struct VoroCellInfo
{
	FVec3 Position;
	float Volume;
	int Id;
	std::vector<FVec3> Vertices;
	std::vector<uint32_t> Faces;
	std::vector<uint32_t> Indices;
	std::vector<Edge> Edges;
	std::vector<int> Neighbors;
	std::vector<FVec3> Normals;
	std::vector<float> Areas;
	std::vector<FVec2> UVs;
	physx::PxRigidDynamic* rigidDynamic;
	DirectX::BoundingBox Box;
};


class  FVoronoi3D
{
public:
	struct BBox {
		FVec3 Min, Max;
	};
	struct Sphere
	{
		FVec3 center;
		float radius;
	};
private:
	int32_t NumSites;
	voro::container* Container;
	BBox Bounds;
	std::vector<VoroCellInfo> Cells;

public:

	FVoronoi3D(BBox& boundingBox, float SquaredDistSkipPtThreshold = 0.0f);
	~FVoronoi3D();

	int32_t Size() const
	{
		return NumSites;
	}

	void Clear();
	void SetBBox(BBox& boundingBox);

	void AddSites(const std::vector< FVec3>& Sites, float SquaredDistSkipPtThreshold = 0.0f,FVec3 transform=FVec3(0,0,0));
	void AddSite(const FVec3& Site, float SquaredDistSkipPtThreshold = 0.0f);
	void AddSites(int  count, float SquaredDistSkipPtThreshold = 0.0f);

	void ComputeAllCells();

	void ComputeCellEdgesSerial();
	void ComputeCellEdges();

	bool  VoronoiNeighbors(std::vector<std::vector<int>>& Neighbors, bool bExcludeBounds = true, float SquaredDistSkipPtThreshold = KINDA_SMALL_NUMBER);
	bool  GetVoronoiEdges(const std::vector< FVec3>& Sites, const BBox& Bounds, std::vector<Edge>& Edges, std::vector<int32_t>& CellMember, float SquaredDistSkipPtThreshold = KINDA_SMALL_NUMBER);

	void BoxSampling(BBox& box, std::vector<int32_t>& CellMember, bool random);
	void SphereSampling(FVec3& center, float radius, std::vector<int32_t>& CellMember, bool random);

	VoroCellInfo* GetAllCells()const;
private:
	void _CalculateUVs(VoroCellInfo& cell);
	void _CalculateNormals(VoroCellInfo& cell);
	bool _OutOfBox(const FVec3& p);

	// Add sites to Voronoi container, with contiguous ids, ignoring NaNs
	void _PutSites(voro::container* Container, const std::vector< FVec3>& Sites, int32_t Offset, FVec3& transform);
	// Add sites to Voronoi container, with contiguous ids, ignoring NaNs, ignoring Sites that are on top of existing sites
	int32_t _PutSitesWithDistanceCheck(voro::container* Container, const std::vector< FVec3>& Sites, int32_t Offset, FVec3& transform, float SquaredDistThreshold = 1e-4);

	voro::container* _StandardVoroContainerInit(int SiteCount, float SquaredDistSkipPtThreshold = 0.0f);


};

#endif//VORONOI3D_DIAGRAM_H





