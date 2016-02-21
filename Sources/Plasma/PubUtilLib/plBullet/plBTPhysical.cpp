#include "plBTPhysical.h"

plBTPhysical::plBTPhysical() {
	
}

plBTPhysical::~plBTPhysical() {

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

// From plPhysicalControllerCore
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