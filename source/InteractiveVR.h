#pragma once
#include "VR.h"
#include <UnigineGame.h>
#include "Utils.h"
#include "ObjMovable.h"
#include "Smoke.h"
#include <UnigineMap.h>
#include "MiniGun.h"
#include "Resource.h"
#include "Winch.h"
#include "Glove.h"
#include "LeapMotion.h"

class InteractiveVR 
{
public:

	static InteractiveVR* instance;
	static InteractiveVR* get();

	virtual void init();

	virtual void update();
	virtual void render();


	virtual void destroy();

	virtual ~InteractiveVR() {}
	//the second int is the nodeID 
	//map used to store nodeID
	
	//map used to store smoke obj
	Map<int, Smoke*> smokes;
	Map<int, Smoke*> lids;
	Map<int, Smoke*> tabs;
	Map<int, ObjMovable*> seamarks;
	Map<int, ObjMovable*> laserpoints;
	Map<int, ObjMovable*> flashlights;
	
	Vec3 headsetPosition;
	NodePtr box;
	NodePtr tab_node;
	NodePtr smoke_node;
	NodePtr lid_node;
	NodePtr seamark_node;
	NodePtr smoke_reference;
	NodePtr lid_reference;
	NodePtr tab_reference;
	NodePtr effect_reference;
	NodePtr laserpoint_node;
	NodePtr flashlight_node;

	ObjectParticlesPtr effect_smoke_particle;
	ObjectParticlesPtr effect_seamark_particle;


	Mat4 initialPos_smoke;
	Mat4 initialPos_Lid;
	Mat4 initialPos_Tab;
	Mat4 initialPos_seamark;
	Mat4 initialPos_laserpoint;
	Mat4 initialPos_flashlight;

	bool isCreateNewItem;
	Smoke* smoke;
	Smoke* tab;
	Smoke* lid;

	ObjMovable* handle;
	ObjMovable* hook;
	ObjMovable* door;
	ObjMovable* door1;


	ObjectParticlesPtr seamark_effect;

	Smoke* findSmokeinMaps(Map<int, Smoke*> map, NodePtr node);
	ObjMovable* findMovableObjinMaps(Map<int, ObjMovable*> map, NodePtr node);

	void allShowRay(Map<int, ObjMovable*> map);
	void allHideRay(Map<int, ObjMovable*> map);
	void allClear(Map<int, ObjMovable*> map);
	void allDim(Map<int, ObjMovable*> map);
	void updateSmokeMap(Map<int, Smoke*> map);
	void updateMovableObjMap(Map<int, ObjMovable*> map);

	void createNewSmoke();
	void createNewSeamark();
	void createNewLaserPoint();
	void createNewFlashlight();
	bool allTrownSmoke(Map<int, Smoke*> map);
	bool allTrownMovableObj(Map<int, ObjMovable*> map);
	Smoke* findunThrownSmoke(Map<int, Smoke*> map);
	ObjMovable* findunThrownMovableObj(Map<int, ObjMovable*> map);
	void resetPosition();

	Mat4 vive_tracker_transfrom;

	bool isNight;
	bool isNVG;
	bool isBatteryFailureMode;

	Map<int, ObjectParticlesPtr> smoke_effects;

	VR *vr = nullptr; // if VR mode is enabled

protected:

	
	NodePtr seamark;
	void createNewItems();

	//used as a one time flag
	bool oneTimeFlag = true;


	//virtual GuiPtr getGui() override { return gui->getGui(); }

	//int getHandDegreesOfFreedom(int num) ;	// get hand's degrees of freedom


	// called when player holds "use" button when holding some object
	UNIGINE_INLINE void setUseCallback(CallbackBase* callback) { delete use_callback; use_callback = callback; } // (NodePtr node)
																												 // called when player grab some object
	UNIGINE_INLINE void setGrabCallback(CallbackBase* callback) { delete grab_callback; grab_callback = callback; } // (NodePtr node)
																													// called when player holding some object
	UNIGINE_INLINE void setHoldCallback(CallbackBase* callback) { delete hold_callback; hold_callback = callback; } // (NodePtr node)
																													// called when player throw some object
	UNIGINE_INLINE void setThrowCallback(CallbackBase* callback) { delete throw_callback; throw_callback = callback; } // (NodePtr node)

	UNIGINE_INLINE NodePtr getHead() { return vr->getHead(); }
	UNIGINE_INLINE void setGrabMode(VR::GRAB_MODE mode) { vr->setGrabMode(mode); }			// grab via BoundBox-BoundBox or Line-Surface intersection?
	UNIGINE_INLINE int getNumHands() { return vr->getNumHands(); }									// get count of hands
	UNIGINE_INLINE NodePtr getHandNode(int num) { return vr->getHandNode(num); }
	UNIGINE_INLINE vec3 getHandLinearVelocity(int num) { return vr->getHandLinearVelocity(num); }	// get speed of hand
	UNIGINE_INLINE vec3 getHandAngularVelocity(int num) { return vr->getHandAngularVelocity(num); }	// get angular speed of hand
	UNIGINE_INLINE int getHandState(int num) { return vr->getHandState(num); }						// get hand state (grab, hold, throw)
	UNIGINE_INLINE NodePtr getGrabNode(int num) { return vr->getGrabNode(num); }					// get object, grabbed by hand

	
private:

	// callbacks
	CallbackBase* use_callback = NULL;
	CallbackBase* grab_callback = NULL;
	CallbackBase* hold_callback = NULL;
	CallbackBase* throw_callback = NULL;


	int is_gamepad_last_control = 0;

};