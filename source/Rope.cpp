#include "Rope.h"




void Rope::init()
{
	Rope_intial_point_node = Resource::get()->Rope_intial_point->clone();
	Rope_intial_point_node->setEnabled(1);
	rope_id = Rope_intial_point_node->getID();
	Rope_intial_point = ObjectDummy::cast(Rope_intial_point_node);
	BodyDummy::create(Rope_intial_point->getObject());

	handle_node = Resource::get()->Handle->clone();

	handlebody = handle_node->getObjectBodyRigid();

	hook_node = Resource::get()->currentHook->clone();
	hook_node->release();
	hookBody = hook_node->getObjectBodyRigid();

	basket_node = Resource::get()->weight_basket->clone();
	basketBody = basket_node->getObjectBodyRigid();

	weight_strop_node = Resource::get()->weight_strop->clone();
	stropBody = weight_strop_node->getObjectBodyRigid();
	weight_stretcher_node = Resource::get()->weight_stretcher->clone();
	stretcherBody = weight_stretcher_node->getObjectBodyRigid();
	
	rope_upper = Resource::get()->rope_upper->clone();
	rope_lower = Resource::get()->rope_lower->clone();

	rope_upper->setEnabled(1);
	rope_lower->setEnabled(1);
	handle_node->setEnabled(1);
	hook_node->setEnabled(1);
	
	rope_upper->setParent(Rope_intial_point_node);
	rope_lower->setParent(Rope_intial_point_node);
	handle_node->setParent(Rope_intial_point_node);
	hook_node->setParent(Rope_intial_point_node);
	
	upper_winchrope = ObjectMeshDynamic::cast(rope_upper);
	lower_winchrope = ObjectMeshDynamic::cast(rope_lower);
	
	ropebody1 = createropebody(upper_winchrope, Resource::get()->rope_mass_upper);
	ropebody2 = createropebody(lower_winchrope, Resource::get()->rope_mass_lower);

	joint1 = JointParticles::create(Rope_intial_point->getBody(), ropebody1->getBody(), Rope_intial_point->getWorldPosition(), vec3(0.04));
	joint2 = JointParticles::create(handlebody->getBody(), ropebody1->getBody(), handle_node->getWorldPosition(), vec3(0.16));
	joint3 = JointParticles::create(handlebody->getBody(), ropebody2->getBody(), handle_node->getWorldPosition(), vec3(0.2));
	joint4 = JointParticles::create(hookBody->getBody(), ropebody2->getBody(), hook_node->getWorldPosition(), vec3(0.1));
	
	joint5 = JointBall::create(hookBody->getBody(), basketBody->getBody());
	joint5->setWorldAxis(vec3(1.0f, 0.0f, 0.0f));
	joint5->setAngularDamping(0);
	joint5->setNumIterations(16);
	joint5->setLinearRestitution(0.8f);
	joint5->setAngularRestitution(0.8f);
	joint5->setLinearSoftness(0.0f);
	joint5->setAngularSoftness(0.0f);


	joint6 = JointBall::create(hookBody->getBody(), stropBody->getBody());
	joint6->setWorldAxis(vec3(1.0f, 0.0f, 0.0f));
	joint6->setAngularDamping(0);
	joint6->setNumIterations(16);
	joint6->setLinearRestitution(0.8f);
	joint6->setAngularRestitution(0.8f);
	joint6->setLinearSoftness(0.0f);
	joint6->setAngularSoftness(0.0f);

	joint7 = JointBall::create(hookBody->getBody(), stretcherBody->getBody());
	joint7->setWorldAxis(vec3(1.0f, 0.0f, 0.0f));
	joint7->setAngularDamping(0);
	joint7->setNumIterations(16);
	joint7->setLinearRestitution(0.8f);
	joint7->setAngularRestitution(0.8f);
	joint7->setLinearSoftness(0.0f);
	joint7->setAngularSoftness(0.0f);

	handle_node->getObjectBodyRigid()->getShape(0)->setMass(Resource::get()->handle_mass);
	hook_node->getObjectBodyRigid()->getShape(0)->setMass(Resource::get()->hook_mass);
	basket_node->getObjectBodyRigid()->getShape(0)->setMass(1);
	weight_strop_node->getObjectBodyRigid()->getShape(0)->setMass(1);
	weight_stretcher_node->getObjectBodyRigid()->getShape(0)->setMass(1);

	handle_objmovable = new ObjMovable(handle_node);
	hook_objmovable = new ObjMovable(hook_node);

}

void Rope::update()
{	
	hook_node->setRotation(quat(0.275, 359.750, 0.241));
}


BodyRopePtr Rope::createropebody(ObjectMeshDynamicPtr OMD, float mass) {
	//assigning a rope body to the dynamic mesh object and setting rope parameters
	BodyRopePtr body = BodyRope::create(OMD->getObject());
	body->setMass(mass);
	//body->setRadius(0.05f);
	body->setLinearDamping(0.0f);
	body->setAngularRestitution(0.0);
	body->setFriction(0.0f);
	body->setRestitution(0.0f);
	///the extent of rope moving 1 biggest 0 none
	body->setRigidity(0.0f);
	//body->setLinearStretch(0.0f);
	body->setNumIterations(24);
	return body;
}