#ifndef FFRACTURE_GRAPH_H
#define FFRACTURE_GRAPH_H
#include"FDamage.h"
#include <set>
#include <vector>
class FChunk;
class FActor;

class FFractureGraph {
public:
	FFractureGraph() = default;
	~FFractureGraph();
	bool Release();
	bool Intersection(FDamage& damage);
	bool Init(std::vector<FChunk*>&chunks);
	bool Remove(FChunk* chunk);
	int Size();
private:
	struct Edge {
		FChunk* chunkA;
		FChunk* chunkB;
	};
	int m_ChunkCount;
	std::set<FChunk*>m_Nodes;
	std::set<Edge*> m_Edges;
};

#endif//FFRACTURE_GRAPH_H