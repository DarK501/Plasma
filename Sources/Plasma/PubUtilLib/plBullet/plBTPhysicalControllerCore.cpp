
#include "plBTPhysicalControllerCore.h"
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


}