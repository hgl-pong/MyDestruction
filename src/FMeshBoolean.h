#ifndef FMESH_BOOLEAN_H
#define FMESH_BOOLEAN_H
#include "MeshOperations/MeshSetOperation.h"
#include "MeshOperations/MeshMergeOperation.h"
#include "Mesh.h"
#include "Vector.h"
#include "FVoronoi3D.h"
using namespace MeshWarrior;

enum BooleanType
{
	DIFFEREN=1,
	INTERSEECTION=2,
	UNION=3
};
class FChunk;
class FMeshBoolean
{

public:
	FMeshBoolean(FChunk*);
	~FMeshBoolean();
	void FetchBooleanResult(VoroCellInfo& cell, VoroCellInfo& result, BooleanType type);

private:
	void _Difference(Mesh& mesh, VoroCellInfo& result);
	void _Intersection(Mesh& mesh, VoroCellInfo& result);
	void _Union(Mesh& mesh, VoroCellInfo& result);

	void _Merge(const Mesh*mesh, VoroCellInfo& result);
	void _Reset();
private:

	std::vector<Mesh::Vertex> m_Vertices;
	std::vector<std::vector<size_t>> m_Triangles;
	Mesh m_SourceMesh;
};

//static void exportObject(const char* filename, const std::vector<Mesh::Vertex>& vertices, const std::vector<std::vector<size_t>>& faces)
//{
//    FILE* fp = fopen(filename, "wb");
//    for (const auto& it : vertices) {
//        fprintf(fp, "v %f %f %f\n", it.x, it.y, it.z);
//    }
//    for (const auto& it : faces) {
//        if (it.size() == 2) {
//            fprintf(fp, "l");
//            for (const auto& v : it)
//                fprintf(fp, " %zu", v + 1);
//            fprintf(fp, "\n");
//            continue;
//        }
//        fprintf(fp, "f");
//        for (const auto& v : it)
//            fprintf(fp, " %zu", v + 1);
//        fprintf(fp, "\n");
//    }
//    fclose(fp);
//}
#endif // FMESH_BOOLEAN_H