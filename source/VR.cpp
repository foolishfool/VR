#include "VR.h"
#include <UnigineGame.h>
#include <UnigineEditor.h>
#include <UnigineApp.h>
#include <string> 
#include "Utils.h"
#include "Resource.h"
#define HIDE_BASESTATIONS


using namespace Unigine;
using namespace Math;

float movement_speed_player;
float roll_speed_player = 10.0f;
float null_zone_player = 0.1f;

VR* VR::instance;

VR* VR::get()
{
	return instance;
}

int VR::isPluginLoaded()
{
	if (Engine::get()->findPlugin("AppVive") != -1 )
		return 1;
	return 0;
}

void VR::init()
{
	instance = this;

	elapsed_time_velocity = CHANGE_INTERVAL_VELOCITY;	// current time left to change current scale of our objects

	head = NodeDummy::create();
	//leapmotion_camera is used as the parent of camera of lieap
	leapmotion_camera = Resource::get()->hmd_reference;
	leapmotion_camera->setWorldParent(head->getNode());
	leapmotion_camera->setPosition(Vec3(0, 0, 0));
	leapmotion_camera->setEnabled(1);

	textColor1 = vec4(0.007843, 0.952941, 0.247059, 1.000000);
	//dark green
	textColor2 = vec4(0.000000, 0.309216, 0.089804, 1.000000);

	//the wall will be hidden when too far
	wall_material1 = Resource::get()->wall1->getMaterial(0);
	wall_material2 = Resource::get()->wall2->getMaterial(0);

	initialScale1 = Resource::get()->wall1->getWorldScale();
	initialScale2 = Resource::get()->wall2->getWorldScale();
	// select API
	api = API::None;
	Engine *engine = Engine::get();
	
	if (engine->findPlugin("AppVive") != -1) api = API::Vive;

	if (api == API::None) return;

	// init VIVE
	if (api == API::Vive)
	{
		vive.init();
		controller_tracker[0] = NodeReference::cast(Resource::get()->vive_controller_0);
		//the helicopter should disconnect with the vive_tracker, we don't need to track helicopter as it is already the child of player
		controller_tracker[1] = NodeReference::cast(Resource::get()->vive_controller_1);
		controller_tracker[2] = NodeReference::cast(Resource::get()->vive_controller_2);
		controller_tracker[3] = NodeReference::cast(Resource::get()->vive_tracker_reference);
	
		///as steam vr version problem the second last tracker rotation is tracked wrong, so change that tracker to be the depandent
		//the last two trackers 
		controller_tracker[4] = NodeReference::cast(Resource::get()->vive_tracker_1);
		controller_tracker[5] = NodeReference::cast(Resource::get()->vive_tracker_2);
		//pedant
	//	controller_tracker[6] = NodeReference::cast(Resource::get()->vive_tracker_1);

		controller0_lastposition = controller_tracker[0]->getWorldPosition();
		controller1_lastposition = controller_tracker[1]->getWorldPosition();
		controller2_lastposition = controller_tracker[2]->getWorldPosition();	
	}


	hand_linear_velocity.clear();
	hand_angular_velocity.clear();

	for (int i = 0; i < CONTROLLER_COUNT; i++)
	{
		///cause we need to hide the vive pad for controller 1 and controller 3 
		controller_ref[i] = controller_tracker[i]->getReference();

		controller_obj[i].clear();
		find_obj_in_children(controller_ref[i], &controller_obj[i]);

		Vector<vec3> lv;
		hand_linear_velocity.append(lv);

		Vector<vec3> av;
		hand_angular_velocity.append(av);
	}

	grab_init();

	//create new player
	player = PlayerDummy::create();
	///cannot do that otherwise will be very slow fps
	//Game::get()->getPlayer()->addChild(controller_tracker[0]->getNode());	 
	//Game::get()->getPlayer()->addChild(controller_tracker[1]->getNode());
	//Game::get()->getPlayer()->addChild(controller_tracker[2]->getNode());
	//Game::get()->getPlayer()->addChild(controller_tracker[3]->getNode());
	//Game::get()->getPlayer()->addChild(Resource::get()->vive_tracker_0);
	//Game::get()->getPlayer()->addChild(Resource::get()->vive_tracker_1);

	ui_text1 = ObjectText::cast(LoadFromEditor("hudAGL"));
	ui_text2 = ObjectText::cast(LoadFromEditor("hudDTR"));
	ui_text3 = ObjectText::cast(LoadFromEditor("AGL"));
	ui_text4 = ObjectText::cast(LoadFromEditor("DSR"));

}

void VR::update()
{
	//adjustHandyPosition();

	controller_valid[0] = controller_0_valid;
	controller_valid[1] = controller_1_valid;
	controller_valid[2] = controller_2_valid;
	controller_valid[3] = tracker_0_valid;
	controller_valid[4] = tracker_1_valid;
	controller_valid[5] = tracker_2_valid;
	//controller_valid[6] = tracker_3_valid;

	distance_to_run = sqrtf(pow((Resource::get()->vive_tracker_0->getWorldPosition().x - Resource::get()->HelicopterPad->getWorldPosition().x), 2) + pow((Resource::get()->vive_tracker_0->getWorldPosition().y - Resource::get()->HelicopterPad->getWorldPosition().y), 2));

	//get the current landheight unit: feet
	landHeight = (Resource::get()->vive_tracker_helicopter->getWorldPosition().z - getTerrainHeightAt(Resource::get()->land, Resource::get()->vive_tracker_helicopter->getWorldPosition())) * 3.28084;

	//need minus 4 to ask the bottom of the helicopter to reach the land
	int integer = ((int)(landHeight * 100 + .5) / 100.0) - 4;
	int decimals = ((int)((landHeight - integer) * 10000) / 1000);

	ui_text1->setText((std::to_string(integer)).c_str());

	int integer2 = ((int)(distance_to_run * 100 + 0.5) / 100.0);
	int decimals2 = ((int)((distance_to_run - integer2) * 100000) / 1000);

	ui_text2->setText((std::to_string(integer2) + '.' + std::to_string(decimals2)).c_str());

	
	if (api == API::None) return;

	Mat4 player_transform = Game::get()->getPlayer()->getWorldTransform();

	// update positions of basestations and controllers
	vive_find_devices();

#ifndef HIDE_BASESTATIONS
		if (vive.isDeviceConnected(BASESTATION_DEVICE_0) && BASESTATION_DEVICE_0 != -1) {
			mat4 basestation_0_transform = vive.getDevicePose(BASESTATION_DEVICE_0);
			basestation[0]->setWorldTransform(player_transform * Mat4(basestation_0_transform));
			basestation[0]->setEnabled(1);
		}
		else basestation[0]->setEnabled(0);

		if (vive.isDeviceConnected(BASESTATION_DEVICE_1) && BASESTATION_DEVICE_1 != -1) {
			mat4 basestation_1_transform = vive.getDevicePose(BASESTATION_DEVICE_1);
			basestation[1]->setWorldTransform(player_transform * Mat4(basestation_1_transform));
			basestation[1]->setEnabled(1);
		}
		else basestation[1]->setEnabled(0);
#endif

		
	controller_0_valid = vive.isDeviceConnected(CONTROLLER_DEVICE_0) && CONTROLLER_DEVICE_0 != -1 && vive.isPoseValid(CONTROLLER_DEVICE_0);
	controller_1_valid = vive.isDeviceConnected(CONTROLLER_DEVICE_1) && CONTROLLER_DEVICE_1 != -1 && vive.isPoseValid(CONTROLLER_DEVICE_1);
	controller_2_valid = vive.isDeviceConnected(CONTROLLER_DEVICE_2) && CONTROLLER_DEVICE_2 != -1 && vive.isPoseValid(CONTROLLER_DEVICE_2);
	tracker_0_valid = vive.isDeviceConnected(TRACKER_DEVICE_0) && TRACKER_DEVICE_0 != -1 && vive.isPoseValid(TRACKER_DEVICE_0);
	tracker_1_valid = vive.isDeviceConnected(TRACKER_DEVICE_1) && TRACKER_DEVICE_1 != -1 && vive.isPoseValid(TRACKER_DEVICE_1);
	tracker_2_valid = vive.isDeviceConnected(TRACKER_DEVICE_2) && TRACKER_DEVICE_2 != -1 && vive.isPoseValid(TRACKER_DEVICE_2);
//	tracker_3_valid = vive.isDeviceConnected(TRACKER_DEVICE_3) && TRACKER_DEVICE_3 != -1 && vive.isPoseValid(TRACKER_DEVICE_3);
	
	controller_tracker_update(0, player_transform, controller_0_valid, Mat4(vive.getDevicePose(CONTROLLER_DEVICE_0)));
	controller_tracker_update(1, player_transform, controller_1_valid, Mat4(vive.getDevicePose(CONTROLLER_DEVICE_1)));
	controller_tracker_update(2, player_transform, controller_2_valid, Mat4(vive.getDevicePose(CONTROLLER_DEVICE_2)));
	controller_tracker_update(3, player_transform, tracker_0_valid, Mat4(vive.getDevicePose(TRACKER_DEVICE_0)));
	controller_tracker_update(4, player_transform, tracker_1_valid, Mat4(vive.getDevicePose(TRACKER_DEVICE_1)));
	controller_tracker_update(5, player_transform, tracker_2_valid, Mat4(vive.getDevicePose(TRACKER_DEVICE_2)));
//	controller_tracker_update(6, player_transform, tracker_3_valid, Mat4(vive.getDevicePose(TRACKER_DEVICE_3)));

	//get glove_tracker transform
	glove_tracker_transform_left = Mat4(vive.getDevicePose(TRACKER_DEVICE_2));
	glove_tracker_transform_right = Mat4(vive.getDevicePose(TRACKER_DEVICE_3));

	//only controller needs grabing opertion
	grab_update(0, controller_0_valid, getControllerAxis(0, BUTTON::TRIGGER), getControllerButtonDown(0, BUTTON::TRIGGER));
	//controller 1 is the torch
	//grab_update(1, controller_1_valid, getControllerAxis(1, BUTTON::TRIGGER), getControllerButtonDown(1, BUTTON::TRIGGER));

	quat rot = player->getRotation();

	if (CONTROLLER_DEVICE_0 != -1)
	{
		push_hand_linear_velocity(0, rot * (getworldVelocity(controller_tracker[0], controller0_lastposition)* hand_force_multiplier));
		push_hand_angular_velocity(0, rot * vive.getDeviceAngularVelocity(CONTROLLER_DEVICE_0));
		controller0_lastposition = controller_tracker[0]->getWorldPosition();
	}
	if (CONTROLLER_DEVICE_1 != -1)
	{
		push_hand_linear_velocity(1, rot * (getworldVelocity(controller_tracker[1], controller1_lastposition) *hand_force_multiplier));
		push_hand_angular_velocity(1, rot * vive.getDeviceAngularVelocity(CONTROLLER_DEVICE_1));
		controller1_lastposition = controller_tracker[1]->getWorldPosition();
	}
	if (CONTROLLER_DEVICE_2 != -1)
	{
		push_hand_linear_velocity(2, rot * (getworldVelocity(controller_tracker[2], controller1_lastposition) *hand_force_multiplier));
		push_hand_angular_velocity(2, rot * vive.getDeviceAngularVelocity(CONTROLLER_DEVICE_2));
		controller2_lastposition = controller_tracker[2]->getWorldPosition();
	}

	// update head position info
	hmd_transform = Mat4(vive.getDevicePose(HMD_DEVICE_0));
	hmd_transform_world = player_transform * hmd_transform;

	Vec3 head_offset = Vec3(0, 0, 0);
	if ((api == API::Vive && vive.isDeviceConnected(HMD_DEVICE_0) && HMD_DEVICE_0 != -1))
		head_offset = player_transform.getTranslate() - hmd_transform_world.getTranslate();
	head_offset.z = 0;
	head->setWorldTransform(hmd_transform_world);

	//use Vr controller to controll the winch
	//this method is old version and is stupid 
	//improve in the next step
	Resource::get()->WinchTrigger->setScale(vec3(1, 1, 1));

	if (vive_getControllerDPadPressed(CONTROLLER_DEVICE_1, BUTTON_DPAD_UP) || vive_getControllerDPadPressed(CONTROLLER_DEVICE_0, BUTTON_DPAD_UP))
	{
		Resource::get()->WinchTrigger->setScale(vec3(2, 1, 1));
	}
	else if (vive_getControllerDPadPressed(CONTROLLER_DEVICE_1, BUTTON_DPAD_DOWN) || vive_getControllerDPadPressed(CONTROLLER_DEVICE_0, BUTTON_DPAD_DOWN))
	{
		Resource::get()->WinchTrigger->setScale(vec3(1, 2, 1));
	}

	else if (vive_getControllerDPadPressed(CONTROLLER_DEVICE_1, BUTTON_DPAD_RIGHT) || vive_getControllerDPadPressed(CONTROLLER_DEVICE_0, BUTTON_DPAD_RIGHT))
	{

		Resource::get()->WinchTrigger->setScale(vec3(1, 1, 2));
	}
	else if (vive_getControllerDPadPressed(CONTROLLER_DEVICE_1, BUTTON_DPAD_LEFT) || vive_getControllerDPadPressed(CONTROLLER_DEVICE_0, BUTTON_DPAD_LEFT))
	{
		Resource::get()->WinchTrigger->setScale(vec3(1, 3, 1));
	}
	//create new rope
	else if (vive.getControllerButtonPressed(CONTROLLER_DEVICE_0, BUTTON_APPLICATIONMENU) == 1 && Resource::get()->WinchTrigger->getScale().x <2.8f)
	{
		Resource::get()->WinchTrigger->setScale(vec3(3, 1, 1));
	}
	
//create new items only press one button
//	else if ((vive.getControllerButtonPressed(CONTROLLER_DEVICE_1, BUTTON_APPLICATIONMENU) == 1 && !vive.getControllerButtonPressed(CONTROLLER_DEVICE_1, BUTTON_STEAMVR_TRIGGER) == 1)||
//			(vive.getControllerButtonPressed(CONTROLLER_DEVICE_0, BUTTON_APPLICATIONMENU) == 1 && !vive.getControllerButtonPressed(CONTROLLER_DEVICE_0, BUTTON_STEAMVR_TRIGGER) == 1))
//		Resource::get()->WinchTrigger->setScale(vec3(3, 1, 1));
//	//CONTROLLER_DEVICE_2 use laser pointer
//	else if (vive.getControllerButtonPressed(CONTROLLER_DEVICE_1, BUTTON_APPLICATIONMENU) == 1 && vive.getControllerButtonPressed(CONTROLLER_DEVICE_1, BUTTON_STEAMVR_TRIGGER) == 1)
//	{
//	//	Log::message("%d %d 111\n", vive.getControllerButtonPressed(CONTROLLER_DEVICE_2, BUTTON_APPLICATIONMENU), vive.getControllerButtonPressed(CONTROLLER_DEVICE_2, BUTTON_STEAMVR_TRIGGER));
//		Resource::get()->WinchTrigger->setScale(vec3(4, 1, 1));
//	}	
//	//CONTROLLER_DEVICE_0 use laser pointer
//	else if (vive.getControllerButtonPressed(CONTROLLER_DEVICE_0, BUTTON_APPLICATIONMENU) == 1  && vive.getControllerButtonPressed(CONTROLLER_DEVICE_0, BUTTON_STEAMVR_TRIGGER) == 1)
//		Resource::get()->WinchTrigger->setScale(vec3(1, 4, 1));
	else
		Resource::get()->WinchTrigger->setScale(vec3(1, 1, 1));
	
	if (clickedTrigger(getControllerAxis(1, BUTTON::TRIGGER)) == 1.0f)
	{
		useTorch = true;
	}
	else
	{
		useTorch = false;
	}

	if (getControllerAxis(2, BUTTON::TRIGGER) == 1 || getControllerAxis(2, BUTTON::GRIP) == 1)
	{
		fire = true;
	}
	else
	{
		fire = false;
	}

	//hide or show wall
	showWall();
}
//how does the wall show
void VR::showWall() {
	 
	float distance1 = getDistancetoWall1();

	if (distance1 < 0.3f)
	{
		wall_material1->setParameterSlider(wall_material1->findParameter("transparent_pow"), 0.5f);
		Resource::get()->wall1->setEnabled(1);
	}
	else
	{
		Resource::get()->wall1->setEnabled(0);
		wall_material1->setParameterSlider(wall_material1->findParameter("transparent_pow"), 10.0f);
	}
}

float VR::getDistancetoWall1()
{
	float a = Resource::get()->wall1_point2->getWorldPosition().y - Resource::get()->wall1_point1->getWorldPosition().y;
	float b = Resource::get()->wall1_point1->getWorldPosition().x - Resource::get()->wall1_point2->getWorldPosition().x;
	float c = Resource::get()->wall1_point2->getWorldPosition().x *  Resource::get()->wall1_point1->getWorldPosition().y - Resource::get()->wall1_point1->getWorldPosition().x *  Resource::get()->wall1_point2->getWorldPosition().y;

	float distance1 = fabsf(head->getWorldPosition().x * a + head->getWorldPosition().y * b + c) / fsqrt(a*a + b*b);
	return distance1;
}

float VR::getDistancetoWall2()
{
	float a = Resource::get()->wall2_point2->getWorldPosition().y - Resource::get()->wall2_point1->getWorldPosition().y;
	float b = Resource::get()->wall2_point1->getWorldPosition().x - Resource::get()->wall2_point2->getWorldPosition().x;
	float c = Resource::get()->wall2_point2->getWorldPosition().x *  Resource::get()->wall2_point1->getWorldPosition().y - Resource::get()->wall2_point1->getWorldPosition().x *  Resource::get()->wall2_point2->getWorldPosition().y;

	float distance2= (head->getWorldPosition().x * a + head->getWorldPosition().y* b + c) / fsqrt(a*a + b*b);
	return distance2;
}

int VR::clickedTrigger( float trigger_value,  float pressed_pos)
{
	int button_pressed = 0;
	button_pressed = trigger_value > pressed_pos;
	return button_pressed;
}

//update the status of controllers 
void VR::grab_update(int num, int pose_valid, float trigger_value, int button_touch, float pressed_pos, float release_pos)
{
	// calculate button_pressed (user experience improvement)
	int button_pressed = 0;

	if (hand_state[num] == HAND_FREE)
	{
		throw_trigger_value[num] = min(throw_trigger_value[num], trigger_value);
		button_pressed = trigger_value > pressed_pos + throw_trigger_value[num]; // around +30% from minimum pressure
	}
	else
	{
		throw_trigger_value[num] = max(throw_trigger_value[num], trigger_value);
		button_pressed = trigger_value > release_pos * throw_trigger_value[num]; // around 80% from maximum pressure

	}

	// you can't grab anything if controller doesn't have valid pose
	if (!pose_valid)
	{
		set_outline(num, 0);
		return;
	}

	// hide outline
	if (last_selected_timer[num] >= 0)
	{
		last_selected_timer[num] -= Game::get()->getIFps() / Game::get()->getScale();
		if (last_selected_timer[num] < 0)
			set_outline(num, 0);
	}
	//pointer_update(num, node_grabbed[num] == 0);

	if (node_grabbed[num] == 0)
	{
		hand_state[num] = HAND_FREE;
		hitObj = last_selected[num];
		// get intersections, find new hitObj
		if (last_selected_timer[num] < 0)
		{
			if (grab_mode == GRAB_MODE::BOUNDBOX)
			{

				hitObj = ObjectPtr(); // null pointer

				intersections.clear();

				if (World::get()->getIntersection(controller_tracker[num]->getWorldBoundBox(), intersections))
				{

					// find nearest interactive object (can i grab it?)
					int nearest = -1;
					Scalar min_dist = UNIGINE_INFINITY;
					for (int k = 0; k < intersections.size(); k++)
					{
						if (can_i_grab_it(intersections[k]->getNode()))
						{
							Scalar dist = length2(intersections[k]->getWorldPosition() - controller_tracker[num]->getWorldPosition());
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
						hitObj = intersections[nearest];

					}
				}
			}
			else if (grab_mode == GRAB_MODE::INTERSECTIONS)
			{
				Vec3 pos1 = controller_tracker[num]->getPosition() - Vec3(getWorldDown(controller_tracker[num]->getNode()))* ray_back_length;
				Vec3 pos2 = controller_tracker[num]->getPosition() + Vec3(getWorldDown(controller_tracker[num]->getNode())) * ray_forward_length;

				hitObj = World::get()->getIntersection(pos1, pos2, 1, intersection);
				if (!hitObj) // second intersection (for best player's experience)
					hitObj = World::get()->getIntersection(pos1, pos2 + Vec3(getWorldLeft(controller_tracker[num]->getNode())) * ptr_width * 0.5f, 1, intersection);
				if (!hitObj) // third intersection (for best player's experience)
					hitObj = World::get()->getIntersection(pos1, pos2 + Vec3(getWorldRight(controller_tracker[num]->getNode())) * ptr_width * 0.5f, 1, intersection);
			}
		}

		if (hitObj)
		{
				
			// check if this object grabbed by another controller
			for (int z = 0; z < grab_node.size(); z++)
				if (z != num && node_grabbed[z] && hitObj->getNode() == grab_node[z])
					return;

			// all right. It's movable or switcher?
			if (can_i_grab_it(hitObj->getNode()))
			{
			//	vibrateController(num, 1);
				// show controller's outline
				last_selected[num] = hitObj;
				if (last_selected_timer[num] < 0)
					last_selected_timer[num] = 0.1f;
				set_outline(num, 1);

				// ... and grab it (or touch)
				if (button_pressed )
				{
					last_selected_timer[num] = 0;
					grab_node[num] = hitObj->getNode();
					node_grabbed[num] = 1;
					hand_state[num] = HAND_GRAB;
					isHolding[num] = true;
				}
			}
		}
	}
	else
	{

		if (isHolding[num] && button_pressed)
		{
			// hold
			hand_state[num] = HAND_HOLD;
		}
		else  if (isHolding[num] && button_pressed == 0)
		{
			//throw!
			node_grabbed[num] = 0;
			hand_state[num] = HAND_THROW;
			isHolding[num] = false;
		}

	}


}

//set the material of the controller and grabbed node
void VR::set_outline(int num, int enable)
{
	if (node_selected[num] == enable)
		return;
	node_selected[num] = enable;
	// set material state (to controller)
	for (int i = 0; i < controller_obj[num].size(); i++)
		for (int k = 0; k < controller_obj[num][i]->getNumSurfaces(); k++)
		{
			controller_obj[num][i]->setMaterialState("auxiliary", enable, k);
		}
	// set material state (to object)
	if (last_selected[num])
		for (int i = 0; i < last_selected[num]->getNumSurfaces(); i++)
			last_selected[num]->setMaterialState("auxiliary", enable, i);
}

//if it has grable
int VR::can_i_grab_it(const NodePtr &node)
{
	PropertyPtr prop = node->getProperty();
	if (prop)
	{
		int index = prop->findParameter("grable");
		if (index != -1)
			return 1;
	}
	return 0;
}

void VR::vibrateController(int controller_num, float amplitude)
{		
	if (amplitude > 0)
	{
		if (controller_num == 0) vive.setControllerVibration(CONTROLLER_DEVICE_0, 100);
		if (controller_num == 1) vive.setControllerVibration(CONTROLLER_DEVICE_1, 100);
	}	
}

float VR::getControllerAxis(int controller_num, int button)
{	
	if (!controller_valid[controller_num])
		return 0;
	int device;

	if (controller_num == 0)
	{
		device = CONTROLLER_DEVICE_0;
	}
	else	if (controller_num == 1)
	{
		device = CONTROLLER_DEVICE_1;
	}
	else if (controller_num == 2)
	{
		device = CONTROLLER_DEVICE_2;	
	}

	switch (button)
	{
	case STICK_X: return vive.getControllerAxis(device, 0).x;
	case STICK_Y: return vive.getControllerAxis(device, 0).y;
	case TRIGGER: return vive.getControllerAxis(device, 1).x;
	case GRIP: return itof(vive.getControllerButtonPressed(device, BUTTON_GRIP));
	case XA: return itof(vive_getControllerDPadPressed(device, BUTTON_DPAD_DOWN));
	case YB: return itof(vive_getControllerDPadPressed(device, BUTTON_DPAD_UP));
	case MENU: return itof(vive.getControllerButtonPressed(device, BUTTON_APPLICATIONMENU));
	}

	return 0;
}

int VR::getControllerButton(int controller_num, int button)
{
	if (!controller_valid[controller_num])
		return 0;
	int device;
	if (controller_num == 0)
	{
		device = CONTROLLER_DEVICE_0;
	}
	else if (controller_num == 1)
	{
		device = CONTROLLER_DEVICE_1;
	}
	else if (controller_num == 2)
	{
		device = CONTROLLER_DEVICE_2;
	}

	switch (button)
	{
	case STICK_X: return Math::abs(vive.getControllerAxis(device, 0).x) > 0.5f;
	case STICK_Y: return Math::abs(vive.getControllerAxis(device, 0).y) > 0.5f;
	case TRIGGER: return vive.getControllerAxis(device, 1).x > 0.5f;
	case GRIP: return vive.getControllerButtonPressed(device, BUTTON_GRIP);
	case XA: return vive_getControllerDPadPressed(device, BUTTON_DPAD_DOWN);
	case YB: return vive_getControllerDPadPressed(device, BUTTON_DPAD_UP);
	case MENU: return vive.getControllerButtonPressed(device, BUTTON_APPLICATIONMENU);
	}

	return 0;
}

NodePtr VR::getHandNode(int num)
{
	return controller_tracker[num]->getNode();
}

int VR::getNumHands()
{
	return CONTROLLER_COUNT;
}

void VR::find_obj_in_children(const NodePtr &node, Vector<ObjectPtr> *obj)
{
	if (!node) return;

	ObjectPtr o = Object::cast(node);
	if (o) obj->append(o);

	for (int i = 0; i < node->getNumChildren(); i++)
	{
		NodeReferencePtr nodeRef = NodeReference::cast(node->getChild(i));
		if (nodeRef)
			find_obj_in_children(nodeRef->getReference(), obj);
		else
			find_obj_in_children(node->getChild(i), obj);
	}
}

void VR::update_button_states()
{
	for (int k = 0; k < CONTROLLER_COUNT; k++)
		for (int i = 0; i < VR::BUTTON::COUNT; i++)
			buttons_prev_state[k][i] = getControllerButton(k, i);
}

int VR::getHandDegreesOfFreedom(int num)
{
	return 6; // x, y, z + pitch, yaw, roll
}

vec3 VR::getHandLinearVelocity(int num)
{
	return linearRegression(hand_linear_velocity[num]);
}

vec3 VR::getHandAngularVelocity(int num)
{
	return linearRegression(hand_angular_velocity[num]);
}

void VR::shutdown()
{

}

int VR::getHandState(int num)
{
	return hand_state[num];
}

NodePtr VR::getGrabNode(int num)
{
	return grab_node[num];
}

int VR::getControllerTriggerPressed(int num)
{
	if (api == API::Vive)
	{
		int device;
		if (num == 0)
		{
			device = CONTROLLER_DEVICE_0;
		}
		else if (num == 1)
			{
				device = CONTROLLER_DEVICE_1;
			}
		else if (num == 2)
		{
			device = CONTROLLER_DEVICE_2;
		}
		
		return
			(vive.isDeviceConnected(device) && device != -1) &&
			(vive.getControllerAxis(device, 1).x > 0.5f);
	}

}

int VR::getControllerMenuPressed()
{
	if (api == API::Vive)
	{
		return
			(vive.isDeviceConnected(CONTROLLER_DEVICE_0) && CONTROLLER_DEVICE_0 != -1 && vive.getControllerButtonPressed(CONTROLLER_DEVICE_0, BUTTON_APPLICATIONMENU)) ||
			(vive.isDeviceConnected(CONTROLLER_DEVICE_1) && CONTROLLER_DEVICE_1 != -1 && vive.getControllerButtonPressed(CONTROLLER_DEVICE_1, BUTTON_APPLICATIONMENU)) ||
			(vive.isDeviceConnected(CONTROLLER_DEVICE_2) && CONTROLLER_DEVICE_2 != -1 && vive.getControllerButtonPressed(CONTROLLER_DEVICE_2, BUTTON_APPLICATIONMENU));
	}
}


void VR::controller_tracker_update(
	int num,
	const Mat4 &player_transform,
	int is_device_connected,
	const Mat4 &device_pose)
{
	if (is_device_connected)
	{
		controller_tracker[num]->setWorldTransform(player_transform * device_pose);
	}
}

//old version 
/*
void VR::controller_update(int num, const Mat4 &player_transform, int device_id)
{
	if (device_id != -1 && vive.isDeviceConnected(device_id))
	{
		mat4 transform = vive.getDevicePose(device_id);
		controller[num]->setWorldTransform(player_transform * Mat4(transform));

		// update buttons
		float trigger_position = vive.getControllerAxis(device_id, 1).x;
		set_controller_trigger_position(controller[num], trigger_position);
		///disable otherwise the fps will be very slow, still don't konw the reason
		if (controller[num]->isEnabled())
			controller_ref[num]->setEnabled(1);
	}
	else if (controller[num]->isEnabled())
		controller_ref[num]->setEnabled(0);
}
*/


//adjust the relative positon when object is holding in hand
void VR::adjustHandyPosition()
{
	float ratio = 0.01;

	if (App::get()->getKeyState(119) == 1)
	{
		handyPosX += Game::get()->getIFps()*ratio;
	}
	if (App::get()->getKeyState(97) == 1)
	{
		handyPosY += Game::get()->getIFps()*ratio;
	}
	if (App::get()->getKeyState(113) == 1)
	{
		handyPosZ += Game::get()->getIFps()*ratio;
	}
	if (App::get()->getKeyState(115) == 1)
	{
		handyPosX -= Game::get()->getIFps()*ratio;
	}
	if (App::get()->getKeyState(100) == 1)
	{
		handyPosY -= Game::get()->getIFps()*ratio;
	}
	if (App::get()->getKeyState(101) == 1)
	{
		handyPosZ -= Game::get()->getIFps()*ratio;
	}

	//	Log::message("%d %d %d  \n", int(x_left_handbone_rotation)%360, int(y_left_handbone_rotation) % 360, int(z_left_handbone_rotation) % 360);
	Log::message(" %f %f %f \n ", handyPosX, handyPosY, handyPosZ);
}


void VR::vive_find_devices()
{
	int curTracking = 0;
	int curDevice = 0;
	for (int i = 0; i < MAX_TRACKED_DEVICE_COUNT; i++)
	{
		int deviceType = vive.getDeviceType(i);
		String num = vive.getDeviceSerialNumber(i);

		switch (deviceType)
		{
		case TRACKED_DEVICE_TRACKING:
			if (curTracking == 0)
				BASESTATION_DEVICE_0 = i;
			else
				BASESTATION_DEVICE_1 = i;
			curTracking++;
			break;
		case TRACKED_DEVICE_CONTROLLER:
			if (curDevice == 0)
			{
				CONTROLLER_DEVICE_0 = i;
			} 
			else if (curDevice == 1)
			{
				CONTROLLER_DEVICE_1 = i;
			}
			else if (curDevice == 2)
				CONTROLLER_DEVICE_2 = i;
			curDevice++;
			break;
		case TRACKED_DEVICE_TRACKER:
		
	//	if (std::string(num) == "LHR-46782BCE")
	//	{
	//		if (curDevice == 3)
	//		{
	//			//only traver 1 is the tracker_device[0]
	//			TRACKER_DEVICE_0 = i;
	//		}
	//	}
	//	else
		{
				if (curDevice == 3)
				{
					TRACKER_DEVICE_0 = i;
				} else

				if (curDevice == 4)
				{
					TRACKER_DEVICE_1 = i;
				} else
				if (curDevice == 5)
				{
					TRACKER_DEVICE_2 = i;
				} else 
				if (curDevice == 6)
				{
					TRACKER_DEVICE_3 = i;
				}
			}		
			curDevice++;
			break;
		case TRACKED_DEVICE_HMD:
			HMD_DEVICE_0 = i;
			break;
		default:
			break;
		}
	}
}

void VR::vive_set_controller_trigger_position(const NodeReferencePtr &controller, float position)
{
	NodePtr root = controller->getReference();
	int pivot_id = root->findChild("vive_trigger_pivot");

	if (pivot_id != -1) {
		NodePtr pivot_node = root->getChild(pivot_id);
		pivot_node->setTransform(Mat4(translate(0.0f, -0.039f, -0.018f) * rotateX(-position * 20.0f)));
	}
}

int VR::vive_getControllerDPadPressed(int device, int button)
{
	// You might expect that pressing one of the edges of the SteamVR controller touchpad could
	// be detected with a call to k_EButton_DPad_* (BUTTON_DPAD_*), but currently this always returns false.
	// Not sure whether this is SteamVR's design intent, not yet implemented, or a bug.
	// The expected behaviour can be achieved by detecting overall Touchpad press, with Touch-Axis comparison to an edge threshold.

	if (vive.getControllerButtonPressed(device, BUTTON_STEAMVR_TOUCHPAD))
	{
		vec2 axis = vive.getControllerAxis(device, 0);
		if ((axis.y > 0.3f && button == BUTTON_DPAD_UP) ||
			(axis.y < -0.3f && button == BUTTON_DPAD_DOWN) ||
			(axis.x > 0.6f && button == BUTTON_DPAD_RIGHT) ||
			(axis.x < -0.6f && button == BUTTON_DPAD_LEFT))
			return 1;
	}

	return 0;
}

void VR::grab_init()
{
	hand_state.clear();
	node_grabbed.clear();
	node_selected.clear();
	last_selected.clear();
	last_selected_timer.clear();
	grab_node.clear();
	throw_trigger_value.clear();

	NodePtr node_null;
	ObjectPtr obj_null;
	for (int i = 0; i < CONTROLLER_COUNT; i++)
	{
		grab_node.append(node_null);
		hand_state.append(0);
		node_grabbed.append(0);
		last_selected.append(obj_null);
		last_selected_timer.append(0);
		node_selected.append(0);
		throw_trigger_value.append(0);
		isHolding.append(0);
	}

	//pointer_init();
}

//get speed of controller
void VR::push_hand_linear_velocity(int num, const vec3 &velocity)
{
	Vector<vec3> *buffer = &hand_linear_velocity[num];

	if (buffer->size() < CONTROLLER_BUFFER_COUNT)
		buffer->append(velocity);
	else
	{
		for (int i = 0; i < buffer->size() - 1; i++)
			buffer->get(i) = buffer->get(i + 1);

		buffer->get(buffer->size() - 1) = velocity;
	}

}

void VR::push_hand_angular_velocity(int num, const vec3 &velocity)
{
	Vector<vec3> *buffer = &hand_angular_velocity[num];

	if (buffer->size() < CONTROLLER_BUFFER_COUNT)
		buffer->append(velocity);
	else
	{
		for (int i = 0; i < buffer->size() - 1; i++)
			buffer->get(i) = buffer->get(i + 1);

		buffer->get(buffer->size() - 1) = velocity;
	}
}

//get current vive tracker position to used for helicopter
void  VR::loadposition()
{
	Resource::get()->vive_tracker_0->setRotation(controller_tracker[3]->getRotation());
	Resource::get()->vive_tracker_0->setPosition(controller_tracker[3]->getPosition());
}
//get controller speed
vec3 VR::getworldVelocity(NodeReferencePtr node, Vec3 lastPosition)
{
	if (elapsed_time_velocity >= 0)
	{
		elapsed_time_velocity -= Game::get()->getIFps();
	//	Log::message("%f   \n",  elapsed_time_velocity);
	}
	if (elapsed_time_velocity <0)
	{
		Vec3 currentposition = node->getWorldPosition();

		float x = currentposition.x - lastPosition.x;
		float y = currentposition.y - lastPosition.y;
		float z = currentposition.z - lastPosition.z;
		lastPosition = currentposition;
		// resetting elapsed time counter
		elapsed_time_velocity = CHANGE_INTERVAL_VELOCITY;
		//0.1 second change to 1s
		return vec3(x * 10, y * 10, z * 10);
	}
	else
		return vec3(0, 0, 0);
}

