#pragma once
#include <UniginePlayers.h>
#include <UnigineEngine.h>
#include <UnigineLights.h>
#include <UnigineDecals.h>
#include <UnigineInterpreter.h>
#include <UnigineGame.h>
#include <UnigineMaterials.h>
#include "Utils.h"
#include "Helicopter.h"
#include "InteractiveVR.h"
#include "Winch.h"

using namespace Unigine;
using namespace Math;

//this class is used in demo version
class XmlControl
{
public:


	friend class Helicopter;
	friend class InteractiveVR;
	friend class Resource;
	
	void init();
	void update();
	void shutdown();

	Vec3 hotpoint6InitialPosition;
	vec3 getHellicopterDirection(float value);

	Winch *winch = nullptr;
	
	//store position
	Vec3 initialBoxPosition;
	quat initialBoxRotation;
	String  positionX, positionY, positionZ, rotationX, rotationY, rotationZ, rotationW;
	float x, y, z, Rx, Ry, Rz, Rw;
	bool demoToCigi = true;

	int seastate_value;

	AmbientSourcePtr sound;

	Map<int, ObjectParticlesPtr> bushfire_effects;
	Map<int, ObjectParticlesPtr> haze_effects;
	Map<int, ObjectParticlesPtr> cheminey_effects;
	Map<int, float>  bushfire_effects_velocities;



	XmlPtr xml = Xml::create();
	XmlPtr xml2 = Xml::create();
	String  currentTime, currentClouds, currentWind, demooption, currentHotpoint, currentWeight, hotpoint6Rest, nightvision,whitenvg, darkvision, NVGCurrentTubeFailure, NVGTubeFailureEnabled, Lunarvalue, SoundStop;
	String SeaStateRange, changeWeather, Rain, GreenLight, FloodHeight, NorthSouth, WestEast, FireVisable, hudAGL, hudDTR, HazeData;
	String CameraNumber, HoistFailureNumber, LifeRaftVisible, GroundCasualtyVisible, SwimmerVisible,WaterDirection,DistanceBetweenLenses;
	float timevalue, cloudsvalue, windvalue, rainvalue, hazevalue, floodvalue, northsouthvalue, westeastvalue,water_direction_value,distance_lenses = 0.6;
	int  camera_number, hoistfailure_number, liferaft_number, groundcasualty_number, swimmer_number, isWhiteNVG;
	int  hotpoint, weightNumber, hotpointreset, isNightVision, currentFailure, NVGCurrentTubeFailureValue, isTubeFailure, moonValue, isSoundEnable;
	int isChangeWeather, isGreenlight, isfireshow, ishudAGL, ishudDTR;
	int currentPoint = 99;
	int isdemo = 0;

	vec4 textColor1;
	vec4 textColor2;

	int moonstate;

	PlayerDummyPtr player_cigi;

	///must have a initial value
	float day_cycle_time = 8.0f;
	float rain_power = 0.0f;
	float clouds_power = 0.75f;

	bool isNight;;
	bool isNVG;
	bool isBatteryFailureMode;

	void initial_sound();
	void loadPosition();
	void storePosition();
	void loadTracker();
	//read XML file
	void loadXML();
	void updatelocalXML();
	void storeTerrainHeight();
	void updateHotpoint();
	void updateHosit();
	void updateWaterDirection();
	void updateAGLtext();
	void updateWeight();
	void updateNVG();
	void update_laser_flash_light();
	void updateMoon();
	void changeEnvironment();
	void beaufort_changed();
	void change_weather_and_time();
	void change_clouds();
	void changewindSpeed();
	void openGreenLight();
	void changefloodHieght();
	void changewindDirection();
	void seteffectGravity(Map<int, ObjectParticlesPtr> effects, float ratio, float normalGravity, float zGravity);
	void seteffectSpawnRate(Map<int, ObjectParticlesPtr> effects, float ratio, int initialNumber);
	void loadEffectVelocity(Map<int, ObjectParticlesPtr> effects);
	void seteffectVelocity(Map<int, ObjectParticlesPtr> effects, float ratio);
	void updateSmokeEffects(NodePtr node, Map<int, ObjectParticlesPtr> &smoke_effects, const char * property_name);
	void loadSmokeEffects(Map<int, ObjectParticlesPtr> &smoke_effects, const char * property_name);
	void showFire();
	void change_sound();
	void updateAGL();
	void updateDBL();
	int checkSmokeEffect(const PropertyPtr &prop, const char * property_name);
	void setParticleForce(Map<int, ObjectParticlesPtr> effects, float ratio);


	//old rope
	NodePtr weight_basket;
	NodePtr weight_strop;
	NodePtr weight_stretcher;
};

