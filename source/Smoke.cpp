#include "Smoke.h"
#include "Utils.h"
#include <UnigineGame.h>
#include <UnigineRender.h>
#include <UnigineWorlds.h>
#include <UnigineEditor.h>
//#include "../Utils/SoundManager.h"

#define PI 3.14159265

#define SOUND_THRESHOLD 0.1f
#define MAX_ATTACH_HAND_VELOCITY 2.0f
#define IMPACT_SOUND_GAIN 1.0f
#define SLIDE_SOUND_GAIN 0.1f

const float CHANGE_INTERVAL_SMOKE = 2.0f; // the interval of timekeeping
const float CLOSE_INTERVAL_SMOKE = 180.0f; // the interval of timekeeping

Map<int, Smoke*> Smoke::links;

int Smoke::checkObject(const PropertyPtr &prop)
{
	if (prop)
	{
		int index = prop->findParameter("grable");
		if (index != -1)
			return 1;
	}
	return 0;
}

Smoke* Smoke::getSmoke(const NodePtr &node)
{
	auto it = links.find(node->getID());
	if (it != links.end())
		return it->data;

	return nullptr;
}

void Smoke::init(const NodePtr &node)
{
	this->node = node;
	PropertyPtr prop = node->getProperty();
	// read properties
	if (prop)
	{
		use_handy_pos = prop->getParameterToggle(prop->findParameter("use_handy_pos"));
		handy_pos = prop->getParameterVec3(prop->findParameter("handy_pos"));
		vec3 handy_rot_euler = prop->getParameterVec3(prop->findParameter("handy_rot"));
		handy_rot = quat(handy_rot_euler.x, handy_rot_euler.y, handy_rot_euler.z);
		handy_pos_vr = prop->getParameterVec3(prop->findParameter("handy_pos_vr"));
		handy_rot_euler = prop->getParameterVec3(prop->findParameter("handy_rot_vr"));
		/// for the controller the value is 85 180 0
		///for glove the value is -90 180 0
		handy_rot_vr = quat(handy_rot_euler.x, handy_rot_euler.y, handy_rot_euler.z);
		is_door_handle = prop->getParameterToggle(prop->findParameter("door_handle"));
	}

	cap_prop = Properties::get()->findProperty("cap");
	tab_prop = Properties::get()->findProperty("tab");

	body = node->getObjectBodyRigid();
	status = CONNECTED;
	
	if (body)
	{
		initialMass = body->getBody()->getShape(0)->getMass();
		float mass = powf(itof(0), 3) * 0.1f;
		float damping = lerp_clamped(0.0f, 1.0f, 0.5f - mass * 10.0f);

		body->setLinearDamping(damping);
		body->setAngularDamping(damping);
		body->setFrozen(1);

		shapes_mass.clear();
		for (int i = 0; i < body->getNumShapes(); i++)
			shapes_mass.append(body->getShape(i)->getMass());

		body->setPositionCallback(MakeCallback(this, &Smoke::position_callback));
	//	body->setContactCallback(MakeCallback(this, &ObjMovable::contact_callback));
		body->setFrozenCallback(MakeCallback(this, &Smoke::frozen_callback));
	//	play_sound_action_id = onContact.reserveId();
		unfreeze_action_id = onContact.reserveId();
		mute_action_id = onFrozen.reserveId();
	}
	
	initialPosition = node->getWorldPosition();
	initialRotation = node->getWorldRotation();

	links[node->getID()] = this;

	last_pos_index = 0;
	last_pos[0] = last_pos[1] = node->getWorldPosition();

	//set the smoke as the parent
	if (node->getParent() && !(std::string(node->getParent()->getName()) == "Box" ))
	{
		parent = new Smoke(node->getParent());
	}

	elapsed_time_smoke = CHANGE_INTERVAL_SMOKE;
	elapsed_time_close = CLOSE_INTERVAL_SMOKE;
}

void Smoke::shutdown()
{
	links.remove(node->getID());
}

void Smoke::grabIt(VR* vr, const NodePtr &hand, int hand_num, int dof, const vec3 &linear_velocity, const vec3 &angular_velocity)
{
	// get local transform of the node to the hand transform
	if (this->use_handy_pos && is_door_handle == 0)
	{
		hand->setEnabled(0);
		transform = hand->getIWorldTransform() * node->getWorldTransform();
	}

	if (std::string(node->getName()) == "smoke_canister")
	{
		lid->node->setProperty(cap_prop);
	}

	if (std::string(node->getName()) == "LidPArent")
	{
		tab->node->setProperty(tab_prop);
	}


	if (body)
	{
		// unfreeze and unimmovable (for contact callbacks)
		activate_body(body->getBody());

		// set mass to zero (objs will no longer jump on top)

		for (int i = 0; i < body->getNumShapes(); i++)
			body->getShape(i)->setMass(0);
		
		//disconect with parent
		NodePtr parentnode = node->getParent();
		///when droped and hold again, there is no parent		
	}

	//when grab the lid then can be grabed
	///if already thorwn, then doesnt execute, otherwise error as cannot find child
	//if (smoke_node_type == 1 && status!= THROWN)
	//{
	//	PropertyPtr prop = node->getChild(1)->getProperty();
	//	//will be used in can_grab_it function
	//	prop->setParameterToggle(prop->findParameter("door_handle"),1);
	//}


	hold = 1;
	status = DISCONNECTED;

}

void Smoke::grabIt(Glove* glove, const NodePtr &hand, int hand_num, int dof, const vec3 &linear_velocity, const vec3 &angular_velocity)
{
	// get local transform of the node to the hand transform
	if (this->use_handy_pos && is_door_handle == 0)
	{
		hand->setEnabled(1);
		transform = hand->getIWorldTransform() * node->getWorldTransform();
	}


	if (std::string(node->getName()) == "smoke_canister")
	{
			lid->node->setProperty(cap_prop);
	}

	if (std::string(node->getName()) == "LidPArent")
	{
			tab->node->setProperty(tab_prop);
	}

	if (body)
	{
		// unfreeze and unimmovable (for contact callbacks)
		activate_body(body->getBody());

		// set mass to zero (objs will no longer jump on top)

		for (int i = 0; i < body->getNumShapes(); i++)
			body->getShape(i)->setMass(0);

		//disconect with parent
		NodePtr parentnode = node->getParent();
		///when droped and hold again, there is no parent		
	}

	//when grab the lid then can be grabed
	///if already thorwn, then doesnt execute, otherwise error as cannot find child
//	if (smoke_node_type == 1 && status != THROWN)
//	{
//		PropertyPtr prop = node->getChild(1)->getProperty();
//		//will be used in can_grab_it function
//		prop->setParameterToggle(prop->findParameter("door_handle"), 1);
//	}


	hold = 1;
	status = DISCONNECTED;

}

void Smoke::grabIt(LeapMotion* glove, const NodePtr &hand, int hand_num, int dof, const vec3 &linear_velocity, const vec3 &angular_velocity)
{
	// get local transform of the node to the hand transform
	if (this->use_handy_pos && is_door_handle == 0)
	{
		hand->setEnabled(1);
		transform = hand->getIWorldTransform() * node->getWorldTransform();
	}

	if (std::string(node->getName()) == "smoke_canister")
	{
		lid->node->setProperty(cap_prop);
	}

	if (std::string(node->getName()) == "LidPArent")
	{
		tab->node->setProperty(tab_prop);
	}

	if (body)
	{
		// unfreeze and unimmovable (for contact callbacks)
		activate_body(body->getBody());

		// set mass to zero (objs will no longer jump on top)

		for (int i = 0; i < body->getNumShapes(); i++)
			body->getShape(i)->setMass(0);

		//disconect with parent
		NodePtr parentnode = node->getParent();
		///when droped and hold again, there is no parent		
	}

	hold = 1;
	status = DISCONNECTED;

}

//if connected, have no rigidbody attribute
void Smoke::update()
{
	
	if (node->getParent() && smoke && smoke_node_type!=1 )
	{
		
		if (( smoke->status == CONNECTED || smoke->status == DISCONNECTED )&& status == CONNECTED)
		{	
			if (smoke_node_type == 2)
			{
				node->setTransform(initialPos_lid);
			}
			else if (smoke_node_type == 3)
			{
				node->setTransform(initialPos_tab);
			}	
		}
		//when throw with lid on it
		else if (status == CONNECTED  || smoke->status == THROWN)
		{
			body->setEnabled(0);	
			
			if (smoke_node_type == 2)
			{
				node->setTransform(initialPos_lid);
			}
			else if (smoke_node_type == 3)
			{
				node->setTransform(initialPos_tab);
			}
		}
		else
		{
			body->setEnabled(1);
		}
			

	}
	else if (status == CONNECTED)
		{
			body->setEnabled(0);
		}
		else
			body->setEnabled(1);

	//began to emit effect
	///because elapsed_time_smoke belongs to tab so usy type ==3 to decide 
	if (smoke_node_type == 3 && (status == DISCONNECTED || status == THROWN))
	{
		isTimingEffect = true;
		isTimingEffectClosed = true;
	}
	else
	{
		isTimingEffect = false;
		isTimingEffectClosed = false;
	}

	if (smoke_node_type == 1 && node->getWorldPosition().z >= 0 && status == THROWN)
	{
		if (node->getWorldPosition().z <= smokeHeight)
		{
			node->setWorldPosition(Vec3(node->getWorldPosition().x, node->getWorldPosition().y, smokeHeight + 0.1));
		}
	}

	// reset the z position of smoke
	if (smoke_node_type == 1 && node->getWorldPosition().z < 0)
	{
		node->setWorldPosition(Vec3(node->getWorldPosition().x, node->getWorldPosition().y, 0));
	
	}



}

void Smoke::set_outline(ObjectPtr node, int enable)
{

	for (int i = 0; i < node->getNumSurfaces(); i++)
		node->setMaterialState("auxiliary", enable, i);

	// set post materials
	if (enable)
		node->getObject()->getMaterial(0)->setState("emission", 1);
}

void Smoke::holdIt(VR* vr, const NodePtr &hand, int hand_num, int dof, const vec3 &linear_velocity, const vec3 &angular_velocity)
{
	// update local position and rotation
		float ifps = Game::get()->getIFps() / Game::get()->getScale();
		setTransformPosition(transform, lerp(transform.getTranslate(), Vec3(vr->getHandyPos() + (dof < 6 ? handy_pos : handy_pos_vr)), handy_pos_factor * ifps));
		setTransformRotation(transform, slerp(transform.getRotate(), vr->getHandyRot() * (dof < 6 ? handy_rot : handy_rot_vr), handy_pos_factor * ifps));
		// update world transform of node relative hand
		node->setWorldTransform(hand->getWorldTransform() * transform);

}

void Smoke::holdIt(Glove* glove, const NodePtr &hand, int hand_num, int dof, const vec3 &linear_velocity, const vec3 &angular_velocity)
{
	// update local position and rotation
	float ifps = Game::get()->getIFps() / Game::get()->getScale();
	setTransformPosition(transform, lerp(transform.getTranslate(), Vec3(glove->getHandyPos() + (dof < 6 ? handy_pos : handy_pos_vr)), handy_pos_factor * ifps));
	setTransformRotation(transform, slerp(transform.getRotate(), glove->getHandyRot() * (dof < 6 ? handy_rot : handy_rot_vr), handy_pos_factor * ifps));
	// update world transform of node relative hand
	node->setWorldTransform(hand->getWorldTransform() * transform);
	Vec3 cupos = hand->getWorldTransform() * transform.getTranslate();
	node->setWorldPosition(Vec3(cupos.x + VR::get()->handyPosX, cupos.y + VR::get()->handyPosY, cupos.z + VR::get()->handyPosZ));
}


void Smoke::holdIt(LeapMotion* glove, const NodePtr &hand, int hand_num, int dof, const vec3 &linear_velocity, const vec3 &angular_velocity)
{
	// update local position and rotation
	float ifps = Game::get()->getIFps() / Game::get()->getScale();
	setTransformPosition(transform, lerp(transform.getTranslate(), Vec3(glove->getHandyPos() + (dof < 6 ? handy_pos : handy_pos_vr)), handy_pos_factor * ifps));
	setTransformRotation(transform, slerp(transform.getRotate(), glove->getHandyRot() * (dof < 6 ? handy_rot : handy_rot_vr), handy_pos_factor * ifps));
	// update world transform of node relative hand
	node->setWorldTransform(hand->getWorldTransform() * transform);
}

void Smoke::throwIt(VR* vr, const NodePtr &hand, int hand_num, int dof, const vec3 &linear_velocity, const vec3 &angular_velocity)
{

	hold = 0;
	NodePtr parentnode = node->getParent();
	///when droped and hold again, there is no parent
	if (parentnode)
	{
		parentnode->removeWorldChild(node);
	}


	if (this->use_handy_pos)
		hand->setEnabled(1);



	if( body )
	{
		// set movable and unfrozen
		activate_body(body->getBody());
		// add impulse
		body->getShape(0)->setMass(1);
		body->setGravity(1);
		body->setLinearVelocity(linear_velocity * 3);
		//disable it as the effect is not very good
		body->setAngularVelocity(angular_velocity * 3);
		status = THROWN;
	}

}

void Smoke::throwIt(Glove* glove, const NodePtr &hand, int hand_num, int dof, const vec3 &linear_velocity, const vec3 &angular_velocity)
{

	hold = 0;
	NodePtr parentnode = node->getParent();
	///when droped and hold again, there is no parent
	if (parentnode)
	{
		parentnode->removeWorldChild(node);
	}


	if (this->use_handy_pos)
		hand->setEnabled(1);



	if (body)
	{
		// set movable and unfrozen
		activate_body(body->getBody());
		// add impulse
		body->getShape(0)->setMass(1);
		body->setGravity(1);
		body->setLinearVelocity(linear_velocity * 3);
		//disable it as the effect is not very good
		body->setAngularVelocity(angular_velocity * 3);
		status = THROWN;
	}

}

void Smoke::throwIt(LeapMotion* glove, const NodePtr &hand, int hand_num, int dof, const vec3 &linear_velocity, const vec3 &angular_velocity)
{
	hold = 0;
	NodePtr parentnode = node->getParent();
	///when droped and hold again, there is no parent
	if (parentnode)
	{
		parentnode->removeWorldChild(node);
	}

	if (this->use_handy_pos)
		hand->setEnabled(1);

	if (body)
	{
		// set movable and unfrozen
		activate_body(body->getBody());
		// add impulse
		body->getShape(0)->setMass(1);
		body->setGravity(1);
		body->setLinearVelocity(linear_velocity);
		//disable it as the effect is not very good
		//body->setAngularVelocity(angular_velocity);
		status = THROWN;
	}

}

void Smoke::unfreeze(const BodyPtr &body)
{
	if (!body) return;

	PropertyPtr prop = body->getObject()->getNode()->getProperty();
	if (prop && checkObject(prop) == 1)
		activate_body(body);
}

void Smoke::mute_action(const BodyPtr &body)
{
	// unsubscribe
	onContact.removeListener(unfreeze_action_id);
	//onContact.removeListener(play_sound_action_id);
	onFrozen.removeListener(mute_action_id);
}



void Smoke::activate_body(const BodyPtr &body)
{
	body->setImmovable(0);
	body->setFrozen(0);
}

void Smoke::unfreeze_action(const BodyPtr &body, int num)
{
	if (num >= body->getNumContacts()) return;

	unfreeze(body->getContactBody0(num));
	unfreeze(body->getContactBody1(num));
}




