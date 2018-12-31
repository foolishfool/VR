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
#include "Resource.h"
#include "ObjMovable.h"

using namespace Unigine;
using namespace Math;

class Rope
{
public:
	void init();
	void update();
	void shutdown();


	Rope() { init(); };
	virtual ~Rope() {}
	

	ObjectDummyPtr Rope_intial_point;

	int rope_id;
	NodePtr Rope_intial_point_node;
	NodePtr hook_node;
	NodePtr handle_node;
	NodePtr rope_upper ;
	NodePtr rope_lower ;
	

	ObjectMeshDynamicPtr hook;
	ObjectMeshDynamicPtr handle;

	BodyRigidPtr hookBody;
	BodyRigidPtr handlebody;
	BodyRigidPtr basketBody;
	BodyRigidPtr stropBody;
	BodyRigidPtr stretcherBody;

	NodePtr basket_node;
	NodePtr weight_strop_node;
	NodePtr weight_stretcher_node;

	BodyRopePtr ropebody1;
	BodyRopePtr ropebody2;

	JointBallPtr joint0;
	JointParticlesPtr joint1;
	JointParticlesPtr joint2;

	ObjMovable* handle_objmovable;
	ObjMovable* hook_objmovable;

private:
	BodyRopePtr createropebody(ObjectMeshDynamicPtr OMD, float mass);

	ObjectMeshDynamicPtr upper_winchrope;
	ObjectMeshDynamicPtr lower_winchrope;

	JointParticlesPtr joint3;
	JointParticlesPtr joint4;
	JointBallPtr joint5;
	JointBallPtr joint6;
	JointBallPtr joint7;
};

