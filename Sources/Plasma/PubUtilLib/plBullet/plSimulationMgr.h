
#ifndef plSimulationMgr_H
#define plSimulationMgr_H

#include <map>
#include "pnKeyedObject/hsKeyedObject.h"
#include "hsTemplates.h"

class btDiscreteDynamicsWorld;
class plStatusLog;
class plLOSDispatch;
class plPhysicsSoundMgr;


class plSimulationMgr : public hsKeyedObject
{
public:
	CLASSNAME_REGISTER(plSimulationMgr);
	GETINTERFACE_ANY(plSimulationMgr, hsKeyedObject);

	static plSimulationMgr* GetInstance();
	static void Init();
	static void Shutdown();

	static bool fExtraProfile;

	static bool InitSimulation();

	void Advance(float delSecs);

	plSimulationMgr();
	virtual ~plSimulationMgr();

	// Output the given debug text to the simulation log.
	static void Log(const char* formatStr, ...);
	static void LogV(const char* formatStr, va_list args);
	static void ClearLog();

	//GetSDK() const { return fSDK; } - this is redundent
	btDiscreteDynamicsWorld* GetScene(plKey world);

protected:

	plPhysicsSoundMgr* fSoundMgr;

	typedef std::map<plKey, btDiscreteDynamicsWorld*> SceneMap;
	SceneMap fScenes;

	//plLOSDispatch* fLOSDispatch;

	bool fSuspended;
	float fAccumulator;
	uint32_t fStepCount;

	//fSDK; - this is redundent
	plStatusLog *fLog;
};



#endif