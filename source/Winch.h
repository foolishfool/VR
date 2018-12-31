#pragma once

#include <UnigineEngine.h>
#include <UnigineNode.h>
#include <UnigineEditor.h>
#include <UnigineGame.h>
#include <UniginePrimitives.h>
#include <UnigineObjects.h>
#include <UnigineApp.h>
#include <UnigineSounds.h>
#include <UniginePhysicals.h>
#include "Utils.h"
#include "Rope.h"

using namespace Unigine;
using namespace Math;
class Winch
{
public:
	void init();
	void update();
	void shutdown();
	Vector<Rope*> ropes;

	static Winch* instance;
	static Winch* get();
	void RopeRotate(float rotateSpeed);
	bool isCut = false;
private: 
	


	AmbientSourcePtr winchsound;
	AmbientSourcePtr cutsound;

	//used as the flag of VR control 
	
	bool isVRUp = false;
	bool isVRDown = false;
	bool isVRCutRope = false;
	bool isVRWinchRightRotate = false;
	bool isVRWinchLeftRotate = false;

	NodePtr wincharm;
	//used as the flag of joystick control 
	bool isUp = false;
	bool isDown = false;
	bool isCutRope = false;
	bool isRotateRight = false;
	bool isRotateLeft = true;

	ControlsJoystickPtr   joystick;
	// the number of axis1;
	float val1 = 0.37f;
	//helicopter speed
	float helicopter_speed;
	PhysicalWindPtr wind;
	Vec3 currentposition;
	Vec3 lastposition;
	vec3 wind_direction;
	
	ObjectTextPtr number;
	
	//used as the initial position of upper rope
	
	float currentRotAngle;

	bool rotateRight = true;
	bool rotateLeft = false;

	Vec3 hookInitialPosition;
	
	//BodyRopePtr ropebody1;
	//BodyRopePtr ropebody2;
	//BodyRigidPtr handlebody;
	BodyRigidPtr hookbody;
	//NodePtr Rope_intial_point;



	NodePtr currenthook;
	// the whole rope's length used as the speed of rope
	float ropelength;
	float rope_speed;


	int Rotate(Unigine::NodePtr node, Unigine::Math::quat rotationDirection)
	{
		// getting current node transformation matrix
		Mat4 transform = node->getTransform();

		// calculating delta rotation around an arbitrary axis
		quat delta_rotation = quat(rotationDirection);

		// setting node's scale, rotation and position
		node->setRotation(node->getRotation()*delta_rotation);
		return 1;
	}



	//cut the rope
	void  CutRope()
	{
			cutsound->play();
	//		ropes[ropes.size()-1]->joint1->setBroken(1);
	}



	//old version rope
	ObjectDummyPtr Rope_intial_point_obj;

	ObjectMeshDynamicPtr hook;
	ObjectMeshDynamicPtr handle;
	BodyRigidPtr hookBody;
	BodyRigidPtr handlebody;

	ObjectMeshDynamicPtr upper_winchrope;
	ObjectMeshDynamicPtr lower_winchrope;

	JointParticlesPtr joint1;
	JointParticlesPtr joint2;
	JointParticlesPtr joint3;
	JointParticlesPtr joint4;


	BodyRopePtr ropebody1;
	BodyRopePtr ropebody2;

	BodyRopePtr createropebody(ObjectMeshDynamicPtr OMD) {
		//assigning a rope body to the dynamic mesh object and setting rope parameters
		BodyRopePtr body = BodyRope::create(OMD->getObject());
		body->setMass(100.0f);
		body->setRadius(0.02f);
		body->setLinearDamping(0.0f);
		body->setAngularRestitution(0.0);
		body->setFriction(100.0f);
		body->setRestitution(0.0f);
		///the extent of rope moving 1 biggest 0 none
		body->setRigidity(0.01f);
		body->setLinearStretch(0.0f);
		body->setNumIterations(24);

		return body;
	}

	void CreateWinchRope() {

		ropebody1 = createropebody(upper_winchrope);
		ropebody2 = createropebody(lower_winchrope);

		joint1 = JointParticles::create(Rope_intial_point_obj->getBody(), ropebody1->getBody(), Rope_intial_point_obj->getWorldPosition(), vec3(0.04));
		joint2 = JointParticles::create(handle->getBody(), ropebody1->getBody(), handle->getWorldPosition(), vec3(0.14));
		joint3 = JointParticles::create(handle->getBody(), ropebody2->getBody(), handle->getWorldPosition(), vec3(0.2));
		joint4 = JointParticles::create(hook->getBody(), ropebody2->getBody(), hook->getWorldPosition(), vec3(0.1));

		ropebody1->setLinearStretch(1.9);

	}
};

