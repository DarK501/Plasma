/*
	plPhysicalControllerCore in the Avatar needs to be here as the Avatar has a lot of calls to the Physics engine, At this point I'm still figuring out what :)
*/
#include "plAvatar/plPhysicalControllerCore.h"

class btRigidBody;

class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;

class plBTPhysicalControllerCore : public plPhysicalControllerCore
{
public:
	//Match plPhysicalControllerCore Implimentation to be compatable with existing code
	plBTPhysicalControllerCore(plKey ownerSO, float height, float radius, bool human);
	~plBTPhysicalControllerCore();

	virtual void Enable(bool enable);

	// Subworld
	virtual void SetSubworld(plKey world);

	virtual void GetState(hsPoint3& pos, float& zRot);
	virtual void SetState(const hsPoint3& pos, float zRot);

	// Movement strategy
	virtual void SetMovementStrategy(plMovementStrategy* strategy);

	virtual void SetGlobalLoc(const hsMatrix44& l2w);

	virtual void GetPositionSim(hsPoint3& pos);

	virtual void Move(hsVector3 displacement, unsigned int collideWidth, unsigned int &collisionResults);

	virtual void SetLinearVelocitySim(const hsVector3& linearVel);

	virtual int SweepControllerPath(const hsPoint3& startPos, const hsPoint3& endPos, bool vsDynamics,
		bool vsStatics, uint32_t& vsSimGroups, std::vector<plControllerSweepRecord>& hits);

	plDrawableSpans* CreateProxy(hsGMaterial* mat, hsTArray<uint32_t>& idx, plDrawableSpans* addTo);

	virtual void LeaveAge();

	static int fBTControllersMax;

protected:
	void ICreateController(const hsPoint3& pos);

	virtual void IHandleEnableChanged();

	bool fKinematicCCT;
	bool fHuman;

	btRigidBody* fActor;

	btDefaultCollisionConfiguration* fCollisionConfig;
	btCollisionDispatcher* fCollisionDispatch;
	btBroadphaseInterface* fBroadphaseInt;
	btSequentialImpulseConstraintSolver* fConstraintSolver;
};