#pragma once

#include <UnigineNode.h>
#include <UnigineGeodetics.h>
#include <UnigineMaterial.h>
#include <UnigineMaterials.h>
#include <UnigineLights.h>
#include <UnigineEditor.h>
#include <UnigineSounds.h>
#include <UniginePlayers.h>
#include <UnigineWorlds.h>;
#include "Utils.h"

using namespace Unigine;
using namespace Math;
class Resource
{
public:

	static Resource* instance;
	static Resource* get();

	void init();
	float handle_mass = 10;
	float hook_mass = 1;
	float rope_mass_upper = 1;
	float rope_mass_lower = 1;
	NodePtr vive_tracker_0;
	NodePtr vive_controller_0;
	NodePtr vive_controller_1;
	NodePtr vive_tracker_1;
	NodePtr vive_tracker_2;

	NodePtr vive_controller_2;
	NodePtr vive_tracker_reference;
	NodePtr WinchTrigger;
	NodePtr vive_tracker_helicopter;
	NodePtr RotorDummy;
	NodePtr RotorDummy_1;
	NodePtr rotor_1;
	NodePtr WinchArm;
	NodePtr water;
	NodePtr ObjectParticles_Airflow;
	NodePtr RightDoor;
	NodePtr LeftDoor;
	NodePtr flood;
	NodePtr bushfire;

	NodePtr HelicopterPad;
	NodePtr greenlight;
	NodePtr wall1_point1;
	NodePtr wall1_point2;
	NodePtr wall2_point1;
	NodePtr wall2_point2;
	NodePtr door_handle_2;
	NodePtr door_handle_3;
	NodePtr hudAGL_text;
	NodePtr hudDTR_text;
	NodePtr weight_basket;
	NodePtr weight_strop;
	NodePtr weight_stretcher;
	NodePtr fieldHeightWater;
	NodePtr smoke_node;
	NodePtr tab_node;
	NodePtr lid_node;
	NodePtr seamark_node;
	NodePtr smoke_reference;
	NodePtr tab_reference;
	NodePtr lid_reference;
	NodePtr effect_reference;
	NodePtr box;
	NodePtr laserpoint_node;
	NodePtr flashlight_node;
	NodePtr fire_effect_small;
	NodePtr fire_effect_middle;
	NodePtr fire_effect_large;
	NodePtr fire_effect_huge;
	NodePtr fire_effect_huge_low;
	NodePtr fire_mesh;
	NodePtr burn_area_small;
	NodePtr burn_area_middle;
	NodePtr burn_area_large;
	NodePtr burn_area_huge;
	NodePtr burn_area_huge_low;
	NodePtr airport_lights;
	NodePtr flash_beam;
	NodePtr hookreference;
	NodePtr currentHook;
	NodePtr scrollwheel;
	NodePtr haze_copy;
	NodePtr rope_upper;
	NodePtr rope_lower;
	NodePtr Handle;
	NodePtr Rope_intial_point;
	NodePtr shoot_effect_barrel_smoke;
	NodePtr shoot_effect_muzzle_flash;
	NodePtr barrel;
	NodePtr bullet;
	NodePtr shell;
	NodePtr shell_position;
	NodePtr shell_end;
	NodePtr barrel_1;
	NodePtr barrel_2;
	NodePtr M134;
	NodePtr delink;
	NodePtr delink_position;
	NodePtr BarrelRotatorParent;
	NodePtr bullet_effect;
	NodePtr explosion_effect;
	NodePtr tracer;
	NodePtr oil_tank;
	NodePtr gun_holder;
	NodePtr left_hand_glove;
	NodePtr right_hand_glove;
	NodePtr Flir_view_hanger;
	NodePtr flir_camera_point1;
	NodePtr flir_camera_point2;
	NodePtr hmd_reference;
	NodePtr leap_motion_camera; 
	NodePtr left_hand_bone;
	NodePtr right_hand_bone;
	NodePtr fireball;

	//player
	PlayerSpectatorPtr Go_pro_view;
	PlayerDummyPtr pilot_view;
	PlayerDummyPtr Top_view;
	PlayerDummyPtr Camera_ThirdPerson;
	PlayerDummyPtr player_helicopterPad;
	PlayerDummyPtr player_boat;
	PlayerDummyPtr player_bushfire;
	PlayerDummyPtr player_factory;
	PlayerDummyPtr player_grass;
	PlayerDummyPtr player_forest;
	PlayerDummyPtr player_satellite;
	PlayerDummyPtr player_flir;
	PlayerDummyPtr player_flir_reference;

	ObjectTerrainGlobalPtr land;


	ObjectMeshStaticPtr wall1;
	ObjectMeshStaticPtr wall2;


	MaterialPtr torch_mat;
	MaterialPtr haze_mat;
	MaterialPtr haze_mat2;
	MaterialPtr haze_volume_mat;
	MaterialPtr bushfire_mat;
	MaterialPtr bushfire_mat_huge;
	MaterialPtr bushfire_mat_middle;
	MaterialPtr bushsmoke_mat1; 
	MaterialPtr bushsmoke_huge_0;
	MaterialPtr bushsmoke_huge_1;
	MaterialPtr haze_mat_dynamic;
	MaterialPtr handle_mat;
	MaterialPtr rope_mat;
	MaterialPtr gun_smoke_mat_1;
	MaterialPtr gun_smoke_mat_2;
	MaterialPtr smoke_canister_mat;
	MaterialPtr greenlight_mat;
	MaterialPtr caution_mat;
	MaterialPtr overheat_mat;
	MaterialPtr star_mat;

	LightWorldPtr moon;
	LightWorldPtr sun;

	GeodeticPivotPtr geopivot;
	ObjectWaterGlobalPtr water_global;


	ObjectCloudLayerPtr clouds;

	ObjectTextPtr ui_text1;
	ObjectTextPtr ui_text2;
	ObjectTextPtr ui_text3;
	ObjectTextPtr ui_text4;
	ObjectTextPtr bulletNumberText;

	ObjectParticlesPtr airflow;
	ObjectParticlesPtr waterflow;
	ObjectParticlesPtr haze_particle_copy;
	ObjectParticlesPtr haze_particle;
	ObjectParticlesPtr haze_dark_particle;
	ObjectParticlesPtr shoot_effect_barrel_smoke1;
	ObjectParticlesPtr shoot_effect_barrel_smoke2;
//	ObjectParticlesPtr bullet_effect;
	SoundSourcePtr sound_fire_loop;
	SoundSourcePtr sound_fire_down;
	SoundSourcePtr sound_fire_up;

	AmbientSourcePtr sound_explosion;
	AmbientSourcePtr sound_button;


	ObjectMeshSkinnedPtr LeftHand_Mesh_Glove;
	ObjectMeshSkinnedPtr RightHand_Mesh_Glove;

	ObjectMeshSkinnedPtr LeftHand_Mesh_LeapMotion;
	ObjectMeshSkinnedPtr RightHand_Mesh_LeapMotion;
};

                                                                               