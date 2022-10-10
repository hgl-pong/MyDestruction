#ifndef FFRACTURE_GRAPH_H
#define FFRACTURE_GRAPH_H
#include"FDamage.h"
#include <unordered_set>
#include <vector>
#include "PxPhysicsAPI.h"
#include "Utils.h"
using namespace physx;
class FChunk;
class FActor;

struct GraphEdge {
	FChunk* chunkA;
	FChunk* chunkB;
	float connectHealth;
};
template<typename T>
inline size_t ChunkHash(const T& valA, const T& valB, const float& i)
{
	return std::hash<T>()(valA) ^ std::hash<T>()(valB) ^ std::hash<float>()(i);
};
namespace std
{
	template<>
	struct hash<GraphEdge>
	{
		std::size_t operator()(const GraphEdge& edge) const
		{
			size_t num = ChunkHash(edge.chunkA, edge.chunkA, edge.connectHealth);
			return num;
		}
	};
	template<>
		struct equal_to<GraphEdge>
		{
			bool operator()(const GraphEdge& e1, const GraphEdge& e2) const
			{
				return (e1.chunkA == e2.chunkA &&
					e1.chunkB == e2.chunkB &&
					e1.connectHealth == e2.connectHealth)||
					(e1.chunkA == e2.chunkB &&
						e1.chunkB == e2.chunkA &&
						e1.connectHealth == e2.connectHealth);
			}
		};
}
class FChunkCluster {
public:
	FChunkCluster() = default;
	FChunkCluster(FActor* actor);
	~FChunkCluster();
	bool Release();
	bool Intersection(FDamage& damage);
	bool Seperate(FChunk* chunk);
	int Size();
	bool InitSharedPhysicsActor();
	PxRigidDynamic* GetPhysicsActor();
	bool Tick();
	bool Init(std::vector<FChunk*>& chunks, FActor* actor);
private:
	FActor* m_pActor;

	PxRigidDynamic* m_pRigidActor;
	PxTransform m_Transform;

	int m_ChunkCount;
	std::vector<bool> m_Connecting;
	std::unordered_set<GraphEdge> m_Edges;
};


#endif//FFRACTURE_GRAPH_H