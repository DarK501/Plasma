/*
	plPhysicalControllerCore in the Avatar needs to be here as the Avatar has a lot of calls to the Physics engine, At this point I'm still figuring out what :)
*/
#include "plAvatar/plPhysicalControllerCore.h"

// Our engine needs to inhert ControllerCore from the Avatar
class plBTPhysicalControllerCore : public plPhysicalControllerCore
{
public:
	//Match plPhysicalControllerCore Implimentation to be compatable with existing code
	plBTPhysicalControllerCore(plKey ownerSO, float height, float radius, bool human);
	~plBTPhysicalControllerCore();



protected:
	void ICreateController(const hsPoint3& pos);
};