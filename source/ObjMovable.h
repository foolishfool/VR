#pragma once
#include <UnigineNode.h>
#include <UniginePhysics.h>
#include <UnigineVector.h>
#include <UnigineMap.h>
#include "Obj.h"
#include "Utils.h"


using namespace Unigine;
using namespace Math;

class ObjMovable: public Obj
{
public:
	friend class Resource;
	ObjectParticlesPtr seamark_effect;
	float currentTime;
	static int checkObject(const PropertyPtr &prop);
	static ObjMovable* getObjMovable(const NodePtr &node);

	ObjMovable(const NodePtr &node) { init(node); };
	virtual ~ObjMovable() {}

	void init(const NodePtr &node) override;
	void update() override;
	void render() override {}
	void shutdown() override;
	void destroy() override {}

	void grabIt(VR* vr, const NodePtr &hand, int hand_num, int dof, const vec3 &linear_velocity, const vec3 &angular_velocity) override;
	void holdIt(VR* vr, const NodePtr &hand, int hand_num, int dof, const vec3 &linear_velocity, const vec3 &angular_velocity) override;
	void useIt(int isOpen);
	void throwIt(VR* vr, const NodePtr &hand, int hand_num, int dof, const vec3 &linear_velocity, const vec3 &angular_velocity) override;

	void grabIt(Glove* glove, const NodePtr &hand, int hand_num, int dof, const vec3 &linear_velocity, const vec3 &angular_velocity) override;
	void holdIt(Glove* glove, const NodePtr &hand, int hand_num, int dof, const vec3 &linear_velocity, const vec3 &angular_velocity) override;
	void throwIt(Glove* glove, const NodePtr &hand, int hand_num, int dof, const vec3 &linear_velocity, const vec3 &angular_velocity) override;

	void grabIt(LeapMotion* glove, const NodePtr &hand, int hand_num, int dof, const vec3 &linear_velocity, const vec3 &angular_velocity) override;
	void holdIt(LeapMotion* glove, const NodePtr &hand, int hand_num, int dof, const vec3 &linear_velocity, const vec3 &angular_velocity) override;
	void throwIt(LeapMotion* glove, const NodePtr &hand, int hand_num, int dof, const vec3 &linear_velocity, const vec3 &angular_velocity) override;
	
	//the handle needs to follow the movement of the hook
	void followtheHook(NodePtr &hand, NodePtr &handle);
	//old version
	void followtheHook(NodePtr &hand);
	BodyRigid* getBody() { return body.get(); }

	// events
	Action<void(BodyPtr, int)> onContact;
	Action<void(BodyPtr)> onFrozen;

	int use_handy_pos = 0;
	int rope_or_hook; // 0 rope  1 hook

	Vector<ObjectPtr> weights;
	int hit_detect(Vector<ObjectPtr> nodes);

	void ObjMovable::attach(ObjectPtr weight);
	void ObjMovable::detach(ObjectPtr weight);

	vec3 linear_velocity1;
	vec3 angular_velocity1;

	ObjectPtr weight;
	
	bool isHandlePositionReset = false;
	bool isAttached = false;

	enum Status
	{
		CONNECTED,
		DISCONNECTED,
		THROWN
	};

	ObjMovable *parent = nullptr;
	int status;
	NodePtr laser_ray;
	void setRayDirection(NodePtr laser_ray, Vec3 head_position);
	void hideRay();
	void showRay();
	void clear();
	void dim();

	bool isTimingEffect;
	bool isTimingEffectClosed;

	float elapsed_time_effect; //time for begin to effect
	float elapsed_time_close; // the time for close effect

private:



	float initialMass;
	static Map<int, ObjMovable*> links; // links from node to objMovable

	BodyRigidPtr body;
	Mat4 transform;			// local transform (when holding) of the node
	NodePtr null_node;
	Vec3 initialPosition; // local position of the handle
	quat initialRotation; // local rotation of the handle
	
	NodePtr point;
	NodePtr handle;
	NodePtr door_reference;
	NodePtr door_reference_closed;
	NodePtr door_reference1;
	NodePtr door_reference_closed1;

	float distance_z; // the intial distance between handle and initial point in z axis 
	int hold = 0;			// this node is holding now?


	NodePtr laser_light;
	NodePtr laser_hit;
	Mat4 laser_ray_mat;
	MaterialPtr laserpointer_mat;
	WorldIntersectionPtr intersection = WorldIntersection::create();


	NodePtr flash_light;
	NodePtr flashlight_ray;
	Mat4 flashlight_ray_mat;
	float flashlight_ray_initial_length;
	MaterialPtr flashlight_mat;
	ObjectVolumeProjPtr flashlight_volume;
	float flashlight_volume_initial_length;
	LightProjPtr flash;
	LightProjPtr flash2;


	ObjectTerrainPtr land;
	// saveState / restoreState variables
	Vector<int> s_immovable;
	Vector<int> s_frozen;

	float handy_pos_factor = 7.5f; // speed of changing pos/rot to handy position for hand

	float max_attach_distance = 0.3f;
	NodePtr attached_to = null_node;
	vec3 hat_def_pos; // hat default position / rotation
	vec3 hat_def_rot;
	vec3 handy_pos;
	vec3 handy_pos_vr;
	quat handy_rot, handy_rot_vr;
	int type;
	const char* on_grab_event;
	int hasRigidbody = 0;
	int is_door_handle;

	int mass_index = 0;		   // 0 - auto, 1 - lightest, 2 - light...
	Vector<float> shapes_mass; // mass of each shape in body

	// buffer of last two positions for true velocity detection
	Vec3 last_pos[2];
	int last_pos_index;

	void unfreeze(const BodyPtr &body);

	void activate_body(const BodyPtr &body);

	

	int ObjMovable::isWeight(const NodePtr &node);
	void ObjMovable::set_outline(ObjectPtr node, int enable);
	//set node height
	void setNodeHeight(NodePtr node, ObjectTerrainPtr land);
	// callbacks and actions
	UNIGINE_INLINE void position_callback(BodyPtr body) { last_pos_index = 1 - last_pos_index; last_pos[last_pos_index] = body->getPosition(); }
	UNIGINE_INLINE void contact_callback(BodyPtr body, int num) { onContact(body, num); }
	UNIGINE_INLINE void frozen_callback(BodyPtr body) { onFrozen(body); }

	int unfreeze_action_id, mute_action_id;
	void unfreeze_action(const BodyPtr &body, int num);
	void mute_action(const BodyPtr &body);

};
