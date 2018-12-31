#pragma once
#include <UnigineLogic.h>
#include <UnigineVisualizer.h>
#include <UnigineWidgets.h>
#include <UnigineViewport.h>
#include <UniginePlayers.h>
#include <UnigineGame.h>
#include <UnigineEditor.h>
#include <UnigineApp.h>
#include <UnigineConsole.h>
#include <UniginePrimitives.h>
#include <plugins\LeapMotionInterface.h>
#include "Utils.h"
#include "VR.h"
#include "Resource.h"


using namespace Unigine;
using namespace Math;

class LeapMotion
{
public:

	void init();
	void update();

	static LeapMotion* instance;
	static LeapMotion* get();

	int getNumHands();
	int getHandState(int num);
	vec3 getHandLinearVelocity(int num);
	NodePtr getHandNode(int num);
	int getHandDegreesOfFreedom(int num);
	vec3 getHandAngularVelocity(int num);
	NodePtr getGrabNode(int num);

	UNIGINE_INLINE virtual vec3 getHandyPos() { return vec3(0, 0, 0); }
	///280 initial value is 10 the reason +270 because in the world file the controller is rotated 270 degree
	UNIGINE_INLINE virtual quat getHandyRot() { return quat(-180, 0, 0); }

	void push_hand_linear_velocity(int num, const vec3 &velocity);
	vec3 getworldVelocity(NodePtr node, Vec3 lastPosition);
	//used to calculate the speed of hands
	Vec3 lefthand_lastposition;
	Vec3 righthand_lastposition;

	float hand_force_multiplier = 5.0f;	// getHandLinearVelocity = velocity * hand_force_multiplier

	float hand_offset_x = 0;
	float hand_offset_y = 0;
	float hand_offset_z = 0;

	ObjectMeshDynamicPtr palm_bone_left;
	ObjectMeshDynamicPtr palm_bone_right;

private:


	//used for rotation adjustment for hand_mesh
	float x_left_mesh_rotation = -4.802559;
	float y_left_mesh_rotation = 1.769511;
	float z_left_mesh_rotation = 68.608437;


	LeapMotionInterface *lm;
	Visualizer *visualizer;

	GuiPtr gui;

	PlayerDummyPtr leap_motion_camera;

	NodePtr eye = VR::get()->leapmotion_camera;

	WidgetSpritePtr widget_leap_img;
	WidgetSpriteViewportPtr widget_hands;

	void create_widget_leap_img();
	void create_widget_hands();
	void draw_leap_motion_hand(const Unigine::LeapMotionInterface::Hand &hand);
	Vec3 leap_pos_to_world_pos(const Unigine::Math::vec3 &pos);
	void draw_finger_joints(int hand_type, int finger_type, int finger_num, Vec3 pos,vec4 hand_color);
	void draw_bones();
	void create_joints();
	void create_bones();
	void hide_joints_left(int hide);
	void hide_joints_right(int hide);


	void adjustPlamPosition();

	bool isShowLeftHand;
	bool isShowRightHand;


	ObjectMeshDynamicPtr wrist_joint_left;
	ObjectMeshDynamicPtr wrist_joint_right;
	ObjectMeshDynamicPtr arm_joint_left;
	ObjectMeshDynamicPtr arm_joint_right;


	ObjectMeshDynamicPtr finger_joint_thumb_left[LeapMotionInterface::Bone::BONE_NUM_TYPES + 1];
	ObjectMeshDynamicPtr finger_joint_thumb_right[LeapMotionInterface::Bone::BONE_NUM_TYPES + 1];
	ObjectMeshDynamicPtr finger_joint_index_left[LeapMotionInterface::Bone::BONE_NUM_TYPES + 1];
	ObjectMeshDynamicPtr finger_joint_index_right[LeapMotionInterface::Bone::BONE_NUM_TYPES + 1];
	ObjectMeshDynamicPtr finger_joint_middle_left[LeapMotionInterface::Bone::BONE_NUM_TYPES + 1];
	ObjectMeshDynamicPtr finger_joint_middle_right[LeapMotionInterface::Bone::BONE_NUM_TYPES + 1];
	ObjectMeshDynamicPtr finger_joint_ring_left[LeapMotionInterface::Bone::BONE_NUM_TYPES + 1];
	ObjectMeshDynamicPtr finger_joint_ring_right[LeapMotionInterface::Bone::BONE_NUM_TYPES + 1];
	ObjectMeshDynamicPtr finger_joint_pinky_left[LeapMotionInterface::Bone::BONE_NUM_TYPES + 1];
	ObjectMeshDynamicPtr finger_joint_pinky_right[LeapMotionInterface::Bone::BONE_NUM_TYPES + 1];


	ObjectMeshDynamicPtr left_bone_thumb[4];
	ObjectMeshDynamicPtr left_bone_index[4];
	ObjectMeshDynamicPtr left_bone_middle[4];
	ObjectMeshDynamicPtr left_bone_ring[4];
	ObjectMeshDynamicPtr left_bone_pinky[4];
	ObjectMeshDynamicPtr left_bone_index_thumb;
	ObjectMeshDynamicPtr left_bone_arm;
	ObjectMeshDynamicPtr left_bone_index_pinky;
	ObjectMeshDynamicPtr left_hand_holder;

	ObjectMeshDynamicPtr right_bone_thumb[4];
	ObjectMeshDynamicPtr right_bone_index[4];
	ObjectMeshDynamicPtr right_bone_middle[4];
	ObjectMeshDynamicPtr right_bone_ring[4];
	ObjectMeshDynamicPtr right_bone_pinky[4];
	//bone between index and thumb
	ObjectMeshDynamicPtr right_bone_index_thumb;
	ObjectMeshDynamicPtr right_bone_arm;
	ObjectMeshDynamicPtr right_bone_index_pinky;
	ObjectMeshDynamicPtr right_hand_holder;

	float length_thumb_left[4];
	float length_index_left[4];
	float length_middle_left[4];
	float length_ring_left[4];
	float length_pinky_left[4];
	float length_index_thumb_left;
	float length_arm_left;
	float length_hand_holder_left;

	float length_thumb_right[4];
	float length_index_right[4];
	float length_middle_right[4];
	float length_ring_right[4];
	float length_pinky_right[4];
	float length_index_thumb_right;
	float length_arm_right;
	float length_hand_holder_right;

	void changeMeshBonesTransform(int leftorright, Mat4 transform, int num);

	bool isHoldingLeft;
	bool isHoldingRight;

	//interaction
	// pointer (part of grabbing)
	NodePtr CurrentHandNode;

	float ptr_width = 0.0125f;
	float ray_back_length = 0.03f;		// ray's length of controllers from pivot
	float ray_forward_length = 0.05f;
	//use to detect the release status of hand
	Vector<bool> isHolding;

	WorldIntersectionPtr intersection = WorldIntersection::create();

	// the object current hit that need to hold
	ObjectPtr hitObj;
	Vector<ObjectPtr> intersections;
	Vector<int> hand_state;		// status of hand: free, grab, hold, throw, etc.
	Vector<int> node_selected;	// hand around grabable node?
	Vector<NodePtr> grab_node;
	Vector<int> node_grabbed;	// hand grab something?
	Vector<ObjectPtr> last_selected;
	Vector<float> last_selected_timer;

	// hand velocities
	Vector<Vector<vec3>> hand_linear_velocity;
	Vector<Vector<vec3>> hand_angular_velocity;


	void grab_update(int num, int closed_hand);

	enum GRAB_MODE
	{
		BOUNDBOX,
		INTERSECTIONS
	};

	GRAB_MODE grab_mode = GRAB_MODE::BOUNDBOX;

	enum {
		HAND_FREE,
		HAND_GRAB,
		HAND_HOLD,
		HAND_THROW
	};

	void set_outline(int num, int enable);
	int can_i_grab_it(const NodePtr &node);

	void grab_init();
};

