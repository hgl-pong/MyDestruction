#include "FMeshBoolean.h"
#include "FChunk.h"

FMeshBoolean::FMeshBoolean(FChunk* chunk) {

	m_Vertices.resize(chunk->m_Vertices2.size());
	m_Triangles.resize(chunk->m_Indices.size() / 3);
	for (int i = 0; i < chunk->m_Vertices2.size(); i++) {
		m_Vertices[i] = Vector3(chunk->m_Vertices2[i].X, chunk->m_Vertices2[i].Y, chunk->m_Vertices2[i].Z);
	}
	for (int i = 0; i < m_Triangles.size(); i++) {
		m_Triangles[i] = {chunk->m_Indices[3*i],chunk->m_Indices[3 * i+1] ,chunk->m_Indices[3 * i+2] };
	}

	std::vector<Vector3> *Vertices=new std::vector<Vector3>(m_Vertices);
	std::vector<std::vector<size_t>> *Triangles=new std::vector<std::vector<size_t>>(m_Triangles);
	m_SourceMesh.setVertices(Vertices);
	m_SourceMesh.setTriangles(Triangles);
	m_SourceMesh.prepare();
}

FMeshBoolean::~FMeshBoolean() {

}

void FMeshBoolean::FetchBooleanResult(VoroCellInfo& cell, VoroCellInfo& result, BooleanType type) 
{
	_Reset();
	std::vector<Vector3> Vertices;
	std::vector<std::vector<size_t>> Triangles;

	Vertices.resize(cell.Vertices.size());
	Triangles.resize(cell.Indices.size() / 3);
	for (int i = 0; i < cell.Vertices.size(); i++) {
		Vertices[i] = Vector3(cell.Vertices[i].X, cell.Vertices[i].Y, cell.Vertices[i].Z);
	}
	for (int i = 0; i < Triangles.size(); i++) {
		Triangles[i] = { cell.Indices[3 * i],cell.Indices[3 * i + 1] ,cell.Indices[3 * i + 2] };
	}

	SolidMesh mesh;
	mesh.setVertices(&Vertices);
	mesh.setTriangles(&Triangles);
	mesh.prepare();

	switch (type)
	{
	case DIFFERENCE:
		_Difference(mesh, result);
		break;
	case INTERSEECTION:
		_Intersection(mesh, result);
		break;
	case UNION:
		_Union(mesh, result);
		break;
	default:
		break;
	}
	
}

void FMeshBoolean::_Difference(SolidMesh& mesh, VoroCellInfo& result) {
	SolidBoolean solidBoolean(&m_SourceMesh, &mesh);
	solidBoolean.combine()
		;
	std::vector<std::vector<size_t>> resultTriangles;
	solidBoolean.fetchDiff(resultTriangles);
	_Merge(solidBoolean.resultVertices(), resultTriangles, result);
}

void FMeshBoolean::_Intersection(SolidMesh& mesh, VoroCellInfo& result) {
	SolidBoolean solidBoolean(&m_SourceMesh, &mesh);
	solidBoolean.combine();

	std::vector<std::vector<size_t>> resultTriangles;
	solidBoolean.fetchIntersect(resultTriangles);
	_Merge(solidBoolean.resultVertices(), resultTriangles, result);

	exportObject("debug-merged-result.obj",
		solidBoolean.resultVertices(), resultTriangles);
}

void FMeshBoolean::_Union(SolidMesh& mesh, VoroCellInfo& result) {
	SolidBoolean solidBoolean(&m_SourceMesh, &mesh);
	solidBoolean.combine();

	std::vector<std::vector<size_t>> resultTriangles;
	solidBoolean.fetchDiff(resultTriangles);
	_Merge(solidBoolean.resultVertices(), resultTriangles, result);
}

void FMeshBoolean::_Merge(const std::vector<Vector3>& vertices,
	const std::vector<std::vector<size_t>>& triangles, VoroCellInfo& result) {

	for (const auto& it : triangles) {
		result.Indices.push_back(it[0]);
		result.Indices.push_back(it[1]);
		result.Indices.push_back(it[2]);
	}
	for (const auto& it : vertices) {
		result.Vertices.push_back(FVec3(it[0],it[1],it[2]));
	}

	result.Normals = std::vector<FVec3>(vertices.size(), FVec3(0, 1, 0));
	result.UVs = std::vector<FVec2>(vertices.size(), FVec2(0, 0));

}

void FMeshBoolean::_Reset() {
	std::vector<Vector3>* Vertices = new std::vector<Vector3>(m_Vertices);
	std::vector<std::vector<size_t>>* Triangles = new std::vector<std::vector<size_t>>(m_Triangles);
	m_SourceMesh.setVertices(Vertices);
	m_SourceMesh.setTriangles(Triangles);
	m_SourceMesh.prepare();
}