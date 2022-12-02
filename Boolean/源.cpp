#include <iostream>
#include <string>
#include <fstream>
#include "FAccelerator.h"
#include "FGeometryCollection.h"
#include "IO.h"
#include "IntersectUtils.h"
using namespace std;

int main() {

	string file = "bunny";
	string outputDir = file+".vtk";
	string input = file+".obj";
	ifstream is;
	FMeshData meshA;
	FMeshData meshB;
	is.open(input);
	if (!is.is_open()) {
		cout << "fail to open the file" << endl;
		return -1;
	}
	readTri(is, meshA, 3);
	is.close();
	is.open("BoxA.obj");
	if (!is.is_open()) {
		cout << "fail to open the file" << endl;
		return -1;
	}

	readTri(is, meshB, 1);
	is.close();
	FBoundingBox boxA(meshA.m_Vertices);
	FBoundingBox boxB(meshB.m_Vertices);

	FBoxAccelerator accel(boxA, meshA);
	printf("accel\n");
	FMeshData mesh = accel.CollecteTriangles(boxB);
	FGeometryCollection collection(boxB, meshB);
	printf("collect\n");

	collection.SetMeshB(mesh);
	FMeshData output;
		output = collection.FetchResult(CollectionType::INTERSECT);

	if (writeVtk(outputDir, output.m_Vertices, output.m_Triangles))
		printf("write success!-----------------\n");


	//IntersectEdge* edge = nullptr;
	//FVec3 v0(-1, -1, -1);
	//FVec3 v1(-1, 1, -1);
	//FVec3 v2(1, 1, 1);
	//FVec3 v3(1, 1, -1);
	//FVec3 v4(1, -1, -1);
	//FVec3 v5(-1, 1, 1);
	//FVec3 v6(-1, -1, 1);
	//FVec3 v7(1, -1, 1);

	//FTriangle triA(v0, v1, v2);
	//FTriangle triB(v3, v4, v5);
	////FTriangle triA(v0, v1, v3);
	////FTriangle triB(v3, v4, v1);
	//FTriangle triC(v0, v7, v2);
	//FTriangle triD(v6, v4, v5);

	//FMeshData testA;
	//FMeshData testB;
	//testA.m_Triangles.push_back(triA);
	////testA.m_Triangles.push_back(triC);
	//testB.m_Triangles.push_back(triB);
	////testB.m_Triangles.push_back(triD);
	//testA.m_Vertices.push_back(triA.i());
	//testA.m_Vertices.push_back(triA.j());
	//testA.m_Vertices.push_back(triA.k());
	//testA.m_Vertices.push_back(triC.j());

	//testB.m_Vertices.push_back(triB.i());
	//testB.m_Vertices.push_back(triB.j());
	//testB.m_Vertices.push_back(triB.k());
	//testB.m_Vertices.push_back(triD.i());


	//FGeometryCollection collection2(triB.box, testB);
	//printf("collect2\n");

	//collection2.SetMeshB(testA);
	//FMeshData output2 = collection2.FetchResult(CollectionType::INTERSECT);

	//if (writeVtk("test.vtk", output2.m_Vertices, output2.m_Triangles))
	//	printf("write success!-----------------\n");
	//if (IntersectUtils::TrianglesIntersect(triA, triB, edge))
	//	printf("intersect+++++++++++++++++++++++++++++++]\n");
    
}