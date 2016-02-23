#include "plBTPhysical.h"
#include "plPhysical/plSimDefs.h"

#include "hsResMgr.h"

#include "plSimulationMgr.h"
#include "pnMessage/plNodeRefMsg.h"

#include "plPhysical/plPhysicalSndGroup.h"

#include "BulletCollision/CollisionShapes/btCompoundShape.h"
#include "BulletCollision/CollisionShapes/btSphereShape.h"
#include "BulletCollision/CollisionShapes/btConvexShape.h"
//#include "BulletCollision/CollisionShapes/btTriangleMeshShape.h"
#include "BulletCollision/CollisionShapes/btBoxShape.h"

#include "hsStream.h"
#include "hsBitVector.h"
#include "hsMatrix44.h"
#include "hsQuat.h"

class PhysRecipe
{
public:
	PhysRecipe();

	float mass;
	float friction;
	float restitution;
	plSimDefs::Bounds bounds;
	plSimDefs::Group group;
	uint32_t reportsOn;
	plKey objectKey;
	plKey sceneNode;
	plKey worldKey;

	// The local to subworld matrix (or local to world if worldKey is nil)
	hsMatrix44 l2s;

	// these are the only PhsyX elements we need to convert?
	//NxConvexMesh* convexMesh;
	//NxTriangleMesh* triMesh;

	// For spheres only
	float radius;
	hsPoint3 offset;

	// For Boxes
	hsPoint3 bDimensions;
	hsPoint3 bOffset;

	// For export time only.  The original data used to create the mesh
	// hsVectorStream* meshStream;
	// Does this mean that we have the original mesh somewhere that we can then use in bullet?
};

plBTPhysical::plBTPhysical()
	:fWorldKey(nil)
{
	
}

plBTPhysical::~plBTPhysical() {

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

	/*NxActorDesc actorDesc;
	NxSphereShapeDesc sphereDesc;
	NxConvexShapeDesc convexShapeDesc;
	NxTriangleMeshShapeDesc trimeshShapeDesc;
	NxBoxShapeDesc boxDesc;*/

	btCompoundShape* actorDesc; // maybe??
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
			
			// need to make the transform to add to the child group ??
		}
	}

	return true;
}

void plBTPhysical::Write(hsStream* stream, hsResMgr* mgr)
{

}

bool plBTPhysical::MsgReceive(plMessage* msg) 
{
	// Send this down let plPhysical deal with it
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
	plDrawableSpans* myDraw = addTo;
	return myDraw;
}