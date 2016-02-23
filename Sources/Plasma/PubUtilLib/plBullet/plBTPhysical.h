#include "plPhysical.h"
#include "hsMatrix44.h"
#include "hsBitVector.h"

#include "plPhysical/plSimDefs.h"

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

static void ClearMatrix(hsMatrix44 &m);

class plBTPhysical : public plPhysical
{
public:
	friend class plSimulationMgr;

	enum PhysRefType
	{
		kPhysRefWorld,
		kPhysRefSndGroup

	};

	plBTPhysical();
	virtual ~plBTPhysical();

	CLASSNAME_REGISTER(plBTPhysical);
	GETINTERFACE_ANY(plBTPhysical, plPhysical);

	// Export and Internal use only - maybe protect?
	bool Init(PhysRecipe& recipe);

	virtual void Read(hsStream* s, hsResMgr* mgr);
	virtual void Write(hsStream* s, hsResMgr* mgr);

	virtual bool MsgReceive(plMessage* msg);

	//
	// From plPhysical
	//
	virtual plPhysical& SetProperty(int prop, bool b);
	virtual bool GetProperty(int prop) const { return fProps.IsBitSet(prop) != 0; }

	virtual void SetObjectKey(plKey key) { fObjectKey = key; }
	virtual plKey GetObjectKey() const { return fObjectKey; }

	virtual void SetSceneNode(plKey node);
	virtual plKey GetSceneNode() const;

	virtual bool GetLinearVelocitySim(hsVector3& vel) const;
	virtual void SetLinearVelocitySim(const hsVector3& vel);
	virtual void ClearLinearVelocity();

	virtual bool GetAngularVelocitySim(hsVector3& vel) const;
	virtual void SetAngularVelocitySim(const hsVector3& vel);

	virtual void SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l, bool force = false);
	virtual void GetTransform(hsMatrix44& l2w, hsMatrix44& w2l);

	virtual int GetGroup() const { return fGroup; }

	virtual void    AddLOSDB(uint16_t flag) { hsSetBits(fLOSDBs, flag); }
	virtual void    RemoveLOSDB(uint16_t flag) { hsClearBits(fLOSDBs, flag); }
	virtual uint16_t  GetAllLOSDBs() { return fLOSDBs; }
	virtual bool    IsInLOSDB(uint16_t flag) { return hsCheckBits(fLOSDBs, flag); }

	virtual plKey GetWorldKey() const { return fWorldKey; }

	virtual plPhysicalSndGroup* GetSoundGroup() const { return fSndGroup; }

	virtual void GetPositionSim(hsPoint3& pos) const { IGetPositionSim(pos); }

	virtual void SendNewLocation(bool synchTransform = false, bool isSynchUpdate = false);

	virtual void SetHitForce(const hsVector3& force, const hsPoint3& pos) { fWeWereHit = true; fHitForce = force; fHitPos = pos; }
	virtual void ApplyHitForce();
	virtual void ResetHitForce() { fWeWereHit = false; fHitForce.Set(0, 0, 0); fHitPos.Set(0, 0, 0); }

	virtual void GetSyncState(hsPoint3& pos, hsQuat& rot, hsVector3& linV, hsVector3& angV);
	virtual void SetSyncState(hsPoint3* pos, hsQuat* rot, hsVector3* linV, hsVector3* angV);

	virtual plDrawableSpans* CreateProxy(hsGMaterial* mat, hsTArray<uint32_t>& idx, plDrawableSpans* addTo);

	virtual void ExcludeRegionHack(bool cleared);

	virtual float GetMass() { return fMass; }

protected:

	void IGetPositionSim(hsPoint3& pos) const;

	plSimDefs::Bounds fBoundsType;
	plSimDefs::Group fGroup;
	plPhysicalSndGroup* fSndGroup;
	
	uint32_t fReportsOn;           // bit vector for groups we report interactions with
	uint16_t fLOSDBs;             // Which LOS databases we get put into
	hsBitVector fProps;          // plSimulationInterface::plSimulationProperties kept here
	float   fMass;

	plKey fSceneNode;

	// we need to remember the last matrices we sent to the coordinate interface
	// so that we can recognize them when we send them back and not reapply them,
	// which would reactivate our body. inelegant but effective
	hsMatrix44 fCachedLocal2World;

	bool        fWeWereHit;
	hsVector3   fHitForce;
	hsPoint3    fHitPos;

	plKey fWorldKey; // either a subworld or nil
	plKey fObjectKey;           // the key to our scene object

};