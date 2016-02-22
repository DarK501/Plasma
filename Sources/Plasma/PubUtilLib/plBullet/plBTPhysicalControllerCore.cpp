
#include "plBTPhysical.h"
#include "plBTPhysicalControllerCore.h"
#include "plSimulationMgr.h"

#include "pnSceneObject/plSceneObject.h"

// bullet includes
#include "btBulletDynamicsCommon.h"

plBTPhysicalControllerCore::plBTPhysicalControllerCore(plKey ownerSO, float height, float radius, bool human)
:plPhysicalControllerCore(ownerSO, height, radius) 
{

	//create controller

}

plBTPhysicalControllerCore::~plBTPhysicalControllerCore()
{

	// destroy controller
}

void plBTPhysicalControllerCore::ICreateController(const hsPoint3& pos) 
{
	
	btDiscreteDynamicsWorld* scene = plSimulationMgr::GetInstance()->GetScene(fWorldKey);
	
	if (fKinematicCCT) {
		
		// We are a charater controller and therefore should be a capsual
		btCollisionShape* colShape = new btCapsuleShape( btScalar(fRadius), btScalar(fHeight) );
		

	}

	
}

plPhysicalControllerCore* plPhysicalControllerCore::Create(plKey ownerSO, float height, float width, bool human)
{

	// Yada Yada create bullet instance?
	float radius = width / 2.0f;
	float realHeight = height - width;
	return new plBTPhysicalControllerCore(ownerSO, realHeight, radius, human);

}

void plBTPhysicalControllerCore::Enable(bool enable)
{

}

void plBTPhysicalControllerCore::SetSubworld(plKey world) 
{

}

void plBTPhysicalControllerCore::GetState(hsPoint3& pos, float& zRot)
{

}

void plBTPhysicalControllerCore::SetState(const hsPoint3& pos, float zRot)
{


}

void plBTPhysicalControllerCore::SetMovementStrategy(plMovementStrategy* strategy)
{
	if (fKinematicCCT != strategy->IsKinematic())
	{
		// recreate the controller as Kinematic

	}

	// for now just pass what we are given to the object
	fMovementStrategy = strategy;
}

void plBTPhysicalControllerCore::SetGlobalLoc(const hsMatrix44& l2w)
{

}

void plBTPhysicalControllerCore::GetPositionSim(hsPoint3& pos)
{

}

void plBTPhysicalControllerCore::Move(hsVector3 displacement, unsigned int collideWidth, unsigned int &collisionResults)
{

}

void plBTPhysicalControllerCore::SetLinearVelocitySim(const hsVector3& linearVel)
{

}

int plBTPhysicalControllerCore::SweepControllerPath(const hsPoint3& startPos, const hsPoint3& endPos, bool vsDynamics,
	bool vsStatics, uint32_t& vsSimGroups, std::vector<plControllerSweepRecord>& hits) 
{
	// this should possibly return something more intelligent at some point
	return 0;
}

void plBTPhysicalControllerCore::LeaveAge() 
{

}

void plBTPhysicalControllerCore::IHandleEnableChanged() 
{

}

plDrawableSpans* plBTPhysicalControllerCore::CreateProxy(hsGMaterial* mat, hsTArray<uint32_t>& idx, plDrawableSpans* addTo) {

	//Just return what we are given
	return addTo;
}