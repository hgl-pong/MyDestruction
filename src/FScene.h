#ifndef FSCENE_H
#define FSCENE_H
class FScene
{
public:
	FScene();
	~FScene();
	
	bool Init();
	bool Release();
	bool ReInit();

	bool AddActor();
	bool RemoveActor();
	bool RemoveAllActors();
	bool Update();

	bool Simulate(bool simulate);

};
#endif // FSCENE_H


