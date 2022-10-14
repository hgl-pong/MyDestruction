#ifndef FCONNECT_GRAPH_H
#define FCONNECT_GRAPH_H
#include <map>
#include <set>
#include<vector>

#include<iostream>
#include<string>
#include<queue>
#include<stack>

using namespace std;
template <typename T>
class GraphEdge {
public:
	T m_Node;
	float m_ConnectHealth;

	GraphEdge(T neighbour_vertex) {
		this->m_Node = neighbour_vertex;
		this->m_ConnectHealth = 0;
	}

	GraphEdge(T neighbour_vertex, float weight) {
		this->m_Node = neighbour_vertex;
		this->m_ConnectHealth = weight;
	}

	bool operator<(const GraphEdge& obj) const {
		return m_Node < obj.m_Node;
	}

	bool operator==(const GraphEdge& obj) const {
		return obj.m_Node == m_Node;
	}
};

template <typename T>
class FConnectGraph
{
public:
	FConnectGraph() = default;
	FConnectGraph(std::map<T, std::set<GraphEdge<T>>>& map);

	inline bool Contains(const T& u);
	inline bool Adjacent(const T& u, const T& v);

	inline void AddNode(const T& u);
	inline void AddEdge(const T& u, const T& v, float weight);

	inline void ChangeHealth(const T& u, const T& v, float weight);

	inline void RemoveNode(const T& u);
	inline void RemoveEdge(const T& u, const T& v);

	inline int Degree(const T& u);
	inline int NodeSize();
	inline int EdgeSize();
	inline int LargestDegree();

	inline float GetWeight(const T& u, const T& v);
	inline std::vector<T> GetNodes();
	inline std::map<T, float> GetNeighbors(const T& u);

	inline std::map<T, std::set<GraphEdge<T>>> BFSSearch(const T& u, bool deleteNode = false);
	inline bool IsIsolated(const T& u);
	inline std::vector<T> CollectAllIsolatedNode();
	inline std::vector<std::map<T, std::set<GraphEdge<T>>>> CollectAllCluster();
public:
	std::map<T, std::set<GraphEdge<T>>> m_Container;
};

template<typename T>
inline FConnectGraph<T>::FConnectGraph(std::map<T,std::set<GraphEdge<T>>>& map)
{
	m_Container = map;
}

template <typename T>
inline bool FConnectGraph<T>::Contains(const T& u) {
	return m_Container.find(u) != m_Container.end();
}

template <typename T>
inline bool FConnectGraph<T>::Adjacent(const T& u, const T& v) {
	if (Contains(u) && Contains(v) && u != v) {
		for (auto edge : m_Container[u])
			if (edge.m_Node == v)
				return true;
	}
	return false;
}

template <typename T>
inline void FConnectGraph<T>::AddNode(const T& u) {
	if (!Contains(u)) {
		set<GraphEdge<T>> edge_list;
		m_Container[u] = edge_list;
	}
}

template <typename T>
inline void FConnectGraph<T>::AddEdge(const T& u, const T& v, float weight) {
	if (!Adjacent(u, v)) {
		m_Container[u].insert(GraphEdge<T>(v, weight));
		m_Container[v].insert(GraphEdge<T>(u, weight));
	}
}

template <typename T>
inline void FConnectGraph<T>::ChangeHealth(const T& u, const T& v, float weight) {
	if (Contains(u) && Contains(v)) {
		if (m_Container[u].find(GraphEdge<T>(v)) != m_Container[u].end()) {
			m_Container[u].erase(GraphEdge<T>(v));
			m_Container[u].insert(GraphEdge<T>(v, weight));
		}

		if (m_Container[v].find(GraphEdge<T>(u)) != m_Container[v].end()) {
			m_Container[v].erase(GraphEdge<T>(u));
			m_Container[v].insert(GraphEdge<T>(u, weight));
		}
	}
}

template <typename T>
inline void FConnectGraph<T>::RemoveNode(const T& u) {
	if (Contains(u)) {
		for (auto& vertex : m_Container) {
			if (vertex.second.find(GraphEdge<T>(u)) != vertex.second.end())
				vertex.second.erase(GraphEdge<T>(u));
		}
		m_Container.erase(u);
	}
}

template <typename T>
inline void FConnectGraph<T>::RemoveEdge(const T& u, const T& v) {
	if (u == v || !Contains(u) || !Contains(v)) return;

	if (m_Container[u].find(GraphEdge<T>(v)) != m_Container[u].end()) {
		m_Container[u].erase(GraphEdge<T>(v));
		m_Container[v].erase(GraphEdge<T>(u));
	}
}


template <typename T>
inline int FConnectGraph<T>::Degree(const T& u) {
	if (Contains(u)) return m_Container[u].size();

	return -1; // 度数为-1说明图中没有该顶点
}

template <typename T>
inline int FConnectGraph<T>::NodeSize() {
	return m_Container.size();
}

template <typename T>
inline int FConnectGraph<T>::EdgeSize() {
	int count = 0;
	set<GraphEdge<T>> vertex_set;

	for (auto vertex : m_Container) {
		vertex_set.insert(GraphEdge<T>(vertex.first, 0));
		for (auto edge : vertex.second) {
			if (vertex_set.find(edge) != vertex_set.end()) continue;
			count++;
		}
	}
	return count;
}

template <typename T>
inline int FConnectGraph<T>::LargestDegree() {
	if (NodeSize() == 0) return 0;

	unsigned max_degree = 0;
	for (auto vertex : m_Container) {
		if (vertex.second.size() > max_degree)
			max_degree = vertex.second.size();
	}
	return max_degree;
}

template <typename T>
inline float  FConnectGraph<T>::GetWeight(const T& u, const T& v) {
	if (Contains(u) && Contains(v)) {
		for (GraphEdge<T> edge : m_Container[u])
			if (edge.m_Node == v) return edge.m_ConnectHealth;
	}
	return -1;
}

template <typename T>
inline std::vector<T> FConnectGraph<T>::GetNodes() {
	std::vector<T> vertices;
	for (auto vertex : m_Container)
		vertices.push_back(vertex.first);

	return vertices;
}

template <typename T>
inline std::map<T, float> FConnectGraph<T>::GetNeighbors(const T& u) {
	map<T, float> neighbours;

	if (Contains(u)) {
		for (GraphEdge<T> edge : m_Container[u])
			neighbours[edge.m_Node] = edge.m_ConnectHealth;
	}

	return neighbours;
}


template<typename T>
inline std::map<T, std::set<GraphEdge<T>>> FConnectGraph<T>::BFSSearch(const T& u, bool deleteNode) {
	std::map<T, std::set<GraphEdge<T>>> cluster;
	std::queue<T> buffer;
	buffer.push(u);
	while (!buffer.empty()) {
		T node = buffer.front();
		buffer.pop();
		if (Contains(node)) {
			auto it = m_Container.find(node);
			for (GraphEdge<T> edge : it->second) {
				if (cluster.find(edge.m_Node) == cluster.end()) {
					buffer.push(edge.m_Node);
				}
			}
			cluster.emplace(*it);
			if (deleteNode)
				RemoveNode(it->first);
		}
	}
	return cluster;
}

template<typename T>
inline bool FConnectGraph<T>::IsIsolated(const T& u) {
	auto it = m_Container.find(u);
	if (it == m_Container.end())
		return false;
	return it->second.empty();
}

template<typename T>
inline std::vector<T> FConnectGraph<T>::CollectAllIsolatedNode() {
	std::vector<T> buffer;
	for (auto node : m_Container)
	{
		if (IsIsolated(node.first))
			buffer.push_back(node.first);
	}
	return buffer;
}

template <typename T>
inline std::vector<std::map<T, std::set<GraphEdge<T>>>> FConnectGraph<T>::CollectAllCluster() {
	std::vector<std::map<T, std::set<GraphEdge<T>>>> buffer;
	while (!m_Container.empty())
		buffer.push_back(BFSSearch(m_Container.begin()->first, true));
	return buffer;
}

#endif//FCONNECT_GRAPH_H