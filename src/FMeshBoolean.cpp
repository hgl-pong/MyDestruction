#include "FMeshBoolean.h"
#include "FChunk.h"
#include <unordered_map>
FMeshBoolean::FMeshBoolean(FChunk* chunk)
:m_SouceMesh(nullptr){
	FMeshData meshdata;
	meshdata.m_Vertices.resize(chunk->m_Vertices2.size());
	meshdata.m_Triangles.resize(chunk->m_Indices.size() / 3);
	for (int i = 0; i < chunk->m_Vertices2.size(); i++) {
		meshdata.m_Vertices[i].position = chunk->m_Vertices2[i];
		meshdata.m_Vertices[i].normals = chunk->m_Normals2[i];
		meshdata.m_Vertices[i].uv = chunk->m_UVs[i];
	}
	for (int i = 0; i < meshdata.m_Triangles.size(); i++) {
		meshdata.m_Triangles[i] = { meshdata.m_Vertices[chunk->m_Indices[3*i]],meshdata.m_Vertices[chunk->m_Indices[3 * i+1]] ,meshdata.m_Vertices[chunk->m_Indices[3 * i+2]] };
	}
	FBoundingBox boxA(meshdata.m_Vertices);
	m_SouceMesh = new FBoxAccelerator(boxA, meshdata);
}

FMeshBoolean::~FMeshBoolean() {
	FDELETE(m_SouceMesh);
}

void FMeshBoolean::FetchBooleanResult(VoroCellInfo& cell, VoroCellInfo& result, CollectionType type) 
{
	FMeshData meshB;
	meshB.m_Vertices.resize(cell.Vertices.size());
	meshB.m_Triangles.resize(cell.Indices.size() / 3);
	for (int i = 0; i < cell.Vertices.size(); i++) {
		meshB.m_Vertices[i].position = cell.Vertices[i];
		meshB.m_Vertices[i].normals = cell.Normals[i];
		meshB.m_Vertices[i].uv = cell.UVs[i];
	}
	for (int i = 0; i < meshB.m_Triangles.size(); i++) {
		meshB.m_Triangles[i] = { meshB.m_Vertices[cell.Indices[ 3 * i]],meshB.m_Vertices[cell.Indices[3 * i + 1]] ,meshB.m_Vertices[cell.Indices[3 * i + 2]] };
	}
	FBoundingBox boxB(meshB.m_Vertices);
	FMeshData mesh = m_SouceMesh->CollecteTriangles(boxB);
	FGeometryCollection collection(boxB, meshB);
	collection.SetMeshB(mesh);

	FMeshData 	output = collection.FetchResult(type);

	std::unordered_map<FVertex, int> indexMap;
	result.Vertices.clear();
	result.Normals.clear();
	result.UVs.clear();
	result.Indices.clear();
	for (FIndex i = 0; i < output.m_Vertices.size(); i++) {
		result.Vertices.push_back(output.m_Vertices[i].position);
		result.Normals.push_back(output.m_Vertices[i].normals);
		result.UVs.push_back(output.m_Vertices[i].uv);
		indexMap.emplace(std::make_pair(output.m_Vertices[i], i));
	}

	for (FIndex i = 0; i < output.m_Triangles.size(); i++) {
		result.Indices.push_back(indexMap[output.m_Triangles[i].i()]);
		result.Indices.push_back(indexMap[output.m_Triangles[i].j()]);
		result.Indices.push_back(indexMap[output.m_Triangles[i].k()]);

	}
	result.Neighbors = cell.Neighbors;
	result.Areas = cell.Areas;
	result.Volume = CalCulateVolume(output);
}