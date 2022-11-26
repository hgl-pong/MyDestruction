#ifndef FMESH_BOOLEAN_H
#define FMESH_BOOLEAN_H
#include "Boolean/FAccelerator.h"
#include "Boolean/FGeometryCollection.h"
#include "FVoronoi3D.h"

class FChunk;
class FMeshBoolean
{

public:
	FMeshBoolean(FChunk*);
	~FMeshBoolean();
	void FetchBooleanResult(VoroCellInfo& cell, VoroCellInfo& result, CollectionType type);

private:
	FBoxAccelerator* m_SouceMesh;
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