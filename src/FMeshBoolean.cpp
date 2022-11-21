#include "FMeshBoolean.h"
#include "FChunk.h"

FMeshBoolean::FMeshBoolean(FChunk* chunk) {

	m_Vertices.resize(chunk->m_Vertices2.size());
	m_Triangles.resize(chunk->m_Indices.size() / 3);
	for (int i = 0; i < chunk->m_Vertices2.size(); i++) {
		m_Vertices[i].point = Vector(chunk->m_Vertices2[i].X, chunk->m_Vertices2[i].Y, chunk->m_Vertices2[i].Z);
		m_Vertices[i].normal = Vector(chunk->m_Normals2[i].X, chunk->m_Normals2[i].Y, chunk->m_Normals2[i].Z);
		m_Vertices[i].texCoords = Vector(chunk->m_UVs[i].X, chunk->m_UVs[i].Y,0);
	}
	for (int i = 0; i < m_Triangles.size(); i++) {
		m_Triangles[i] = {chunk->m_Indices[3*i],chunk->m_Indices[3 * i+1] ,chunk->m_Indices[3 * i+2] };
	}

	std::vector<Mesh::Vertex> *Vertices=new std::vector<Mesh::Vertex>(m_Vertices);
	std::vector<std::vector<size_t>> *Triangles=new std::vector<std::vector<size_t>>(m_Triangles);
	for (int i = 0; i < Vertices->size(); i++)
		m_SourceMesh.AddVertex(( * Vertices)[i]);
	for (int i = 0; i < Triangles->size(); i++) {
		Mesh::Face face;
		face.vertexArray.push_back(( * Triangles)[i][0]);
		face.vertexArray.push_back(( * Triangles)[i][1]);
		face.vertexArray.push_back(( * Triangles)[i][2]);
		m_SourceMesh.AddFace( face);
	}
}

FMeshBoolean::~FMeshBoolean() {

}

void FMeshBoolean::FetchBooleanResult(VoroCellInfo& cell, VoroCellInfo& result, BooleanType type) 
{
	//_Reset();
	std::vector<Mesh::Vertex> Vertices;
	std::vector<std::vector<size_t>> Triangles;

	Vertices.resize(cell.Vertices.size());
	Triangles.resize(cell.Indices.size() / 3);
	for (int i = 0; i < cell.Vertices.size(); i++) {
		Vertices[i].point = Vector(cell.Vertices[i].X, cell.Vertices[i].Y, cell.Vertices[i].Z);
		Vertices[i].normal = Vector(cell.Normals[i].X, cell.Normals[i].Y, cell.Normals[i].Z);
		Vertices[i].texCoords = Vector(cell.UVs[i].X, cell.UVs[i].Y, 0);
	}
	for (int i = 0; i < Triangles.size(); i++) {
		Triangles[i] = { cell.Indices[3 * i],cell.Indices[3 * i + 1] ,cell.Indices[3 * i + 2] };
	}

	Mesh mesh;
	for(int i=0;i<Vertices.size(); i++)
		mesh.AddVertex(Vertices[i]);
	for (int i = 0; i < Triangles.size() ; i++) {
		Mesh::Face face;
		face.vertexArray.push_back(Triangles[i][0]);
		face.vertexArray.push_back(Triangles[i][1]);
		face.vertexArray.push_back(Triangles[i][2]);
		mesh.AddFace(face);
	}

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

void FMeshBoolean::_Difference(Mesh& mesh, VoroCellInfo& result) {
	std::vector<Mesh*> inputMeshArray;
	inputMeshArray.push_back(&m_SourceMesh);
	inputMeshArray.push_back(&mesh);
	std::vector<Mesh*> outputMeshArray;
	
	MeshSetOperation meshOp(MW_FLAG_A_MINUS_B_SET_OP);
	(static_cast<MeshOperation*>(&meshOp))->Calculate(inputMeshArray, outputMeshArray);

	std::vector<FileObject*> fileObjectArray;
	for (Mesh* mesh : outputMeshArray)
		fileObjectArray.push_back(mesh);


	for (const FileObject* fileObject : fileObjectArray)
	{
		const Mesh* mesh = dynamic_cast<const Mesh*>(fileObject);

		_Merge(mesh,result);
	}
}

void FMeshBoolean::_Intersection(Mesh& mesh, VoroCellInfo& result) {
	std::vector<Mesh*> inputMeshArray;
	inputMeshArray.push_back(&m_SourceMesh);
	inputMeshArray.push_back(&mesh);
	std::vector<Mesh*> outputMeshArray;

	MeshSetOperation meshOp(MW_FLAG_INTERSECTION_SETP_OP);
	(static_cast<MeshOperation*>(&meshOp))->Calculate(inputMeshArray, outputMeshArray);

	std::vector<FileObject*> fileObjectArray;
	for (Mesh* mesh : outputMeshArray)
		fileObjectArray.push_back(mesh);

	for (const FileObject* fileObject : fileObjectArray)
	{
		const Mesh* mesh = dynamic_cast<const Mesh*>(fileObject);

		_Merge(mesh, result);
	}
}

void FMeshBoolean::_Union(Mesh& mesh, VoroCellInfo& result) {
	std::vector<Mesh*> inputMeshArray;
	inputMeshArray.push_back(&m_SourceMesh);
	inputMeshArray.push_back(&mesh);
	std::vector<Mesh*> outputMeshArray;

	MeshSetOperation meshOp(MW_FLAG_UNION_SET_OP);
	(static_cast<MeshOperation*>(&meshOp))->Calculate(inputMeshArray, outputMeshArray);

	std::vector<FileObject*> fileObjectArray;
	for (Mesh* mesh : outputMeshArray)
		fileObjectArray.push_back(mesh);

	for (const FileObject* fileObject : fileObjectArray)
	{
		const Mesh* mesh = dynamic_cast<const Mesh*>(fileObject);

		_Merge(mesh, result);
	}
}

void FMeshBoolean::_Merge(const Mesh*mesh, VoroCellInfo& result) {
	int totalVertices = mesh->GetNumVertices();
	int totalFaces = mesh->GetNumFaces();

	for (int i = 0; i < totalVertices; i++) {
		const Mesh::Vertex* v = mesh->GetVertex(i);
		result.Vertices.push_back(FVec3(v->point.x, v->point.y, v->point.z));
		result.Normals.push_back(FVec3(v->normal.x, v->normal.y, v->normal.z));
		result.UVs.push_back(FVec2(v->texCoords.x, v->texCoords.y));
	}

	for (int i = 0; i < totalFaces; i++) {
		const Mesh::Face* face = mesh->GetFace(i);
		result.Indices.push_back(face->vertexArray[2]);
		result.Indices.push_back(face->vertexArray[1]);
		result.Indices.push_back(face->vertexArray[0]);
	}

}

void FMeshBoolean::_Reset() {
	std::vector<Mesh::Vertex>* Vertices = new std::vector<Mesh::Vertex>(m_Vertices);
	std::vector<std::vector<size_t>>* Triangles = new std::vector<std::vector<size_t>>(m_Triangles);
	for (int i = 0; i < Vertices->size(); i++)
		m_SourceMesh.SetVertex(i, (*Vertices)[i]);
	for (int i = 0; i < Triangles->size(); i++) {
		Mesh::Face face;
		face.vertexArray.push_back((*Triangles)[i][0]);
		face.vertexArray.push_back((*Triangles)[i][1]);
		face.vertexArray.push_back((*Triangles)[i][2]);
		m_SourceMesh.SetFace(i, face);
	}
}