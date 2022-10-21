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
	std::map<int, FVec3> vbuff;
	std::map<int, int>ibuff;
	for (const auto& it : triangles) {
		vbuff.emplace(it[0], FVec3(vertices[it[0]][0], vertices[it[0]][1], vertices[it[0]][2]));
		vbuff.emplace(it[1], FVec3(vertices[it[1]][0], vertices[it[1]][1], vertices[it[1]][2]));
		vbuff.emplace(it[2], FVec3(vertices[it[2]][0], vertices[it[2]][1], vertices[it[2]][2]));
	}
	int i = 0;
	for (auto v : vbuff) {
		ibuff.emplace(v.first, i);
		i++;
	}
	for (const auto& it : triangles) {
		result.Indices.push_back(ibuff[it[0]]);
		result.Indices.push_back(ibuff[it[1]]);
		result.Indices.push_back(ibuff[it[2]]);
	}
	for (const auto& it : vbuff) {
		result.Vertices.push_back(it.second);
	}

	result.Normals = std::vector<FVec3>(result.Vertices.size(), FVec3(0, 0, 0));
	for (int i = 0; i < result.Indices.size() / 3; i++) {
		uint32_t p0, p1, p2;
		p0 = result.Indices[3 * i];
		p1 = result.Indices[3 * i + 1];
		p2 = result.Indices[3 * i + 2];
		FVec3 v01 = result.Vertices[p1] - result.Vertices[p0];
		FVec3 v02 = result.Vertices[p2] - result.Vertices[p0];
		FVec3 normal = v01.Cross(v02);

		result.Normals[p0] = result.Normals[p0] + normal;
		result.Normals[p1] = result.Normals[p1] + normal;
		result.Normals[p2] = result.Normals[p2] + normal;
	}
	for (int i = 0; i < result.Normals.size(); i++)
		result.Normals[i].Normalize();
	result.UVs = std::vector<FVec2>(vertices.size(), FVec2(0, 0));

}

void FMeshBoolean::_Reset() {
	std::vector<Vector3>* Vertices = new std::vector<Vector3>(m_Vertices);
	std::vector<std::vector<size_t>>* Triangles = new std::vector<std::vector<size_t>>(m_Triangles);
	m_SourceMesh.setVertices(Vertices);
	m_SourceMesh.setTriangles(Triangles);
	m_SourceMesh.prepare();
}