/* This is a direct replacement to the PhysX Simulation Manager - as we would have to re-register a new class against plasma and thats a faff

The model for physics so far seems to follow (its really more complicated but using this as a guide for now) :

plAvatar -> plPhysicalControllerCore -> plBTPhysicalControllerCore -> plSimulationMgr -> Engine

*/

#include "plSimulationMgr.h"
#include "plStatusLog/plStatusLog.h"

//#include "plLOSDispatch.h"
#include "plPhysical/plSimDefs.h"
#include "plPhysical/plPhysicsSoundMgr.h"

#include "plBTDebug.h"

#include "plBTPhysicalControllerCore.h"
#include "btBulletDynamicsCommon.h"

#define kDefaultMaxDelta    (0.15)        // if the step is greater than .15 seconds, clamp to that
#define kDefaultStepSize (1.f / 60.f)  // default simulation freqency is 60hz

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
	: fSuspended(true)
	, fAccumulator(0.0f)
	, fStepCount(0)
	//, fLOSDispatch(new plLOSDispatch())
	, fSoundMgr(new plPhysicsSoundMgr)
	, fLog(nil)
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

void plSimulationMgr::Advance(float delSecs) {

	if (fSuspended)
		return;

	fAccumulator += delSecs;

	if (fAccumulator > kDefaultStepSize)
	{
		return;
	}
	else if (fAccumulator > kDefaultMaxDelta)
	{
		//if (fExtraProfile)
		//	Log("Step clamped from %f to limit of %f", fAccumulator, kDefaultMaxDelta);
		fAccumulator = kDefaultMaxDelta;
	}

	++fStepCount;

	int numSubSteps = (int)(fAccumulator / kDefaultStepSize + 0.000001f);
	float delta = numSubSteps* kDefaultStepSize;
	fAccumulator -= delta;

	plBTPhysicalControllerCore::Apply(delta);

}

void plSimulationMgr::Shutdown() 
{
	// Trash the Simulation
}

btDiscreteDynamicsWorld* plSimulationMgr::GetScene(plKey world)
{

	if (!world)
		world = GetKey();

	if (!fScenes[world]) // don't have a scene
	{

		btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
		btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);
		btBroadphaseInterface* overlappingPairCache = new btDbvtBroadphase();
		btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;

		// create and set scene defaults here -- we might have to stash all the preconfig as well?
		btDiscreteDynamicsWorld* scene = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);

		scene->setGravity(btVector3(0, 0, -32.174049f)); // these are previous plasma defaults for PhysX and will not be right
		//scene->stepSimulation(btScalar(kDefaultStepTime));

		// set friction and restitution now occurs on the actual body
		
		plBTDebug *btDebug = new plBTDebug;

		btDebug->setDebugMode(btDebug->getDebugMode() | btIDebugDraw::DBG_DrawWireframe);
		scene->setDebugDrawer(btDebug);

		/*
		
		Collision Groups replacement - http://www.bulletphysics.org/mediawiki-1.5.8/index.php?title=Collision_Filtering

		for (int i = 0; i < plSimDefs::kGroupMax; i++)
		{
			scene->setGroupCollisionFlag(i, plSimDefs::kGroupAvatarBlocker, false);
			scene->setGroupCollisionFlag(i, plSimDefs::kGroupDynamicBlocker, false);
			scene->setGroupCollisionFlag(i, plSimDefs::kGroupLOSOnly, false);
			scene->setGroupCollisionFlag(plSimDefs::kGroupLOSOnly, i, false);
			scene->setGroupCollisionFlag(i, plSimDefs::kGroupAvatarKinematic, false);
		}

		scene->setGroupCollisionFlag(plSimDefs::kGroupAvatar, plSimDefs::kGroupAvatar, false);
		scene->setGroupCollisionFlag(plSimDefs::kGroupAvatar, plSimDefs::kGroupAvatarBlocker, true);
		scene->setGroupCollisionFlag(plSimDefs::kGroupDynamic, plSimDefs::kGroupDynamicBlocker, true);
		scene->setGroupCollisionFlag(plSimDefs::kGroupAvatar, plSimDefs::kGroupStatic, true);
		scene->setGroupCollisionFlag(plSimDefs::kGroupStatic, plSimDefs::kGroupAvatar, true);
		scene->setGroupCollisionFlag(plSimDefs::kGroupAvatar, plSimDefs::kGroupDynamic, true);

		// Kinematically controlled avatars interact with detectors and dynamics
		scene->setGroupCollisionFlag(plSimDefs::kGroupAvatarKinematic, plSimDefs::kGroupDetector, true);
		scene->setGroupCollisionFlag(plSimDefs::kGroupAvatarKinematic, plSimDefs::kGroupDynamic, true);

		// The dynamics are in actor group 1, everything else is in 0.  Request
		// a callback for whenever a dynamic touches something.
		scene->setActorGroupPairFlags(0, 1, NX_NOTIFY_ON_TOUCH);
		scene->setActorGroupPairFlags(1, 1, NX_NOTIFY_ON_TOUCH);*/

		// Defaults for Collision, Materials and other things go here
		fScenes[world] = scene;
		return scene;
	}
	else 
	{
		btDiscreteDynamicsWorld* scene = fScenes[world];
		return scene;
	}

	
}