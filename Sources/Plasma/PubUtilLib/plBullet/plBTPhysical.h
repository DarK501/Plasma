#include "plPhysical.h"
#include "hsMatrix44.h"
#include "hsBitVector.h"

#include "plPhysical/plSimDefs.h"

class plBTPhysical : public plPhysical
{
public:
	friend class plSimulationMgr;

	plBTPhysical();
	virtual ~plBTPhysical();

	CLASSNAME_REGISTER(plBTPhysical);
	GETINTERFACE_ANY(plBTPhysical, plPhysical);

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

	plSimDefs::Group fGroup;
	plPhysicalSndGroup* fSndGroup;

	bool        fWeWereHit;
	hsVector3   fHitForce;
	hsPoint3    fHitPos;

	uint32_t fReportsOn;          // bit vector for groups we report interactions with
	uint16_t fLOSDBs;             // Which LOS databases we get put into
	hsBitVector fProps;         // plSimulationInterface::plSimulationProperties kept here
	float   fMass;
	
	plKey fWorldKey;    // either a subworld or nil
	plKey fObjectKey;           // the key to our scene object

};