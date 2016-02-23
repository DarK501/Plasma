
#include "plBTPhysical.h"
#include "plBTPhysicalControllerCore.h"
#include "plSimulationMgr.h"

#include "pnSceneObject/plSceneObject.h"

// bullet includes
#include "btBulletDynamicsCommon.h"
#include "BulletDynamics/Character/btKinematicCharacterController.h"

#define kCCTSkinWidth 0.1f
#define kPhysHeightCorrection 0.8f
#define kAvatarMass 200.0f

static std::vector<plBTPhysicalControllerCore*> gControllers;
int plBTPhysicalControllerCore::fBTControllersMax = 0;

plBTPhysicalControllerCore::plBTPhysicalControllerCore(plKey ownerSO, float height, float radius, bool human)
:plPhysicalControllerCore(ownerSO, height, radius) 
{

	//create controller
	btDefaultCollisionConfiguration* fCollisionConfig = new btDefaultCollisionConfiguration();
	btCollisionDispatcher* fCollisionDispatch = new btCollisionDispatcher(fCollisionConfig);
	btBroadphaseInterface* fBoardphaseInt = new btDbvtBroadphase();
	btSequentialImpulseConstraintSolver* fConstraintSolver = new btSequentialImpulseConstraintSolver();
	
 	ICreateController(fLocalPosition);

	gControllers.push_back(this);
}

plBTPhysicalControllerCore::~plBTPhysicalControllerCore()
{
	// destroy controllers - bullet advises reverse destruction!
	int numControllers = gControllers.size();

	for (int i = numControllers - 1; i > 0; --i)
	{
		if (gControllers[i] == this)
		{
			gControllers.erase(gControllers.begin() + i);
			break;
		}	
	}
	//IDeleteController() - for when we have a scene to release
}

void plBTPhysicalControllerCore::ICreateController(const hsPoint3& pos) 
{
	
	btDiscreteDynamicsWorld* scene = plSimulationMgr::GetInstance()->GetScene(fWorldKey);

	if (fKinematicCCT) {
		// Add a shape to the exisiting actor?
		btCollisionShape* desc = new btCapsuleShape( btScalar(fRadius), btScalar(fHeight) );
		
		btTransform descTransform;
		descTransform.setIdentity();
		descTransform.setOrigin(btVector3(pos.fX, pos.fY, pos.fZ));
		
		btScalar mass(0.);
		btVector3 localInertia(0, 0, 0);

		btDefaultMotionState * myMotionState = new btDefaultMotionState(descTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, desc, localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);

		scene->addRigidBody(body);

	}
	else
	{

		// PhysX previously described the avatar as a Kinematic Controller - this is just a shape for now
		btCollisionShape* capDesc = new btCapsuleShape(btScalar(fRadius + kCCTSkinWidth), btScalar(fHeight + kPhysHeightCorrection));

		btTransform descTransform;
		descTransform.setIdentity();

		// figure out if this is how we deal with freezing the object (Gravity and Rotation)?
		btScalar mass(kAvatarMass);
		btVector3 localIntertia(0, 0, 0);
		
		if (fEnabled) 
		{
			capDesc->setUserIndex = plSimDefs::kGroupAvatar;
		}
		else
		{
			// switch to be Kinematic?
			capDesc->setUserIndex = plSimDefs::kGroupAvatarKinematic;
		}

	}

	
}

plPhysicalControllerCore* plPhysicalControllerCore::Create(plKey ownerSO, float height, float width, bool human)
{

	if (!plBTPhysicalControllerCore::fBTControllersMax || gControllers.size() < plBTPhysicalControllerCore::fBTControllersMax)
	{
		float radius = width / 2.0f;
		float realHeight = height - width;
		return new plBTPhysicalControllerCore(ownerSO, realHeight, radius, human);
	}
	return nil;
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