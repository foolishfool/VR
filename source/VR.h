#pragma once

#include <UnigineNode.h>
#include <UnigineNodes.h>
#include <UnigineWorld.h>
#include "ViveAPI.h"
#include <UnigineGame.h>
#include <UnigineMap.h>
#include "Utils.h"


#define CONTROLLER_COUNT 6
#define CONTROLLER_BUFFER_COUNT 6

using namespace Unigine;
using namespace Math;



class VR 
{
public:
	static VR* instance;
	static VR* get();

	friend class Resource;
	bool attachWeight;
	bool detachWeight;

	bool isNVG;
	bool isBatteryFailure;

	//	Handle* handle = nullptr;
	CallbackBase* use_callback;
	CallbackBase* grab_callback;
	CallbackBase* hold_callback;
	CallbackBase* throw_callback;

	NodeReferencePtr controller_tracker[7];
	// is -extern_plugin "AppVive/AppOculus" added to command line and successfully loaded?
	static int isPluginLoaded();

	void grab_init();
	
	void init();
	void update();
	//void render();
	void shutdown();
	void adjustHandyPosition();

	UNIGINE_INLINE int isViveLoaded() { return api == API::Vive; }
	//UNIGINE_INLINE int isOculusLoaded() { return api == API::Oculus; }

	// move player to position (and then land him to ground) with some direction

	UNIGINE_INLINE NodePtr getHead() { return head->getNode(); }
	UNIGINE_INLINE PlayerPtr getPlayer() { return player->getPlayer(); }
	UNIGINE_INLINE Mat4 getHeadTransform() { return hmd_transform_world; }
	UNIGINE_INLINE Vec3 getControllerPosition(int num) { return controller_tracker[num]->getWorldPosition(); }
	UNIGINE_INLINE WorldBoundBox getControllerBoundBox(int num) { return controller_tracker[num]->getWorldBoundBox(); }
	int getControllerTriggerPressed(int num);
	int getControllerMenuPressed();
	PlayerDummyPtr player;
	UNIGINE_INLINE PlayerDummyPtr getPlayerRef() { return player; }

	UNIGINE_INLINE virtual vec3 getHandyPos() { return vec3(0, -0.024f, 0.067f); }
	///280 initial value is 10 the reason +270 because in the world file the controller is rotated 270 degree
	UNIGINE_INLINE virtual quat getHandyRot() { return quat(190.0f, 0, 0); }
	
	//used for relative position offset for smoke handle in glove
	float handyPosX = 0.017974;
	float handyPosY = 0.003534;
	float handyPosZ = 0.037561;

	vec3 HandyPos = vec3(handyPosX, handyPosY, handyPosZ);

	int RotateNode(Unigine::NodePtr node, Unigine::Math::quat rotationDirection)
	{
		// getting current node transformation matrix
		Unigine::Math::Mat4 transform = node->getTransform();

		// calculating delta rotation around an arbitrary axis
		Unigine::Math::quat delta_rotation = Unigine::Math::quat(rotationDirection);

		// setting node's scale, rotation and position

		node->setRotation(node->getRotation() * delta_rotation);
		//node->setWorldPosition(node->getWorldPosition() + forward_direction * MOVING_SPEED *ifps);

		return 1;
	}

	//int AppWorldLogic::TranslateNode(NodePtr node, Vec3 transformDirection);

	int TranslateNode(NodePtr node, Vec3 translateDirection)
	{
		// getting current node transformation matrix
		Math::Mat4 transform = node->getTransform();
		node->setWorldPosition(node->getWorldPosition() + (-transform.getAxisY() * (translateDirection.x * 50) * Game::get()->getIFps()));
		return 1;
	}


	void vibrateController(int controller_num, float amplitude = 1.0f);

	enum {
		HAND_FREE,
		HAND_GRAB,
		HAND_HOLD,
		HAND_THROW
	};

	enum GRAB_MODE
	{
		BOUNDBOX,
		INTERSECTIONS
	};


	GRAB_MODE grab_mode = GRAB_MODE::BOUNDBOX;

	// baseline controls for VR Controllers (Vive, Oculus, XBox):
	// http://metanautvr.com/wp-content/uploads/2017/07/VRControllersBaselineComparison2C.png

	enum BUTTON
	{
		STICK_X,
		STICK_Y,
		TRIGGER,
		GRIP,
		XA, // X or A
		YB, // Y or B
		MENU,

		COUNT, // reserved. buttons count
	};
	// the object current hit that need to hold
	ObjectPtr hitObj;

	// controllers buttons
	int buttons_prev_state[CONTROLLER_COUNT][BUTTON::COUNT];

	UNIGINE_INLINE int getControllerButtonDown(int controller_num, int button) { return getControllerButton(controller_num, button) && !buttons_prev_state[controller_num][button]; }
	UNIGINE_INLINE int getControllerButtonUp(int controller_num, int button) { return !getControllerButton(controller_num, button) && buttons_prev_state[controller_num][button]; }

	// hands
	void setGrabMode(GRAB_MODE mode) { grab_mode = mode; }; // grab via BoundBox-BoundBox or Line-Surface intersection?
	int getNumHands() ;						// get count of hands
	NodePtr getHandNode(int num) ;			// get hand's node
	int getHandDegreesOfFreedom(int num) ;	// get hand's degrees of freedom
	int getHandState(int num) ;				// get hand state (grab, hold, throw)
	NodePtr getGrabNode(int num) ;
	vec3 getHandLinearVelocity(int num) ;	// get speed of hand
	vec3 getHandAngularVelocity(int num) ;	// get angular speed of hand
	
	void update_button_states();

	float getControllerAxis(int controller_num, int button) ;
	int getControllerButton(int controller_num, int button) ;

	ObjectTextPtr ui_text1;
	ObjectTextPtr ui_text2;
	ObjectTextPtr ui_text3;
	ObjectTextPtr ui_text4;

	void showWall();

	void loadposition();

	bool useTorch;
	bool fire;

	Mat4 glove_tracker_transform_left;
	Mat4 glove_tracker_transform_right;

	/*
	VIVE
	*/
	ViveAPI vive;
	//used to get leapmotion camera position 
	NodePtr leapmotion_camera;

private:
	//local transform of hmd
	Mat4 hmd_transform;
	
	//used to get the formula of the line of wall

	float getDistancetoWall1();
	float getDistancetoWall2();

	vec3 getworldVelocity(NodeReferencePtr node, Vec3 lastPosition);

	// grabbing

	Mat4 initial_transform_vive_tracker_demo;
	NodeDummyPtr head;

	

	Vec3 controller0_lastposition;
	Vec3 controller1_lastposition;
	Vec3 controller2_lastposition;
	// controller velocities
	Vector<Vector<vec3>> hand_linear_velocity;
	Vector<Vector<vec3>> hand_angular_velocity;

	NodePtr controller_ref[CONTROLLER_COUNT];
	Vector<ObjectPtr> controller_obj[CONTROLLER_COUNT];

	Vector<int> hand_state;		// status of hand: free, grab, hold, throw, etc.
	Vector<NodePtr> grab_node;	// what is it?
	Vector<int> node_grabbed;	// hand grab something?
	Vector<int> node_selected;	// hand around grabable node?
	Vector<ObjectPtr> last_selected;
	Vector<float> last_selected_timer;
	Vector<float> throw_trigger_value;	// for best experience (grab when 20%, throw when 80%)
	float ray_back_length = 0.03f;		// ray's length of controllers from pivot
	float ray_forward_length = 0.05f;

	Vector<ObjectPtr> intersections;


	int can_i_grab_it(const NodePtr &node);


	// pointer (part of grabbing)
	float ptr_width = 0.0125f;
	//use to detect the release status of contorller
	Vector<bool> isHolding;

	//trigger value: the extent value to trigger the grab when holding the grab buttons
	void grab_update(int num, int pose_valid, float trigger_value, int button_touch, float pressed_pos = 0.3f, float release_pos = 0.8f);
	int clickedTrigger( float trigger_value, float pressed_pos = 0.3f);
	void set_outline(int num, int enable);

	void find_obj_in_children(const NodePtr &node, Vector<ObjectPtr> *obj);
	
	// moving

	int controller_valid[CONTROLLER_COUNT];

	int controller_0_valid = 0;
	int controller_1_valid = 0;
	int controller_2_valid = 0;
	int tracker_0_valid = 0;
	int tracker_1_valid = 0;
	int tracker_2_valid = 0;
	//int tracker_3_valid = 0;

	float hand_force_multiplier = 5.0f;	// getHandLinearVelocity = velocity * hand_force_multiplier

	const float CHANGE_INTERVAL_VELOCITY = 0.01f ; // the interval between changes of position, in seconds
	float elapsed_time_velocity ;	// current time left to change current scale of our objects
	float landHeight;
	float distance_to_run;

	MaterialPtr wall_material1;
	MaterialPtr wall_material2;

	//the node to represent the HMD
	ObjectDummyPtr hmd;

	vec4 textColor1;
	vec4 textColor2;

	// the initial sacle of wall
	vec3 initialScale1;
	vec3 initialScale2;

	/*
	API
	*/
	enum API { None, Vive, Oculus };
	API api;

	/*
	COMMON
	*/
	Mat4 hmd_transform_world;

	WorldIntersectionPtr intersection = WorldIntersection::create();

	// - methods
	void controller_tracker_update(int num, const Mat4 &player_transform, int is_device_connected, const Mat4 &device_pose);
	//void controller_update(int num, const Mat4 &player_transform, int device_id);



	NodeReferencePtr basestation[2];

	int BASESTATION_DEVICE_0 = -1,
		BASESTATION_DEVICE_1 = -1,
		CONTROLLER_DEVICE_0 = -1,
		CONTROLLER_DEVICE_1 = -1,
		CONTROLLER_DEVICE_2 = -1,
		TRACKER_DEVICE_0 = -1,
		TRACKER_DEVICE_1 = -1,
		TRACKER_DEVICE_2 = -1,
		TRACKER_DEVICE_3 = -1,
		HMD_DEVICE_0 = -1;

	void vive_find_devices();
	void vive_set_controller_trigger_position(const NodeReferencePtr &controller, float position);
	int vive_getControllerDPadPressed(int device, int button);

	void push_hand_linear_velocity(int num, const vec3 &velocity);
	void push_hand_angular_velocity(int num, const vec3 &velocity);
};
