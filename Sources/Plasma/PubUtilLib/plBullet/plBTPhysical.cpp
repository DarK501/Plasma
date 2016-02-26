#include "plBTPhysical.h"
#include "plPhysical/plSimDefs.h"

#include "hsResMgr.h"

#include "plSimulationMgr.h"
#include "pnMessage/plNodeRefMsg.h"
#include "plPhysical/plPhysicalSndGroup.h"

#include "plMessage/plSimStateMsg.h"
#include "plMessage/plLinearVelocityMsg.h"
#include "plMessage/plAngularVelocityMsg.h"

#include "btBulletDynamicsCommon.h"

#include "hsStream.h"
#include "hsBitVector.h"
#include "hsMatrix44.h"
#include "hsQuat.h"

#include "pnSceneObject/plSimulationInterface.h"
#include "pnSceneObject/plCoordinateInterface.h"

#include "plPhysical/plPhysicalProxy.h"
#include "plDrawable/plDrawableGenerator.h"
#include "plSurface/hsGMaterial.h"      // For our proxy
#include "plSurface/plLayerInterface.h" // For our proxy

PhysRecipe::PhysRecipe()
	: mass(0.f)
	, friction(0.f)
	, restitution(0.f)
	, bounds(plSimDefs::kBoundsMax)
	, group(plSimDefs::kGroupMax)
	, reportsOn(0)
	, objectKey(nil)
	, sceneNode(nil)
	, worldKey(nil)
	//, convexMesh(nil)
	//, triMesh(nil)
	, radius(0.f)
	, offset(0.f, 0.f, 0.f)
	//, meshStream(nil)
{
	l2s.Reset();
}

plBTPhysical::plBTPhysical()
	: //fSDLMod(nil)
	fActor(nil)
	, fBoundsType(plSimDefs::kBoundsMax)
	, fLOSDBs(plSimDefs::kLOSDBNone)
	, fGroup(plSimDefs::kGroupMax)
	, fReportsOn(0)
	//, fLastSyncTime(0.0f)
	, fProxyGen(nil)
	, fSceneNode(nil)
	, fWorldKey(nil)
	, fSndGroup(nil)
	, fMass(0.f)
	, fWeWereHit(false)
	, fHitForce(0, 0, 0)
	, fHitPos(0, 0, 0)
{
	
}

plBTPhysical::~plBTPhysical() {

	//plSimulationMgr::Log("Destroying physcial %s", GetKeyName().c_str());

	delete fProxyGen;
}

void plBTPhysical::Read(hsStream* stream, hsResMgr* mgr) 
{
	// so some poor sod is eventually going to have to convert all the ages to a bullet format
	// I'm not that person, so for now we are going to read in what data we have and make do.
	plPhysical::Read(stream, mgr);
	ClearMatrix(fCachedLocal2World);

	PhysRecipe recipe;
	recipe.mass = stream->ReadLEScalar();
	recipe.friction = stream->ReadLEScalar();
	recipe.restitution = stream->ReadLEScalar();
	recipe.bounds = (plSimDefs::Bounds)stream->ReadByte();
	recipe.group = (plSimDefs::Group)stream->ReadByte();
	recipe.reportsOn = stream->ReadLE32();
	fLOSDBs = stream->ReadLE16();

	if (fLOSDBs == plSimDefs::kLOSDBSwimRegion)
	{
		recipe.group = plSimDefs::kGroupMax;
	}

	recipe.objectKey = mgr->ReadKey(stream);
	recipe.sceneNode = mgr->ReadKey(stream);
	recipe.worldKey = mgr->ReadKey(stream);

	mgr->ReadKeyNotifyMe(stream, new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kPhysRefSndGroup), plRefFlags::kActiveRef);

	hsPoint3 pos;
	hsQuat rot;
	
	pos.Read(stream);
	rot.Read(stream);
	
	rot.MakeMatrix(&recipe.l2s);

	recipe.l2s.SetTranslate(&pos);

	fProps.Read(stream);

	if (recipe.bounds == plSimDefs::kSphereBounds)
	{
		recipe.radius = stream->ReadLEScalar();
	}
	else if (recipe.bounds == plSimDefs::kBoxBounds)
	{
		recipe.bDimensions.Read(stream);
		recipe.bOffset.Read(stream);

	}
	else 
	{
		// Cooked meshes - not sure what we want to do with them at the moment!

	}

	Init(recipe);

	hsColorRGBA physColor;
	float opac = 1.0f;

	physColor.Set(1.f, 0.f, 0.f, 1.f);
	opac = 1.0f;


	fProxyGen = new plPhysicalProxy(hsColorRGBA().Set(0, 0, 0, 1.f), physColor, opac);
	fProxyGen->Init(this);
		
}

bool plBTPhysical::Init(PhysRecipe& recipe)
{
	bool startAsleep = false;

	// Maybe move some of this into read?
	fBoundsType = recipe.bounds;
	fGroup = recipe.group;
	fReportsOn = recipe.reportsOn;
	fObjectKey = recipe.objectKey;
	fSceneNode = recipe.sceneNode;
	fWorldKey = recipe.worldKey;

	btCompoundShape* actorDesc = new btCompoundShape();

	btSphereShape* sphereDesc;
	btConvexShape* convexShapeDesc;
	//btTriangleMeshShape* trimeshShapeDesc;
	btBoxShape* boxDesc;

	// previously we converted to a PhysX matrix here - but for now I'm assuming bullet will be able to deal with the data comming in

	switch (fBoundsType)
	{
		case plSimDefs::kSphereBounds:
		{
			hsMatrix44 sphereL2W;
			sphereL2W.Reset();
			sphereL2W.SetTranslate(&recipe.offset);

			sphereDesc->setUnscaledRadius(btScalar(recipe.radius));
			sphereDesc->setUserIndex(fGroup);
			boxDesc->setUserPointer(boxDesc);

			// need to make the transform to add to the child group ??
		}
		break;

		case plSimDefs::kBoxBounds:
		{
			// Set Box Dimensions - these might already be in halfs??
			btBoxShape* boxDesc = new btBoxShape(btVector3((recipe.bDimensions.fX / 2.f), (recipe.bDimensions.fY / 2.f) , (recipe.bDimensions.fZ / 2.f)));

			// and pose
			hsMatrix44 boxL2W;
			boxL2W.Reset();
			boxL2W.SetTranslate(&recipe.bOffset);

			btTransform descTransform;
			descTransform.setIdentity();
			// this will eventually set the pose needed
			//descTransform.setBasis() 

			boxDesc->setUserIndex(fGroup);
			//boxDesc->setUserPointer(boxDesc); // set a reference we can use later

			actorDesc->addChildShape(descTransform, boxDesc);
		}
	}

	//build the scenes default state
	btTransform sceneTransform;
	sceneTransform.setIdentity();
	sceneTransform.setOrigin(btVector3(0, 0, 0));

	btScalar mass(0.);
	btVector3 localInertia(0, 0, 0);

	// Apply it to the actor
	actorDesc->calculateLocalInertia(mass, localInertia);

	// grab the current scene so that we can do things with it
	btDiscreteDynamicsWorld* scene = plSimulationMgr::GetInstance()->GetScene(fWorldKey);

	try {
		// creating the actor
		btDefaultMotionState* motionState = new btDefaultMotionState(sceneTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, actorDesc, localInertia);
		btRigidBody* rigidBody = new btRigidBody(rbInfo);

		// bullet cleans up after you add this to the scene, so copy the object to keep hold of it
		fActor = rigidBody;

		scene->addRigidBody(fActor);
	}
	catch (...)
	{
		hsAssert(false, "Actor Creation crashed");
	}

	hsAssert(fActor, "Actor creation failed");
	if (!fActor)
		return false;


	return true;
}

void plBTPhysical::Write(hsStream* stream, hsResMgr* mgr)
{

}

bool plBTPhysical::HandleRefMsg(plGenRefMsg* refMsg)
{
	uint8_t refCtxt = refMsg->GetContext();
	plKey refKey = refMsg->GetRef()->GetKey();
	plKey ourKey = GetKey();
	PhysRefType refType = PhysRefType(refMsg->fType);

	plString refKeyName = refKey ? refKey->GetName() : "MISSING";

	if (refType == kPhysRefWorld)
	{
		if (refCtxt == plRefMsg::kOnCreate || refCtxt == plRefMsg::kOnRequest)
		{
			IGetTransformGlobal(fCachedLocal2World);
		}
		if (refCtxt == plRefMsg::kOnDestroy)
		{
			// our world was deleted from under us: move to main world
			hsAssert(0, "Lost World");
		}
	}
	else if (refType == kPhysRefSndGroup)
	{
		switch(refCtxt)
		{
		case plRefMsg::kOnCreate:
		case plRefMsg::kOnRequest:
			fSndGroup = plPhysicalSndGroup::ConvertNoRef(refMsg->GetRef());
			break;
		case plRefMsg::kOnDestroy:
			fSndGroup = nil;
			break;
		}
	}
	else 
	{
		hsAssert(0, "Unknown ref type, who sent this?");
	}

	return true;

}

void plBTPhysical::IGetTransformGlobal(hsMatrix44& l2w) const
{
	if (fWorldKey)
	{
		plSceneObject* so = plSceneObject::ConvertNoRef(fWorldKey->ObjectIsLoaded());
		hsAssert(so, "SceneObject not loaded while accessing subworld.");
		if (so->GetCoordinateInterface())
		{
			const hsMatrix44& s2w = so->GetCoordinateInterface()->GetLocalToWorld();
			l2w = s2w * l2w;
				
		}

	}
}

bool plBTPhysical::MsgReceive(plMessage* msg) 
{
	if (plGenRefMsg *refM = plGenRefMsg::ConvertNoRef(msg))
	{
		return HandleRefMsg(refM);
	}
	else if (plSimulationMsg *simM = plSimulationMsg::ConvertNoRef(msg))
	{
		plLinearVelocityMsg* velMsg = plLinearVelocityMsg::ConvertNoRef(msg);
		if (velMsg)
		{
			SetLinearVelocitySim(velMsg->Velocity());
			return true;
		}
		plAngularVelocityMsg* angvelMsg = plAngularVelocityMsg::ConvertNoRef(msg);
		if (angvelMsg)
		{
			SetAngularVelocitySim(angvelMsg->AngularVelocity());
			return true;
		}


		return false;
	}
	// couldn't find a local handler: pass to the base
	else
		return plPhysical::MsgReceive(msg);
}

plPhysical& plBTPhysical::SetProperty(int prop, bool b) 
{
	return *this;
}

void plBTPhysical::SetSceneNode(plKey newNode)
{
	
}

plKey plBTPhysical::GetSceneNode() const
{
	return fSceneNode;
}

bool plBTPhysical::GetLinearVelocitySim(hsVector3& vel) const
{
	return false;
}

void plBTPhysical::SetLinearVelocitySim(const hsVector3& vel)
{

}

void plBTPhysical::ClearLinearVelocity()
{

}

bool plBTPhysical::GetAngularVelocitySim(hsVector3& vel) const
{
	return false;
}

void plBTPhysical::SetAngularVelocitySim(const hsVector3& vel)
{

}

void plBTPhysical::SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l, bool force)
{

}

void plBTPhysical::GetTransform(hsMatrix44& l2w, hsMatrix44& w2l)
{

}

void plBTPhysical::ApplyHitForce()
{

}

void plBTPhysical::GetSyncState(hsPoint3& pos, hsQuat& rot, hsVector3& linV, hsVector3& angV)
{

}

void plBTPhysical::SetSyncState(hsPoint3* pos, hsQuat* rot, hsVector3* linV, hsVector3* angV)
{

}

void plBTPhysical::ExcludeRegionHack(bool cleared)
{


}

// From plPhysicalControllerCore - maybe clear these up to their own file at some point
bool CompareMatrices(const hsMatrix44 &matA, const hsMatrix44 &matB, float tolerance)
{
	return
		(fabs(matA.fMap[0][0] - matB.fMap[0][0]) < tolerance) &&
		(fabs(matA.fMap[0][1] - matB.fMap[0][1]) < tolerance) &&
		(fabs(matA.fMap[0][2] - matB.fMap[0][2]) < tolerance) &&
		(fabs(matA.fMap[0][3] - matB.fMap[0][3]) < tolerance) &&

		(fabs(matA.fMap[1][0] - matB.fMap[1][0]) < tolerance) &&
		(fabs(matA.fMap[1][1] - matB.fMap[1][1]) < tolerance) &&
		(fabs(matA.fMap[1][2] - matB.fMap[1][2]) < tolerance) &&
		(fabs(matA.fMap[1][3] - matB.fMap[1][3]) < tolerance) &&

		(fabs(matA.fMap[2][0] - matB.fMap[2][0]) < tolerance) &&
		(fabs(matA.fMap[2][1] - matB.fMap[2][1]) < tolerance) &&
		(fabs(matA.fMap[2][2] - matB.fMap[2][2]) < tolerance) &&
		(fabs(matA.fMap[2][3] - matB.fMap[2][3]) < tolerance) &&

		(fabs(matA.fMap[3][0] - matB.fMap[3][0]) < tolerance) &&
		(fabs(matA.fMap[3][1] - matB.fMap[3][1]) < tolerance) &&
		(fabs(matA.fMap[3][2] - matB.fMap[3][2]) < tolerance) &&
		(fabs(matA.fMap[3][3] - matB.fMap[3][3]) < tolerance);
}

static void ClearMatrix(hsMatrix44 &m)
{
	m.fMap[0][0] = 0.0f; m.fMap[0][1] = 0.0f; m.fMap[0][2] = 0.0f; m.fMap[0][3] = 0.0f;
	m.fMap[1][0] = 0.0f; m.fMap[1][1] = 0.0f; m.fMap[1][2] = 0.0f; m.fMap[1][3] = 0.0f;
	m.fMap[2][0] = 0.0f; m.fMap[2][1] = 0.0f; m.fMap[2][2] = 0.0f; m.fMap[2][3] = 0.0f;
	m.fMap[3][0] = 0.0f; m.fMap[3][1] = 0.0f; m.fMap[3][2] = 0.0f; m.fMap[3][3] = 0.0f;
	m.NotIdentity();
}

void plBTPhysical::SendNewLocation(bool synchTransform, bool isSynchUpdate)
{

}

void plBTPhysical::IGetPositionSim(hsPoint3& pos) const
{

}

plDrawableSpans* plBTPhysical::CreateProxy(hsGMaterial* mat, hsTArray<uint32_t>& idx, plDrawableSpans* addTo)
{
	plDrawableSpans* draw = addTo;
	hsMatrix44 l2w, unused;
	GetTransform(l2w, unused);

	bool blended = ((mat->GetLayer(0)->GetBlendFlags() & hsGMatState::kBlendMask));

	btCollisionShape* shape = fActor->getCollisionShape();

	if (shape->getShapeType())
	{
		btBoxShape* boxDesc = static_cast<btBoxShape *>(shape);
		if (boxDesc)
		{
			// Get Bullet Scale
			btVector3 bDim = boxDesc->getLocalScaling();

			draw = plDrawableGenerator::GenerateBoxDrawable(bDim.x(), bDim.y(), bDim.z(), mat, l2w, blended, nil, &idx, draw);

			// could likely also add the bullet stuff here for the engine debug??

		}
	}

	return draw;
}