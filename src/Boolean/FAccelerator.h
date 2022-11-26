#ifndef FACCELLERATOR_H
#define FACCELLERATOR_H
#include "FBoundingBox.h"
#include "FPositionKey.h"
#include <unordered_set>
class FBoxAccelerator
{
public:
	FBoxAccelerator(FBoundingBox& box,FMeshData& mesh,int level=10);
	~FBoxAccelerator();
	FMeshData CollecteTriangles(FBoundingBox& box);
private:
	void _BuildBox(FMeshData& mesh);
	
private:
	FBoundingBox m_Box;
	FMeshData m_MeshData;
	int m_Level;
	std::vector<std::vector<int>> m_Map;
	std::vector<FBoundingBox> m_Grids;
};
#endif // !FACCELERATOR_H


