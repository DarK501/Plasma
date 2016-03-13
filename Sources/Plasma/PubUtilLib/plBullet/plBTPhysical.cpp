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

//#include "plBTPhysicalControllerCore.h"

// From plPhysicalControllerCore - maybe add these to hsMatrix44?
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
	
	// Maybe add the Clear function to hsMatrix44?
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
		// Cooked meshes - not sure what we want to do with them at the moment, read to ensure that we have the object at least
		stream->ReadLE32(); //?
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
			
			btTransform trans;
			trans.setIdentity();

			hsPoint3 plVec = recipe.l2s.GetTranslate();

			trans.setOrigin(btVector3(plVec.fX, plVec.fY, plVec.fZ));
			
			hsMatrix44 boxL2W;
			boxL2W.Reset();
			boxL2W.SetTranslate(&recipe.bOffset);
			//trans.setRotation(boxL2W);

 		    // Set Box Dimensions - these might already be in halfs??
			btBoxShape* boxDesc = new btBoxShape(btVector3((recipe.bDimensions.fX) / 2.f, (recipe.bDimensions.fY) / 2.f, (recipe.bDimensions.fZ) / 2.f));
			actorDesc->addChildShape(trans, boxDesc);
		}
	}	

	btDiscreteDynamicsWorld* scene = plSimulationMgr::GetInstance()->GetScene(fWorldKey);

	// grab the current scene so that we can do things with it
	
	try {

		btScalar mass(0.f);
		btVector3 localInertia(0, 0, 0);

		// creating the actor
		btDefaultMotionState* motionState = new btDefaultMotionState();
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, actorDesc, localInertia);
		btRigidBody* rigidBody = new btRigidBody(rbInfo);

		rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
		rigidBody->setActivationState(DISABLE_DEACTIVATION);

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

	// Message engine to say we have a new physical
	plNodeRefMsg* refMsg = new plNodeRefMsg(fSceneNode, plRefMsg::kOnCreate, -1, plNodeRefMsg::kPhysical);
	hsgResMgr::ResMgr()->AddViaNotify(GetKey(), refMsg, plRefFlags::kActiveRef);

	if (fWorldKey)
	{
		plGenRefMsg* ref = new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kPhysRefWorld);
		hsgResMgr::ResMgr()->AddViaNotify(fWorldKey, ref, plRefFlags::kActiveRef);
	}

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
	
	// set l2w - magic I've been missing for proxies?
    btTransform bodyLoc = fActor->getWorldTransform();
	btVector3 bodyPos = bodyLoc.getOrigin();

	hsPoint3* transVec = new hsPoint3(bodyPos.getX(), bodyPos.getY(), bodyPos.getZ() );
	l2w.Reset(); // this resets the point to 0
	l2w.SetTranslate(transVec); // we then pass a the vector in as a translation 

	// Currently have objects at 0,0,0! This needs more work to have them placed correctly in the world
		
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

void plBTPhysical::ISetTransformGlobal(const hsMatrix44& l2w) 
{
	
	hsAssert(fActor->isStaticObject(), "Should not move a static Actor");

	if (fActor->isKinematicObject())
		fActor->activate(); // start the Kinematics

	if (fWorldKey)
	{
		plSceneObject* so = plSceneObject::ConvertNoRef(fWorldKey->ObjectIsLoaded());
		hsAssert(so, "Scene object not loaded while accessing subworld");
		hsMatrix44 p2s = so->GetCoordinateInterface()->GetWorldToLocal() * l2w; //Translation?!
		
		// transform the proxy
		if (fProxyGen)
		{
			hsMatrix44 w2l;
			p2s.GetInverse(&w2l);
			fProxyGen->SetTransform(p2s, w2l);
		}

	}
	else
	{
		if (fProxyGen)
		{
			hsMatrix44 w2l;
			l2w.GetInverse(&w2l);
			fProxyGen->SetTransform(l2w, w2l);
		}
	}

	// there was a physX hack here for breaking Kinematics

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
	plKey oldNode = GetSceneNode();
	if (oldNode == newNode)
		return;

	if (newNode)
	{
		plNodeRefMsg* refMsg = new plNodeRefMsg(newNode, plNodeRefMsg::kOnRequest, -1, plNodeRefMsg::kPhysical);
		hsgResMgr::ResMgr()->SendRef(GetKey(), refMsg, plRefFlags::kActiveRef);
	}

	if (oldNode)
		oldNode->Release(GetKey());
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
	
	if (force || (fActor->getCollisionFlags()|btCollisionObject::CF_KINEMATIC_OBJECT)
		&& (fWorldKey || !CompareMatrices(l2w, fCachedLocal2World, .0001f)))
	{
		ISetTransformGlobal(l2w);
		//plProfile_Inc(SetTransforms); - increase profiler stats
	}
	else
	{
		if (!(fActor->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT) && plSimulationMgr::fExtraProfile)
			hsAssert("Setting transform on non-dynamic: %s.", GetKeyName().c_str());
	}
}

void plBTPhysical::GetTransform(hsMatrix44& l2w, hsMatrix44& w2l)
{
	IGetTransformGlobal(l2w);
	//l2w.GetInverse(&w2l);
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
	GetTransform(l2w, unused); // This is mangling some how?

	bool blended = ((mat->GetLayer(0)->GetBlendFlags() & hsGMatState::kBlendMask));

	btCollisionShape* shape = fActor->getCollisionShape();
	btCompoundShape* compDesc = static_cast<btCompoundShape *>(shape);

	for (int i = 0 ; i < compDesc->getNumChildShapes(); i++)
	{

		btBoxShape* boxDesc = static_cast<btBoxShape *>(compDesc->getChildShape(i));
		btVector3 boxMatrix = boxDesc->getHalfExtentsWithoutMargin();

		draw = plDrawableGenerator::GenerateBoxDrawable((boxMatrix.getX() * 2.f), (boxMatrix.getZ() * 2.f), (boxMatrix.getY() * 2.f), mat, l2w, blended, nil, &idx, draw);

	}
	return draw;
}