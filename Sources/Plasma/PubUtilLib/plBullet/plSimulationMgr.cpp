/* This is a direct replacement to the PhysX Simulation Manager - as we would have to re-register a new class against plasma and thats a faff

The model for physics so far seems to follow (its really more complicated but using this as a guide for now) :

plAvatar -> plPhysicalControllerCore -> plBTPhysicalControllerCore -> plSimulationMgr -> Engine

*/

#include "plSimulationMgr.h"
#include "plStatusLog/plStatusLog.h"

#include "btBulletDynamicsCommon.h"

static plSimulationMgr* gTheInstance = NULL;

void plSimulationMgr::Init()
{

	gTheInstance = new plSimulationMgr();
	if (gTheInstance->InitSimulation())
	{
		gTheInstance->RegisterAs(kSimulationMgr_KEY); // become the simulation manager
	}
	else
	{
		delete gTheInstance; //clean up memory
		gTheInstance = nil;
	}
}



plSimulationMgr::plSimulationMgr()
	:fLog(nil)
{

}

plSimulationMgr::~plSimulationMgr() 
{

}

plSimulationMgr* plSimulationMgr::GetInstance()
{
	return gTheInstance;
}

bool plSimulationMgr::InitSimulation()
{
	// Bullet is our world now - no more installation of Runtimes :D

	plStatusLog* fLog = plStatusLogMgr::GetInstance().CreateStatusLog(40, "Simulation.log", plStatusLog::kFilledBackground | plStatusLog::kAlignToTop);
	fLog->AddLine("Initialized Bullet Simulation Manager");

	return true;

}

void plSimulationMgr::Shutdown() 
{
	// Trash the Simulation
}

btDiscreteDynamicsWorld* plSimulationMgr::GetScene(plKey world)
{

	if (!world)
		world = GetKey();

	btDiscreteDynamicsWorld* scene = fScenes[world];

	if (!scene) {

		btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
		btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);
		btBroadphaseInterface* overlappingPairCache = new btDbvtBroadphase();
		btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;

		// create and set scene defaults here -- we might have to stash all the preconfig as well?
		btDiscreteDynamicsWorld* scene = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);

		scene->setGravity(btVector3(0, 0, -32.174049f)); // these are previous plasma defaults for PhysX and will not be right

		// Defaults for Collision, Materials and other things go here

		fScenes[world] = scene;
	}

	return scene;
}