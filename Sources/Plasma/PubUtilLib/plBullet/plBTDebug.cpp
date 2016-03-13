#include "plBTDebug.h"

// use plasma to draw Bullet debug
#include "plDrawable/plDrawableGenerator.h"
#include "plSurface/hsGMaterial.h"      // For our proxy
#include "plSurface/plLayerInterface.h" // For our proxy

void plBTDebug::drawLine(const btVector3& from, const btVector3& to, const btVector3& color) 
{

	//This is mainly to see if we can get this called

	/*plDrawableGenerator::GenerateDrawable(nil, // Vert Count (int)
		nil, // Position (hsPoint3)
		nil, // normals (hsPoint3)
		nil, // uvws
		0,   // uvws per vertex
		nil, // colors - def to white
		true, // quick fade
		nil, // optional color modulations
		0, // num of indices
		0, // indices array
		mat, // materials
		l2w, // local to world matrix
		blended, ///
		&idx,
		addTo
		);
	*/
}

void plBTDebug::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color)
{

}

void plBTDebug::reportErrorWarning(const char* warningString)
{

}

void plBTDebug::draw3dText(const btVector3& location, const char* textString)
{

}

void plBTDebug::setDebugMode(int debugMode)
{
	//m_debugMode = debugMode;
}

int plBTDebug::getDebugMode() const
{
	return DBG_DrawWireframe;
}