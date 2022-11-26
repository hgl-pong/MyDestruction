//
// Created by Administrator on 2019/3/11 0011.
//
/**
 * 管理mesh的io
 */
#pragma once
#ifndef HALFEDGE_DATA_LOADER_H
#define HALFEDGE_DATA_LOADER_H
#include "FBoundingBox.h"
#include <iostream>     // std::cin, std::cout
#include <fstream>      // std::ifstream
#include <vector>
#include <string>
#include <unordered_map>
using namespace std;

int readTri(ifstream& is, FMeshData& mesh,float scale=1) {
    vector<FVertex> points;
    vector<FTriangle> triangles;
    vector<FEdge> edges;
    int pcount = 0, tricount = 0;
    string s;
    char c;
    while (is.get(c)) {
        switch (c) {
        case '#':
            getline(is, s);
            //if (s.find("Vertices") != string::npos) {
            //    points.resize(size_t (stoll(s.substr(11))));
            //}
            //if (s.find("Faces") != string::npos) {
            //    triangles.resize(size_t (stoll(s.substr(8))));
            //}
            break;
        case 'v':
        {
            FVertex p;
            //p.index = pcount;
            is >> p.position.X >> p.position.Y >> p.position.Z;
            is.get();
            p.position = p.position * scale;
            points.push_back(p);
            pcount++;
            //points[pcount++] = p;
            break;
        }
        case 'f':
        {
            FTriangle triangle;
            int x, y, z;
            is >> x >> y >> z;
            is.get();
            if (!(x <= points.size() && y <= points.size() && z <= points.size())) {
                cout << "数组越界：x=" << x << "，y=" << y << "，z=" << z << endl;
                cout << "maxSize=" << points.size() << endl;
                exit(-1);
            }
            if (tricount == 18429) {
                cout << endl;
            }
            triangle = FTriangle(points[x - 1], points[y - 1], points[z - 1]);
            triangles.push_back(triangle);
            tricount++;
            //triangles[tricount++] = triangle;
            break;
        }
        default: {
            break;
        }
        }
    }
    mesh.m_Triangles = triangles;
    mesh.m_Vertices = points;
    return 0;
}

int writeVtk(string path, vector<FVertex> points, vector<FTriangle> triangles) {
    if (points.empty())
        return 0;
    ofstream of("result.obj");
    vector<float > nodes;
    vector<int> faces;
    std::unordered_map<FVertex, int> indexMap;
    for (FIndex i = 0; i < points.size(); i++) {
        of << "v " << points[i].position.X << " " << points[i].position.Y << " " << points[i].position.Z << endl;
        nodes.push_back(points[i].position.X);
        nodes.push_back(points[i].position.Y);
        nodes.push_back(points[i].position.Z);
        indexMap.emplace(std::make_pair(points[i], i));
    }
    of << endl;
    for (FIndex i = 0; i < triangles.size(); i++) {
		faces.push_back(indexMap[triangles[i].i()]);
		faces.push_back(indexMap[triangles[i].j()]);
		faces.push_back(indexMap[triangles[i].k()]);
		of << "f " << indexMap[triangles[i].i()] << " " << indexMap[triangles[i].j()] << " " << indexMap[triangles[i].k()] << endl;

    }
    of.close();
    ofstream os(path.c_str());
    tri2vtk(os, &nodes[0], points.size(), &faces[0], triangles.size());
    return 1;
}
#endif
