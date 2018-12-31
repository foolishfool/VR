#include "ObjMovable.h"
#include "Utils.h"
#include "VR.h"
#include "Resource.h"
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

// we don't want to get intersection with himself, so
// we use offset
#define RAY_OFFSET 0.05f

const float CLOSE_INTERVAL_EFFECT= 300.0f; // the interval of timekeeping

Map<int, ObjMovable*> ObjMovable::links;



int ObjMovable::checkObject(const PropertyPtr &prop)
{
	if (prop)
	{
		int index = prop->findParameter("grable");
		if (index != -1)
			return 1;
	}
	return 0;

}

ObjMovable* ObjMovable::getObjMovable(const NodePtr &node)
{
	auto it = links.find(node->getID());
	if (it != links.end())
		return it->data;

	return nullptr;
}

void ObjMovable::init(const NodePtr &node)
{
	this->node = node;
//	land = ObjectTerrain::cast(LoadFromEditor("Land"));
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

		body->setPositionCallback(MakeCallback(this, &ObjMovable::position_callback));
	//	body->setContactCallback(MakeCallback(this, &ObjMovable::contact_callback));
		body->setFrozenCallback(MakeCallback(this, &ObjMovable::frozen_callback));
	//	play_sound_action_id = onContact.reserveId();
		unfreeze_action_id = onContact.reserveId();
		mute_action_id = onFrozen.reserveId();
	}
	
	initialPosition = node->getWorldPosition();
	initialRotation = node->getWorldRotation();

	handle = Resource::get()->Handle;
	point = Resource::get()->Rope_intial_point;
    door_reference = LoadFromEditor("door_handle_reference");
	door_reference_closed = LoadFromEditor("door_handle_reference_closed");
	door_reference1 = LoadFromEditor("door_handle_reference_1");
	door_reference_closed1 = LoadFromEditor("door_handle_reference_closed_1");
	///only lasesrpointer has children
	if (std::string(node->getName()) == "laser_pointer")
	{
		///cannot use LoadFromEditor as there are many node 
		laser_light = node->getChild(node->findChild("laser_light"));
		laser_ray = laser_light->getChild(laser_light->findChild("laser_ray"));
		laser_hit = laser_light->getChild(laser_light->findChild("laser_hit"));
		laser_ray_mat = laser_ray->getTransform();
		laserpointer_mat = ObjectMeshDynamic::cast(node)->getMaterial(0);
	}

	if (std::string(node->getName()) == "flashlight")
	{
	
		flash_light = node->getChild(node->findChild("flash_light"));
		flash = LightProj::cast(flash_light);
		flash2 = LightProj::cast(flash_light->getChild(flash_light->findChild("flash_light_1")));
		flashlight_ray = flash_light->getChild(flash_light->findChild("flashlight_ray"));
		flashlight_ray_initial_length = flashlight_ray->getScale().y;
		flashlight_ray_mat = flashlight_ray->getTransform();
		flashlight_volume = ObjectVolumeProj::cast(flash_light->getChild(flash_light->findChild("flashlight_volume")));
		flashlight_volume_initial_length = flashlight_volume->getRadius();
		flashlight_mat = ObjectMeshDynamic::cast(node)->getMaterial(0);
	}

	if (std::string(node->getName()) == "SeaMark") 
	{
		elapsed_time_close = CLOSE_INTERVAL_EFFECT;
	}


	links[node->getID()] = this;

	last_pos_index = 0;
	last_pos[0] = last_pos[1] = node->getWorldPosition();

	//set the smoke as the parent
	if (node->getParent() && !(std::string(node->getParent()->getName()) == "Box" ))
	{
		parent = new ObjMovable(node->getParent());
	}

}

void ObjMovable::shutdown()
{
	links.remove(node->getID());
}

void ObjMovable::grabIt(VR* vr, const NodePtr &hand, int hand_num, int dof, const vec3 &linear_velocity, const vec3 &angular_velocity)
{
	// get local transform of the node to the hand transform
	if (this->use_handy_pos && is_door_handle == 0)
	{
		hand->setEnabled(0);
		transform = hand->getIWorldTransform() * node->getWorldTransform();
	}
	else if (std::string(node->getName())== "door_handle")
	{
		hand->setEnabled(0);
		transform = hand->getIWorldTransform() * node->getWorldTransform();
		Vec3 currentPosition = node->getPosition();
		node->setPosition(Vec3(door_reference->getPosition().x, currentPosition.y, door_reference->getPosition().z));
		node->setRotation(door_reference->getRotation());
	}
	else if (std::string(node->getName()) == "door_handle_1")
	{
		hand->setEnabled(0);
		transform = hand->getIWorldTransform() * node->getWorldTransform();
		Vec3 currentPosition = node->getPosition();
		node->setPosition(Vec3(door_reference1->getPosition().x, currentPosition.y, door_reference1->getPosition().z));
		node->setRotation(door_reference1->getRotation());
	}
	
	if (body)
	{
		// unfreeze and unimmovable (for contact callbacks)
		activate_body(body->getBody());

		// set mass to zero (objs will no longer jump on top)

		for (int i = 0; i < body->getNumShapes(); i++)
			body->getShape(i)->setMass(0);
	}

	hold = 1;
	status = DISCONNECTED;

}

void ObjMovable::grabIt(Glove* glove,const NodePtr &hand, int hand_num, int dof, const vec3 &linear_velocity, const vec3 &angular_velocity)
{
	// get local transform of the node to the hand transform
	if (this->use_handy_pos && is_door_handle == 0)
	{
		hand->setEnabled(1);
		transform = hand->getIWorldTransform() * node->getWorldTransform();
	}
	else if (std::string(node->getName()) == "door_handle")
	{
		hand->setEnabled(1);
		transform = hand->getIWorldTransform() * node->getWorldTransform();
		Vec3 currentPosition = node->getPosition();
		node->setPosition(Vec3(door_reference->getPosition().x, currentPosition.y, door_reference->getPosition().z));
		node->setRotation(door_reference->getRotation());
	}
	else if (std::string(node->getName()) == "door_handle_1")
	{
		hand->setEnabled(1);
		transform = hand->getIWorldTransform() * node->getWorldTransform();
		Vec3 currentPosition = node->getPosition();
		node->setPosition(Vec3(door_reference1->getPosition().x, currentPosition.y, door_reference1->getPosition().z));
		node->setRotation(door_reference1->getRotation());
	}

	if (body)
	{
		// unfreeze and unimmovable (for contact callbacks)
		activate_body(body->getBody());

		// set mass to zero (objs will no longer jump on top)

		for (int i = 0; i < body->getNumShapes(); i++)
			body->getShape(i)->setMass(0);
	}

	hold = 1;
	status = DISCONNECTED;

}


void ObjMovable::grabIt(LeapMotion* glove, const NodePtr &hand, int hand_num, int dof, const vec3 &linear_velocity, const vec3 &angular_velocity)
{
	// get local transform of the node to the hand transform
	if (this->use_handy_pos && is_door_handle == 0)
	{
		hand->setEnabled(1);
		transform = hand->getIWorldTransform() * node->getWorldTransform();
	}
	else if (std::string(node->getName()) == "door_handle")
	{
		hand->setEnabled(1);
		transform = hand->getIWorldTransform() * node->getWorldTransform();
		Vec3 currentPosition = node->getPosition();
		node->setPosition(Vec3(door_reference->getPosition().x, currentPosition.y, door_reference->getPosition().z));
		node->setRotation(door_reference->getRotation());
	}
	else if (std::string(node->getName()) == "door_handle_1")
	{
		hand->setEnabled(1);
		transform = hand->getIWorldTransform() * node->getWorldTransform();
		Vec3 currentPosition = node->getPosition();
		node->setPosition(Vec3(door_reference1->getPosition().x, currentPosition.y, door_reference1->getPosition().z));
		node->setRotation(door_reference1->getRotation());
	}

	if (body)
	{
		// unfreeze and unimmovable (for contact callbacks)
		activate_body(body->getBody());

		// set mass to zero (objs will no longer jump on top)

		for (int i = 0; i < body->getNumShapes(); i++)
			body->getShape(i)->setMass(0);
	}

	hold = 1;
	status = DISCONNECTED;

}

//if connected, have no rigidbody attribute
void ObjMovable::update()
{
	if (std::string(node->getName()) == "SeaMark")
	{
		if (status == THROWN && node->getWorldPosition().z < 3.9)
		{
			seamark_effect->setEnabled(1);
			seamark_effect->setEmitterEnabled(1);
			//begin to timing
			isTimingEffectClosed = true;
		}

		if (status == THROWN && node->getWorldPosition().z < 4)
		{
			node->setWorldPosition(Vec3(node->getWorldPosition().x, node->getWorldPosition().y, 4));
			seamark_effect->setWorldRotation(quat(0, 0, 0));
		}
	}


	if (std::string(node->getName()) == "laser_pointer")
	{
		if (laser_light->isEnabled())
		{
			// calculate laser hit position
			laser_ray->setTransform(laser_ray_mat);

			Vec3 dir = -Vec3(getWorldBack(laser_ray));
			Vec3 p0 = laser_ray->getWorldPosition() + dir * RAY_OFFSET;
			Vec3 p1 = p0 + dir * 1000;

			ObjectPtr hit_obj = World::get()->getIntersection(p0, p1, 1, intersection);
		//if (hit_obj)
		//{
		//	laser_ray->setScale(vec3(laser_ray->getScale().x, (float)length(intersection->getPoint() - p0) + RAY_OFFSET, laser_ray->getScale().z));
		//	laser_hit->setWorldPosition(intersection->getPoint());
		//	laser_hit->setEnabled(1);
		//}
		//else
		//	laser_hit->setEnabled(0);
		}


	}

	if (std::string(node->getName()) == "flashlight")
	{

		if (flash_light->isEnabled())
		{
			// calculate laser hit position

			flashlight_ray->setTransform(flashlight_ray_mat);

			Vec3 dir = -Vec3(getWorldBack(flashlight_ray));
			Vec3 p0 = flashlight_ray->getWorldPosition() + dir * RAY_OFFSET;
			Vec3 p1 = p0 + dir * 1000;
			ObjectPtr hit_obj = World::get()->getIntersection(p0, p1, 1, intersection);
			if (hit_obj)
			{
				flashlight_ray->setScale(vec3(flashlight_ray->getScale().x, (float)length(intersection->getPoint() - p0) + RAY_OFFSET, flashlight_ray->getScale().z));
				float flashlight_ray_length = (float)length(intersection->getPoint() - p0) + RAY_OFFSET;
				//the initial 
				float off_length = flashlight_ray_length / flashlight_ray_initial_length;
				flashlight_volume->setRadius(off_length * flashlight_volume_initial_length);
			}
		}

	//	Log::message("%f \n", node->getNodeWorldPosition().z);
	}


	//setNodeHeight(node, land);
}
//set direction of ray 
void ObjMovable::setRayDirection(NodePtr laser_ray, Vec3 head_position)
{
	if ( std::string(node->getName()) == "laser_pointer")
	{
		// rotate ray plane to player's head (Y fixed, Z floats)
		setWorldDirectionY(laser_ray, getWorldBack(laser_ray), vec3(head_position - laser_ray->getWorldPosition()));
	}
	
}

void ObjMovable::hideRay()
{
	if (std::string(node->getName()) == "laser_pointer")
	{
		laser_ray->setEnabled(0);
	}
}

void ObjMovable::showRay()
{
	if (std::string(node->getName()) == "laser_pointer")
	{
		laser_ray->setEnabled(1);
	}
}

void ObjMovable::clear()
{
	if (std::string(node->getName()) == "flashlight")
	{
		flashlight_volume->setStep(0.176);
		flash->setIntensity(5);
		flash2->setIntensity(5);
		flashlight_volume->getMaterial(0)->setParameter("diffuse_color", vec4(0.05f, 0.05f, 0.05f, 1));
	}
}

void ObjMovable::dim()
{
	if (std::string(node->getName()) == "flashlight")
	{
		flashlight_volume->setStep(0.573);
		flashlight_volume->getMaterial(0)->setParameter("diffuse_color", vec4(0.04,0.04,0.04,1));
		flash->setIntensity(0.5);
		flash2->setIntensity(2.5);
	}
}

void ObjMovable::followtheHook(NodePtr &hand, NodePtr &handle)
{
	//set direction
	Vec3 positionB = hand->getWorldPosition();
	Vec3 positionA = point->getWorldPosition();
	vec3 direction = vec3(positionB.x - positionA.x, positionB.y - positionA.y,  positionB.z - positionA.z).normalize();
	///Node::AXIS_Y 2.7.1
	handle->setWorldDirection(direction,getWorldUp(hand),Node::AXIS_NY);

	//set position	
	float distance_B_A = sqrtf(powf(positionB.x - positionA.x, 2) + powf(positionB.y - positionA.y, 2) + powf(positionB.z - positionA.z, 2));
	//store current handle z position
	if (isHandlePositionReset)
	{
		distance_z = point->getWorldPosition().z - handle->getWorldPosition().z;
		isHandlePositionReset = false;
	}	
	float proportion = distance_z / distance_B_A;
	//lerp funciton the third argument is the percentage from position A to position B
	handle->setWorldPosition(lerp(positionA, positionB, proportion));
}

void ObjMovable::followtheHook(NodePtr &hand)
{
	//set direction
	Vec3 positionB = hand->getWorldPosition();
	Vec3 positionA = point->getWorldPosition();
	vec3 direction = vec3(positionB.x - positionA.x, positionB.y - positionA.y, positionB.z - positionA.z).normalize();
	///Node::AXIS_Y 2.7.1
	handle->setWorldDirection(direction, getWorldUp(hand), Node::AXIS_NY);

	//set position	
	float distance_B_A = sqrtf(powf(positionB.x - positionA.x, 2) + powf(positionB.y - positionA.y, 2) + powf(positionB.z - positionA.z, 2));
	//store current handle z position
	if (isHandlePositionReset)
	{
		distance_z = point->getWorldPosition().z - handle->getWorldPosition().z;
		isHandlePositionReset = false;
	}
	float proportion = distance_z / distance_B_A;
	//lerp funciton the third argument is the percentage from position A to position B
	handle->setWorldPosition(lerp(positionA, positionB, proportion));
}


void  ObjMovable::setNodeHeight(NodePtr node , ObjectTerrainPtr land)
{

	float currentpointerHeight = land->getWorldHeight(node->getWorldPosition());
	if (currentpointerHeight >= 0)
	{
		if (node->getWorldPosition().z <= currentpointerHeight)
		{
			node->setWorldPosition(Vec3(node->getWorldPosition().x, node->getWorldPosition().y, currentpointerHeight + 0.1));
		}
	}
	
	if (node->getWorldPosition().z < 0)
	{
		node->setWorldPosition(Vec3(node->getWorldPosition().x, node->getWorldPosition().y, 0.1));
	}
}


int ObjMovable::hit_detect( Vector<ObjectPtr> nodes) {

	if (World::get()->getIntersection(this->node->getWorldBoundBox(),  nodes))
	{
		int nearest = -1;
		Scalar min_dist = UNIGINE_INFINITY;
		for (int k = 0; k < nodes.size(); k++)
		{
			if (isWeight(nodes[k]->getNode()))
			{
				Scalar dist = length2(nodes[k]->getWorldPosition() - this->node->getWorldPosition());
				if (min_dist > dist)
				{
					min_dist = dist;
					nearest = k;
				}
			}
		}

		// select nearest object
		if (nearest >= 0)
		{
			weight = nodes[nearest];
			set_outline(nodes[nearest], 1);
			return 1;
		}
		else 
		{
			if (weight)
				set_outline(weight, 0);
			return 0;
		}
			
	}
}

void ObjMovable::set_outline(ObjectPtr node, int enable)
{

	for (int i = 0; i < node->getNumSurfaces(); i++)
		node->setMaterialState("emission", enable, i);
	if (true)
	{

	}
}

int ObjMovable::isWeight(const NodePtr &node)
{
	PropertyPtr prop = node->getProperty();
	if (std::string(prop->getName() )== "weight")
	{
		return 1;
	}
	else return 0;
}

void ObjMovable::attach(ObjectPtr weight) 
{
	weight->setWorldPosition(Vec3(node->getWorldPosition().x, node->getWorldPosition().y, node->getWorldPosition().z - 0.05));
	weight->setWorldParent(node);
	weight->setRotation(quat(0, 0, 90));
}

void ObjMovable::detach(ObjectPtr weight)
{
	weight->setWorldParent(LoadFromEditor("Box"));
//	weight->setWorldRotation(quat(0, 0, 0));	
}


void ObjMovable::holdIt(VR* vr, const NodePtr &hand, int hand_num, int dof, const vec3 &linear_velocity, const vec3 &angular_velocity)
{
	// update local position and rotation
	if (this->use_handy_pos && is_door_handle == 0)
	{
		float ifps = Game::get()->getIFps() / Game::get()->getScale();
		setTransformPosition(transform, lerp(transform.getTranslate(), Vec3(vr->getHandyPos() + (dof < 6 ? handy_pos : handy_pos_vr)), handy_pos_factor * ifps));
		setTransformRotation(transform, slerp(transform.getRotate(), vr->getHandyRot() * (dof < 6 ? handy_rot : handy_rot_vr), handy_pos_factor * ifps));
		// update world transform of node relative hand
		node->setWorldTransform(hand->getWorldTransform() * transform);

	}
	else if (std::string(node->getName()) == "door_handle") //door
	{

			float ifps = Game::get()->getIFps() / Game::get()->getScale();
			setTransformPosition(transform, lerp(transform.getTranslate(), Vec3(vr->getHandyPos() + (dof < 6 ? handy_pos : handy_pos_vr)), handy_pos_factor * ifps));
			//setposition
			node->setWorldPosition(hand->getWorldTransform() * transform.getTranslate());
			Vec3 currentPosition = node->getPosition();

			node->setPosition(Vec3(door_reference->getPosition().x, currentPosition.y, door_reference->getPosition().z));

			if (node->getPosition().y < door_reference_closed->getPosition().y)
			{
				node->setPosition( Vec3(door_reference->getPosition().x,door_reference_closed->getPosition().y, door_reference->getPosition().z));
			}
			else if (node->getPosition().y > door_reference->getPosition().y)
			{
				node->setPosition(Vec3(door_reference->getPosition().x, door_reference->getPosition().y, door_reference->getPosition().z));
			}
			//keep the door rotation 	
			node->setRotation(door_reference->getRotation());
	}
	else if (std::string(node->getName()) == "door_handle_1") //door
	{

		float ifps = Game::get()->getIFps() / Game::get()->getScale();
		setTransformPosition(transform, lerp(transform.getTranslate(), Vec3(vr->getHandyPos() + (dof < 6 ? handy_pos : handy_pos_vr)), handy_pos_factor * ifps));
		//setposition
		node->setWorldPosition(hand->getWorldTransform() * transform.getTranslate());
		Vec3 currentPosition = node->getPosition();

		node->setPosition(Vec3(door_reference1->getPosition().x, currentPosition.y, door_reference1->getPosition().z));

		if (node->getPosition().y < door_reference_closed1->getPosition().y)
		{
			node->setPosition(Vec3(door_reference1->getPosition().x, door_reference_closed1->getPosition().y, door_reference1->getPosition().z));
		}
		else if (node->getPosition().y > door_reference1->getPosition().y)
		{
			node->setPosition(Vec3(door_reference1->getPosition().x, door_reference1->getPosition().y, door_reference1->getPosition().z));
		}
		//keep the door rotation 	
		node->setRotation(door_reference1->getRotation());
	}

}

void ObjMovable::holdIt(Glove* glove, const NodePtr &hand, int hand_num, int dof, const vec3 &linear_velocity, const vec3 &angular_velocity)
{
	// update local position and rotation
	if (this->use_handy_pos && is_door_handle == 0)
	{
		float ifps = Game::get()->getIFps() / Game::get()->getScale();
		setTransformPosition(transform, lerp(transform.getTranslate(), Vec3(glove->getHandyPos() + (dof < 6 ? handy_pos : handy_pos_vr)), handy_pos_factor * ifps));
		setTransformRotation(transform, slerp(transform.getRotate(), glove->getHandyRot() * (dof < 6 ? handy_rot : handy_rot_vr), handy_pos_factor * ifps));
		// update world transform of node relative hand
		node->setWorldTransform(hand->getWorldTransform() * transform);

	}
	else if (std::string(node->getName()) == "door_handle") //door
	{

		float ifps = Game::get()->getIFps() / Game::get()->getScale();
		setTransformPosition(transform, lerp(transform.getTranslate(), Vec3(glove->getHandyPos() + (dof < 6 ? handy_pos : handy_pos_vr)), handy_pos_factor * ifps));
		//setposition
		node->setWorldPosition(hand->getWorldTransform() * transform.getTranslate());
		Vec3 currentPosition = node->getPosition();

		node->setPosition(Vec3(door_reference->getPosition().x, currentPosition.y, door_reference->getPosition().z));

		if (node->getPosition().y < door_reference_closed->getPosition().y)
		{
			node->setPosition(Vec3(door_reference->getPosition().x, door_reference_closed->getPosition().y, door_reference->getPosition().z));
		}
		else if (node->getPosition().y > door_reference->getPosition().y)
		{
			node->setPosition(Vec3(door_reference->getPosition().x, door_reference->getPosition().y, door_reference->getPosition().z));
		}
		//keep the door rotation 	
		node->setRotation(door_reference->getRotation());
	}
	else if (std::string(node->getName()) == "door_handle_1") //door
	{
		float ifps = Game::get()->getIFps() / Game::get()->getScale();
		setTransformPosition(transform, lerp(transform.getTranslate(), Vec3(glove->getHandyPos() + (dof < 6 ? handy_pos : handy_pos_vr)), handy_pos_factor * ifps));
		//setposition
		node->setWorldPosition(hand->getWorldTransform() * transform.getTranslate());
		Vec3 currentPosition = node->getPosition();

		node->setPosition(Vec3(door_reference1->getPosition().x, currentPosition.y, door_reference1->getPosition().z));

		if (node->getPosition().y < door_reference_closed1->getPosition().y)
		{
			node->setPosition(Vec3(door_reference1->getPosition().x, door_reference_closed1->getPosition().y, door_reference1->getPosition().z));
		}
		else if (node->getPosition().y > door_reference1->getPosition().y)
		{
			node->setPosition(Vec3(door_reference1->getPosition().x, door_reference1->getPosition().y, door_reference1->getPosition().z));
		}
		//keep the door rotation 	
		node->setRotation(door_reference1->getRotation());
	}

}


void ObjMovable::holdIt(LeapMotion* glove, const NodePtr &hand, int hand_num, int dof, const vec3 &linear_velocity, const vec3 &angular_velocity)
{
	// update local position and rotation
	if (this->use_handy_pos && is_door_handle == 0)
	{
		float ifps = Game::get()->getIFps() / Game::get()->getScale();
		setTransformPosition(transform, lerp(transform.getTranslate(), Vec3(glove->getHandyPos() + (dof < 6 ? handy_pos : handy_pos_vr)), handy_pos_factor * ifps));
		setTransformRotation(transform, slerp(transform.getRotate(), glove->getHandyRot() * (dof < 6 ? handy_rot : handy_rot_vr), handy_pos_factor * ifps));
		// update world transform of node relative hand
		node->setWorldTransform(hand->getWorldTransform() * transform);	
	}
	else if (std::string(node->getName()) == "door_handle") //door
	{

		float ifps = Game::get()->getIFps() / Game::get()->getScale();
		setTransformPosition(transform, lerp(transform.getTranslate(), Vec3(glove->getHandyPos() + (dof < 6 ? handy_pos : handy_pos_vr)), handy_pos_factor * ifps));
		//setposition
		node->setWorldPosition(hand->getWorldTransform() * transform.getTranslate());
		Vec3 currentPosition = node->getPosition();

		node->setPosition(Vec3(door_reference->getPosition().x, currentPosition.y, door_reference->getPosition().z));

		if (node->getPosition().y < door_reference_closed->getPosition().y)
		{
			node->setPosition(Vec3(door_reference->getPosition().x, door_reference_closed->getPosition().y, door_reference->getPosition().z));
		}
		else if (node->getPosition().y > door_reference->getPosition().y)
		{
			node->setPosition(Vec3(door_reference->getPosition().x, door_reference->getPosition().y, door_reference->getPosition().z));
		}
		//keep the door rotation 	
		node->setRotation(door_reference->getRotation());
	}
	else if (std::string(node->getName()) == "door_handle_1") //door
	{
		float ifps = Game::get()->getIFps() / Game::get()->getScale();
		setTransformPosition(transform, lerp(transform.getTranslate(), Vec3(glove->getHandyPos() + (dof < 6 ? handy_pos : handy_pos_vr)), handy_pos_factor * ifps));
		//setposition
		node->setWorldPosition(hand->getWorldTransform() * transform.getTranslate());
		Vec3 currentPosition = node->getPosition();

		node->setPosition(Vec3(door_reference1->getPosition().x, currentPosition.y, door_reference1->getPosition().z));

		if (node->getPosition().y < door_reference_closed1->getPosition().y)
		{
			node->setPosition(Vec3(door_reference1->getPosition().x, door_reference_closed1->getPosition().y, door_reference1->getPosition().z));
		}
		else if (node->getPosition().y > door_reference1->getPosition().y)
		{
			node->setPosition(Vec3(door_reference1->getPosition().x, door_reference1->getPosition().y, door_reference1->getPosition().z));
		}
		//keep the door rotation 	
		node->setRotation(door_reference1->getRotation());
	}

}

void ObjMovable::throwIt(VR* vr, const NodePtr &hand, int hand_num, int dof, const vec3 &linear_velocity, const vec3 &angular_velocity)
{

	hold = 0;

	if (this->use_handy_pos)
		hand->setEnabled(1);


	if( body )
	{
		// set movable and unfrozen
		activate_body(body->getBody());
		// add impulse	
		if ((std::string(node->getName()) != "Hook") && (std::string(node->getName()) !="Handle") && (std::string(node->getName()) != "door_handle") && (std::string(node->getName()) != "door_handle_1"))
		{//disconect with parent
			NodePtr parentnode = node->getParent();
			if (parentnode && ((std::string(node->getName()) != "Handle")) && ((std::string(node->getName()) != "Hook")))
			{
				parentnode->removeWorldChild(node);
			}
			body->getShape(0)->setMass(1);
			body->setLinearVelocity(linear_velocity);
			//body->setAngularVelocity(angular_velocity *3);
			
	//	   Log::message("%f, %f, %f linear_velocity \n ", linear_velocity.x , linear_velocity.y , linear_velocity.z );
	//	   Log::message("%f, %f, %f linear_velocity \n ", body->getLinearVelocity().x, body->getLinearVelocity().y, body->getLinearVelocity().z);
		}
		else
		{
			body->getShape(0)->setMass(100);
		}

	//	Log::message("%f  \n", body->getShape(0)->getMass());

		status = THROWN;

	}


}

void ObjMovable::throwIt(Glove* glove, const NodePtr &hand, int hand_num, int dof, const vec3 &linear_velocity, const vec3 &angular_velocity)
{

	hold = 0;

	if (this->use_handy_pos)
		hand->setEnabled(1);


	if (body)
	{
		// set movable and unfrozen
		activate_body(body->getBody());
		// add impulse	
		if ((std::string(node->getName()) != "Hook") && (std::string(node->getName()) != "Handle") && (std::string(node->getName()) != "door_handle") && (std::string(node->getName()) != "door_handle_1"))
		{//disconect with parent
			NodePtr parentnode = node->getParent();
			if (parentnode && ((std::string(node->getName()) != "Handle")) && ((std::string(node->getName()) != "Hook")))
			{
				parentnode->removeWorldChild(node);
			}
			body->getShape(0)->setMass(1);
			body->setGravity(1);
			body->setLinearVelocity(linear_velocity);
			body->setAngularVelocity(angular_velocity);
		}
		else
		{
			body->getShape(0)->setMass(100);
		}

		status = THROWN;

	}


}

void ObjMovable::throwIt(LeapMotion* glove, const NodePtr &hand, int hand_num, int dof, const vec3 &linear_velocity, const vec3 &angular_velocity)
{

	hold = 0;

	if (this->use_handy_pos)
		hand->setEnabled(1);
	if (body)
	{
		// set movable and unfrozen
		activate_body(body->getBody());
		// add impulse	
		if ((std::string(node->getName()) != "Hook") && (std::string(node->getName()) != "Handle") && (std::string(node->getName()) != "door_handle") && (std::string(node->getName()) != "door_handle_1"))
		{//disconect with parent
			NodePtr parentnode = node->getParent();
			if (parentnode )
			{
				parentnode->removeWorldChild(node);
			}
			body->getShape(0)->setMass(1);
			body->setGravity(1);
			body->setLinearVelocity(linear_velocity * 3);
			body->setAngularVelocity(angular_velocity);
		}
		else
		{
			body->getShape(0)->setMass(100);
		}
		status = THROWN;
	}
}

// laser pointer switch on/off (any laser pointer object)
void ObjMovable::useIt(int isOpen)
{
	if (laser_light)
	{
		if (isOpen)
		{
			laserpointer_mat->setParameterSlider(laserpointer_mat->findParameter("emission_scale"),30);
			laser_light->setEnabled(1);
			Resource::get()->sound_button->play();
		}
		else
		{
			laserpointer_mat->setParameterSlider(laserpointer_mat->findParameter("emission_scale"), 0);
			laser_light->setEnabled(0);
		}
	}

	if (flash_light)
	{
		if (isOpen)
		{
			flashlight_mat->setParameterSlider(flashlight_mat->findParameter("emission_scale"), 3);
			flash_light->setEnabled(1);
			Resource::get()->sound_button->play();
		}
		else
		{
			flashlight_mat->setParameterSlider(flashlight_mat->findParameter("emission_scale"), 0);
			flash_light->setEnabled(0);
		}
	}
}

void ObjMovable::unfreeze(const BodyPtr &body)
{
	if (!body) return;

	PropertyPtr prop = body->getObject()->getNode()->getProperty();
	if (prop && checkObject(prop) == 1)
		activate_body(body);
}

void ObjMovable::mute_action(const BodyPtr &body)
{
	// unsubscribe
	onContact.removeListener(unfreeze_action_id);
	//onContact.removeListener(play_sound_action_id);
	onFrozen.removeListener(mute_action_id);
}



void ObjMovable::activate_body(const BodyPtr &body)
{
	body->setImmovable(0);
	body->setFrozen(0);
}

void ObjMovable::unfreeze_action(const BodyPtr &body, int num)
{
	if (num >= body->getNumContacts()) return;

	unfreeze(body->getContactBody0(num));
	unfreeze(body->getContactBody1(num));
}




