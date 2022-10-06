#ifndef FACTOR_H
#define FACTOR_H
#include "PxPhysicsAPI.h"
#include "Common/MeshData.h"
#include <string>
using namespace physx;
struct FChunk {
	float m_Volume;
	PxVec3 m_Center;
	PxRigidDynamic* m_pPxRigidActor;
	float m_Life;
	bool m_IsSleeping;
};

class FRenderMesh;
class FWireMesh;
class FScene;
class FActor
{
public:
	FActor();
	~FActor();

	bool Init(char * name);
	bool Release();
	bool ReInit();

	bool AddActor();
	bool RemoveActor();
	bool RemoveAllActors();
	bool Update();
	
	bool OnEnterScene(FScene* scene);
	bool OnLeaveScene(FScene* scene);
	
	bool SetRenderWireFrame();

private:
	FRenderMesh* m_pRenderMesh;
	FWireMesh* m_pWireMesh;

	Graphics::MeshData* m_pMeshData;
	Graphics::MeshData* m_pVoroMeshData;

	PxMaterial* m_pMaterial;

	char* m_Name;
};
#endif // FACTOR_H


