#ifndef __APP_WORLD_LOGIC_H__
#define __APP_WORLD_LOGIC_H__

#include <UnigineLogic.h>
#include <UnigineStreams.h>
#include "Helicopter.h"
#include "InteractiveVR.h"
#include "XmlControl.h"
#include "UI.h"
#include "VR.h"
#include "Glove.h"
#include "WorldSyncker.h"
#include "LeapMotion.h"

class AppWorldLogic : public Unigine::WorldLogic {
	
public:


	AppWorldLogic();
	virtual ~AppWorldLogic();
	
	virtual int init();
	
	virtual int update();
	virtual int render();
	virtual int flush();
	
	virtual int shutdown();
	virtual int destroy();
	
	virtual int save(const Unigine::StreamPtr &stream);
	virtual int restore(const Unigine::StreamPtr &stream);

	bool loadWorld;
	

private:
	// declaring a CigiClient manager interface pointer

	Editor *editor;
	Glove *glove;
	WorldSyncker *worldsyncker;


	Cigi::CigiClientInterface* cigi = nullptr;

	float min_fov = 0.25f;
	float max_fov = 29.0f;
	float currentFov = 29.0f;

	PlayerDummyPtr flirPlayer;

	bool changeCamera;

	//create new entity
	void create_entity(Cigi::ICigiEntityControl* control);

	Vector<int> entityID;
	Vector<NodePtr> fires;
	Vector<NodePtr> burn_areas;
	bool createNode = false;

	PlayerDummyPtr player_cigi;
	PlayerPtr currentPlayer;
	Resource *resources = nullptr;
	LeapMotion *leapmotion = nullptr;
	UI *ui = nullptr;
	Helicopter *helicopter = nullptr;
	bool initializeCamera = false;

	Vector<NodePtr> nodes;
	Vector<MaterialPtr> materials;

	XmlControl *xmlcontrol = nullptr;

	InteractiveVR *interactive = nullptr;
	void updateSatelliteCamera();
	void updateFlirCamera();
	
	Vector<ObjectParticlesPtr> haze_particles;
	Vector<NodePtr> haze_nodes;
	Vector<MaterialPtr> haze_materials;
	void updateHaze(Vector<NodePtr> haze_nodes);
	void createHaze(int haz
		ber, float radius_radio, float grow_radio);
	void AppWorldLogic::upHazeDark();
	enum EntityType
	{
		SMALL_FIRE = 0,
		MIDDLE_FIRE,
		LARGE_FIRE,
		HUGE_FIRE,
		HUGE_FIRE_LOW
	};


	void setSmoke(ObjectParticlesPtr smoke, float radiusMin, float radiusMax, float growthMin, float growthMax, float lifetimeMin, float lifetimeMax,
		float velocityMin, float velocityMax, float spreadx, float spready, float spreadz);

};

#endif // __APP_WORLD_LOGIC_H__
