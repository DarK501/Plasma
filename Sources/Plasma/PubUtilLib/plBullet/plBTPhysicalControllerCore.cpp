
#include "plBTPhysical.h"
#include "plBTPhysicalControllerCore.h"
#include "plSimulationMgr.h"

#include "pnSceneObject/plSceneObject.h"

// bullet includes
#include "btBulletDynamicsCommon.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#include "BulletDynamics/Character/btKinematicCharacterController.h"

#include "plDrawable/plDrawableGenerator.h"
#include "plSurface/hsGMaterial.h"      // For our proxy
#include "plSurface/plLayerInterface.h" // For our proxy

#define kCCTSkinWidth 0.1f
#define kCCTStepOffset 0.7f
#define kCCTZOffset ((fRadius + (fHeight / 2)) + kCCTSkinWidth)
#define kPhysHeightCorrection 0.8f
#define kPhysZOffset ((kCCTZOffset + (kPhysHeightCorrection / 2)) - 0.05f)
#define kAvatarMass 200.0f

static std::vector<plBTPhysicalControllerCore*> gControllers;
int plBTPhysicalControllerCore::fBTControllersMax = 0;

plBTPhysicalControllerCore::plBTPhysicalControllerCore(plKey ownerSO, float height, float radius, bool human)
	:plPhysicalControllerCore(ownerSO, height, radius)
	, fController(nil)
	, fActor(nil)
	, fProxyGen(nil)
	, fKinematicCCT(true)
	, fHuman(human)
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
		
		btTransform trans;
		trans.setIdentity();
		trans.setOrigin(btVector3(pos.fX, pos.fY, pos.fZ + kCCTZOffset));
		
		btConvexShape* shape = new btCapsuleShape(fRadius, fHeight);

		btPairCachingGhostObject* ghostObj = new btPairCachingGhostObject();
		ghostObj->setWorldTransform(trans);
		// broadphase needs exposing
		ghostObj->setCollisionShape(shape);
		ghostObj->setCollisionFlags(btCollisionObject::CF_KINEMATIC_OBJECT);

		btScalar stepHeight(kCCTStepOffset);

		fController = new btKinematicCharacterController(ghostObj, shape, stepHeight, 2);
		
		fController->setMaxSlope(kSlopeLimit);
		fActor = fController->getGhostObject();

		scene->addCollisionObject(ghostObj);
		scene->addAction(fController);
	}
	else
	{

		btTransform trans;
		trans.setIdentity();
		trans.setOrigin(btVector3(pos.fX, pos.fY, pos.fZ + kCCTZOffset));

		btCollisionShape* shape = new btCapsuleShape(btScalar(fRadius + kCCTSkinWidth), btScalar(fHeight + kPhysHeightCorrection));

		btScalar mass(kAvatarMass);
		btVector3 localIntertia(0, 0, 0);
		
		shape->calculateLocalInertia(mass, localIntertia);
		btDefaultMotionState* shapeMotionShape = new btDefaultMotionState(trans);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, shapeMotionShape, shape, localIntertia);
		btRigidBody* body = new btRigidBody(rbInfo);

		scene->addRigidBody(body);
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
	if (fEnabled != enable) 
	{
		fEnabled = enable;
		if (fEnabled)
		{
			fEnableChanged = true;
		}
		else
		{
			if (!fKinematicCCT)
			{
				// Set body as Kinematic
			}
		}
	}
}

void plBTPhysicalControllerCore::SetSubworld(plKey world) 
{

}

void plBTPhysicalControllerCore::GetState(hsPoint3& pos, float& zRot)
{
	fLocalRotation.NormalizeIfNeeded();
	fLocalRotation.GetAngleAxis(&zRot, (hsVector3*)&pos);

	if (pos.fZ < 0)
		zRot = (2 * float(M_PI)) - zRot; // reverse Axis

	pos = fLocalPosition;
}

void plBTPhysicalControllerCore::SetState(const hsPoint3& pos, float zRot)
{
	plSceneObject* so = plSceneObject::ConvertNoRef(fOwner->ObjectIsLoaded());

}

void plBTPhysicalControllerCore::SetMovementStrategy(plMovementStrategy* strategy)
{
	if (fKinematicCCT != strategy->IsKinematic())
	{
		// recreate the controller as Kinematic
		IDeleteController();
		fKinematicCCT = !fKinematicCCT;
		ICreateController(fLocalPosition);

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

void plBTPhysicalControllerCore::Apply(float delSecs) {

}

void plBTPhysicalControllerCore::IHandleEnableChanged() 
{

}

plDrawableSpans* plBTPhysicalControllerCore::CreateProxy(hsGMaterial* mat, hsTArray<uint32_t>& idx, plDrawableSpans* addTo) 
{

	plDrawableSpans* draw = addTo;
	
	bool blended = ((mat->GetLayer(0)->GetBlendFlags() & hsGMatState::kBlendMask));
	float radius = fRadius;

	draw = plDrawableGenerator::GenerateSphericalDrawable(fLocalPosition, radius, mat, fLastGlobalLoc, blended, nil, &idx, draw);
	
	//Just return what we are given
	return draw;
}

void plBTPhysicalControllerCore::IDeleteController()
{
	if (fKinematicCCT)
	{
		// Release the controller?

	} 
	else
	{
		btDiscreteDynamicsWorld* scene = plSimulationMgr::GetInstance()->GetScene(fWorldKey);

	}
	
	fActor = nil;
	//plSimulationMgr::GetInstance()->ReleaseScene(fWorldKey);
}