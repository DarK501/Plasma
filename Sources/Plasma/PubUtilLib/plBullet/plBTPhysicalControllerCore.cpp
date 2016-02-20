
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