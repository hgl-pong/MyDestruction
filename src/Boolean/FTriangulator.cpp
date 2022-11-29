#include "FTriangulator.h"
#include <queue>
#include <iostream>
#include "earcut.hpp"
#include <array>
bool FTriangulator::Triangulating(FTriangle& triangle, std::vector<FVertex>& points, std::unordered_map<FIndex, std::unordered_set<FIndex>>& neighborMapFrom3, std::vector<FTriangle>& triangles)
{
    FTriangulator triangulator(triangle);
    std::vector<FVertex> pointsBuffer;
	pointsBuffer.push_back(triangle.i());
	pointsBuffer.push_back(triangle.j());
	pointsBuffer.push_back(triangle.k());
    for (auto& point : points)
        pointsBuffer.push_back(point);

	triangulator.SetEdges(pointsBuffer, &neighborMapFrom3);
	if (!triangulator.ReTriangulate())
		return false;

    FVec3 normal = Normal(triangle);
    std::vector<std::vector<FIndex>> trianglesIndex = triangulator.GetTriangles();
    for (auto triangle : trianglesIndex) {
        FTriangle newTriangle(pointsBuffer[triangle[0]], pointsBuffer[triangle[1]], pointsBuffer[triangle[2]]);
        auto d = normal.Dot(Normal(newTriangle));
        if (d < 0) {
            std::swap(newTriangle.v[1], newTriangle.v[2]);
        }
        triangles.push_back(newTriangle);
    }

}

FTriangulator::FTriangulator(FTriangle& triangle)
{
    FVertex* points = triangle.v;
	m_projectNormal = (points[1].position - points[0].position).Cross(points[2].position - points[0].position).Normalize();
	m_projectAxis = (points[1].position - points[0].position).Normalize();
	m_projectOrigin = points[0].position;

	m_points = Project(points, m_projectNormal, m_projectAxis, m_projectOrigin);
}

FTriangulator::FTriangulator( std::vector<FVertex>& points) 
{   

    m_projectNormal= (points[1].position - points[0].position).Cross(points[2].position - points[0].position).Normalize();
    m_projectAxis = (points[1].position - points[0].position).Normalize();
    m_projectOrigin = points[0].position;

    m_points=Project(points.data(),m_projectNormal, m_projectAxis, m_projectOrigin);
}

void FTriangulator::SetEdges(std::vector<FVertex>& points,
    std::unordered_map<FIndex, std::unordered_set<FIndex>>* neighborMapFrom3)
{
    m_points= Project(points, m_projectNormal, m_projectAxis, m_projectOrigin);
    m_neighborMapFrom3 = neighborMapFrom3;
}


void FTriangulator::LookupPolylinesFromNeighborMap(const std::unordered_map<FIndex, std::unordered_set<FIndex>>& neighborMap)
{
    std::vector<FIndex> endpoints;
    endpoints.reserve(neighborMap.size());
    for (const auto& it : neighborMap) {
        if (it.second.size() == 1) {
            endpoints.push_back(it.first);
        }
    }
    for (const auto& it : neighborMap) {
        if (it.second.size() > 1) {
            endpoints.push_back(it.first);
        }
    }

    std::unordered_set<FIndex> visited;
    for (const auto& startEndpoint : endpoints) {
        if (visited.find(startEndpoint) != visited.end())
            continue;
        std::queue<FIndex> q;
        q.push(startEndpoint);
        std::vector<FIndex> polyline;
        while (!q.empty()) {
            FIndex loop = q.front();
            visited.insert(loop);
            polyline.push_back(loop);
            q.pop();
            auto neighborIt = neighborMap.find(loop);
            if (neighborIt == neighborMap.end())
                break;
            for (const auto& it : neighborIt->second) {
                if (visited.find(it) == visited.end()) {
                    q.push(it);
                    break;
                }
            }
        }
        if (polyline.size() >= 3) {
            auto neighborOfLast = neighborMap.find(polyline.back());
            if (neighborOfLast->second.find(startEndpoint) != neighborOfLast->second.end()) {
                m_innerPolygons.push_back(polyline);
                continue;
            }
        }
        if (polyline.size() >= 2)
            m_polylines.push_back(polyline);
    }
}

int FTriangulator::AttachPointToTriangleEdge( FVec2& point)
{
    for (int i = 0; i < 3; ++i) {
        int j = (i + 1) % 3;
        if (point.IsOnLine(m_points[i], m_points[j]))
            return i;
    }
    return -1;
}

void FTriangulator::BuildPolygonHierarchy()
{
    for (FIndex i = 0; i < m_innerPolygons.size(); ++i) {
        for (FIndex j = i + 1; j < m_innerPolygons.size(); ++j) {
            if (m_points[m_innerPolygons[i][0]].IsInPolygon(m_points, m_innerPolygons[j])) {
                m_innerChildrenMap[j].insert(i);
                m_innerParentsMap[i].insert(j);
            }
            else if (m_points[m_innerPolygons[j][0]].IsInPolygon(m_points, m_innerPolygons[i])) {
                m_innerChildrenMap[i].insert(j);
                m_innerParentsMap[j].insert(i);
            }
        }
    }

    for (FIndex i = 0; i < m_innerPolygons.size(); ++i) {
        const auto& inner = m_innerPolygons[i];
        if (m_innerParentsMap.find(i) != m_innerParentsMap.end())
            continue;
        for (FIndex j = 0; j < m_polygons.size(); ++j) {
            if (m_points[inner[0]].IsInPolygon(m_points, m_polygons[j])) {
                m_polygonHoles[j].push_back(i);
            }
        }
    }
}

bool FTriangulator::BuildPolygons()
{
    struct EdgePoint
    {
        FIndex pointIndex = 0;
        FIndex linkToPointIndex = 0;
        int polylineIndex = -1;
        bool reversed = false;
        float squaredDistance = 0.0;
        int linkTo = -1;
    };
    std::vector<std::vector<EdgePoint>> edgePoints(3);
    for (int polylineIndex = 0; polylineIndex < (int)m_polylines.size(); ++polylineIndex) {
        const auto& polyline = m_polylines[polylineIndex];
        int frontEdge = AttachPointToTriangleEdge(m_points[polyline.front()]);
        int backEdge = AttachPointToTriangleEdge(m_points[polyline.back()]);
		if (-1 == frontEdge || -1 == backEdge) {
			std::cout << "Attach point to triangle edge failed" << std::endl;
			return false;
		}
        edgePoints[frontEdge].push_back({
            polyline.front(),
            polyline.back(),
            polylineIndex,
            false,
            (m_points[polyline.front()] - m_points[frontEdge]).LengthSqr()
            });
        edgePoints[backEdge].push_back({
            polyline.back(),
            polyline.front(),
            polylineIndex,
            true,
            (m_points[polyline.back()] - m_points[backEdge]).LengthSqr()
            });
    }
    for (auto& it : edgePoints) {
        std::sort(it.begin(), it.end(), [](const EdgePoint& first, const EdgePoint& second) {
            return first.squaredDistance < second.squaredDistance;
        });
    }

    // Turn triangle to ring
    std::vector<EdgePoint> ringPoints;
    for (FIndex i = 0; i < 3; ++i) {
        ringPoints.push_back({ i });
        for (const auto& it : edgePoints[i])
            ringPoints.push_back(it);
    }

    // Make polyline link
    std::unordered_map<FIndex, FIndex> pointRingPositionMap;
    for (FIndex i = 0; i < ringPoints.size(); ++i) {
        const auto& it = ringPoints[i];
        if (-1 == it.polylineIndex)
            continue;
        pointRingPositionMap.insert({ it.pointIndex, i });
    }
    for (FIndex i = 0; i < ringPoints.size(); ++i) {
        auto& it = ringPoints[i];
        if (-1 == it.polylineIndex)
            continue;
        auto findLinkTo = pointRingPositionMap.find(it.linkToPointIndex);
        if (findLinkTo == pointRingPositionMap.end())
            continue;
        it.linkTo = findLinkTo->second;
    }

    std::unordered_set<FIndex> visited;
    std::queue<FIndex> startQueue;
    startQueue.push(0);
    while (!startQueue.empty()) {
        auto startIndex = startQueue.front();
        startQueue.pop();
        if (visited.find(startIndex) != visited.end())
            continue;
        std::vector<FIndex> polygon;
        auto loopIndex = startIndex;
        do {
            auto& it = ringPoints[loopIndex];
            visited.insert(loopIndex);
            if (-1 == it.polylineIndex) {
                polygon.push_back(it.pointIndex);
                loopIndex = (loopIndex + 1) % ringPoints.size();
            }
            else if (-1 != it.linkTo) {
                const auto& polyline = m_polylines[it.polylineIndex];
                if (it.reversed) {
                    for (int i = (int)polyline.size() - 1; i >= 0; --i)
                        polygon.push_back(polyline[i]);
                }
                else {
                    for (const auto& pointIndex : polyline)
                        polygon.push_back(pointIndex);
                }
                startQueue.push((loopIndex + 1) % ringPoints.size());
                loopIndex = (it.linkTo + 1) % ringPoints.size();
            }
            else {
                std::cerr << "linkTo failed" << std::endl;
                return false;
            }
        } while (loopIndex != startIndex);
        m_polygons.push_back(polygon);
    }

    return true;
}

void FTriangulator::Triangulate()
{
    for (FIndex polygonIndex = 0; polygonIndex < m_polygons.size(); ++polygonIndex) {
        const auto& polygon = m_polygons[polygonIndex];

        std::vector<std::vector<std::array<float, 2>>> polygonAndHoles;
        std::vector<FIndex> pointIndices;

        std::vector<std::array<float, 2>> border;
        for (const auto& it : polygon) {
            pointIndices.push_back(it);
            const auto& v = m_points[it];
            border.push_back(std::array<float, 2> {v.X, v.Y});
        }
        polygonAndHoles.push_back(border);

        auto findHoles = m_polygonHoles.find(polygonIndex);
        if (findHoles != m_polygonHoles.end()) {
            for (const auto& h : findHoles->second) {
                std::vector<std::array<float, 2>> hole;
                for (const auto& it : m_innerPolygons[h]) {
                    pointIndices.push_back(it);
                    const auto& v = m_points[it];
                    hole.push_back(std::array<float, 2> {v.X, v.Y});
                }
                polygonAndHoles.push_back(hole);
            }
        }

        std::vector<FIndex> indices = mapbox::earcut<FIndex>(polygonAndHoles);
        m_triangles.reserve(indices.size() / 3);
        for (FIndex i = 0; i < indices.size(); i += 3) {
            m_triangles.push_back({
                pointIndices[indices[i]],
                pointIndices[indices[i + 1]],
                pointIndices[indices[i + 2]]
                });
        }
    }

    for (FIndex polygonIndex = 0; polygonIndex < m_innerPolygons.size(); ++polygonIndex) {
        const auto& polygon = m_innerPolygons[polygonIndex];

        std::vector<std::vector<std::array<float, 2>>> polygonAndHoles;
        std::vector<FIndex> pointIndices;

        std::vector<std::array<float, 2>> border;
        for (const auto& it : polygon) {
            pointIndices.push_back(it);
            const auto& v = m_points[it];
            border.push_back(std::array<float, 2> {v.X, v.Y});
        }
        polygonAndHoles.push_back(border);

        auto childrenIt = m_innerChildrenMap.find(polygonIndex);
        if (childrenIt != m_innerChildrenMap.end()) {
            auto children = childrenIt->second;
            for (const auto& child : childrenIt->second) {
                auto grandChildrenIt = m_innerChildrenMap.find(child);
                if (grandChildrenIt != m_innerChildrenMap.end()) {
                    for (const auto& grandChild : grandChildrenIt->second) {
                        std::cout << "Grand child removed:" << grandChild << std::endl;
                        children.erase(grandChild);
                    }
                }
            }
            for (const auto& child : children) {
                std::vector<std::array<float, 2>> hole;
                for (const auto& it : m_innerPolygons[child]) {
                    pointIndices.push_back(it);
                    const auto& v = m_points[it];
                    hole.push_back(std::array<float, 2> {v.X, v.Y});
                }
                polygonAndHoles.push_back(hole);
            }
        }

        std::vector<FIndex> indices = mapbox::earcut<FIndex>(polygonAndHoles);
        m_triangles.reserve(indices.size() / 3);
        for (FIndex i = 0; i < indices.size(); i += 3) {
            m_triangles.push_back({
                pointIndices[indices[i]],
                pointIndices[indices[i + 1]],
                pointIndices[indices[i + 2]]
                });
        }
    }
}

bool FTriangulator::ReTriangulate()
{
    LookupPolylinesFromNeighborMap(*m_neighborMapFrom3);
    if (!BuildPolygons()) {
        std::cout << "Build polygons failed" << std::endl;
        return false;
    }
    BuildPolygonHierarchy();
    Triangulate();
    return true;
}

const std::vector<std::vector<FIndex>>& FTriangulator::GetPolygons() const
{
    return m_polygons;
}

const std::vector<std::vector<FIndex>>& FTriangulator::GetTriangles() const
{
    return m_triangles;
}

