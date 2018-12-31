#include "Resource.h"


Resource* Resource::instance;

Resource* Resource::get()
{
	return instance;
}


void Resource::init()
{
	Sound::get()->renderWorld(1);

	instance = this;
	vive_tracker_0 = LoadFromEditor("vive_tracker_0");
	vive_controller_0 = LoadFromEditor("vive_controller_0");
	vive_controller_1 = LoadFromEditor("vive_controller_1");
	vive_controller_2 = LoadFromEditor("vive_controller_2");
	vive_tracker_1 = LoadFromEditor("vive_tracker_1");
	vive_tracker_2 = LoadFromEditor("vive_tracker_2");

	vive_tracker_reference = LoadFromEditor("vive_tracker_reference");
	WinchTrigger = LoadFromEditor("WinchTrigger");
	vive_tracker_helicopter = LoadFromEditor("vive_tracker_helicopter");
	RotorDummy = LoadFromEditor("RotorDummy");
	RotorDummy_1 = LoadFromEditor("RotorDummy_1");
	rotor_1 = LoadFromEditor("rotor_1");
	WinchArm = LoadFromEditor("WinchArm");
	water = LoadFromEditor("water");
	ObjectParticles_Airflow = LoadFromEditor("ObjectParticles_Airflow");
	RightDoor = LoadFromEditor("RightDoor");
	LeftDoor = LoadFromEditor("LeftDoor");
	flood = LoadFromEditor("flood");
	bushfire = LoadFromEditor("bushfire");

	HelicopterPad = LoadFromEditor("HelicopterPad");
	greenlight = LoadFromEditor("greenlight");
	wall1_point1 = LoadFromEditor("point1_1");
	wall1_point2 = LoadFromEditor("point1_2");
	wall2_point1 = LoadFromEditor("point2_1");
	wall2_point2 = LoadFromEditor("point2_2");
	door_handle_2 = LoadFromEditor("door_handle_2");
	door_handle_3 = LoadFromEditor("door_handle_3");
	hudAGL_text = LoadFromEditor("hudAGL");
	hudDTR_text = LoadFromEditor("hudDTR");
	weight_basket = LoadFromEditor("Basket");
	weight_strop = LoadFromEditor("Strop");
	weight_stretcher = LoadFromEditor("Stretcher");
	fieldHeightWater = LoadFromEditor("FieldHeightWater");
	smoke_node = LoadFromEditor("smoke_canister");
	tab_node = LoadFromEditor("Tab");
	lid_node = LoadFromEditor("LidPArent");
	seamark_node = LoadFromEditor("SeaMark");
	smoke_reference = LoadFromEditor("smoke3");
	tab_reference = LoadFromEditor("Tab3");
	lid_reference = LoadFromEditor("LidPArent3");
	effect_reference = LoadFromEditor("smoke_effect3");
	box = LoadFromEditor("Box");
	laserpoint_node = LoadFromEditor("laser_pointer");
	flashlight_node = LoadFromEditor("flashlight");
	fire_effect_small = LoadFromEditor("fire_effect_small");
	fire_effect_middle = LoadFromEditor("fire_effect_middle");
	fire_effect_large = LoadFromEditor("fire_effect_large");
	fire_effect_huge = LoadFromEditor("fire_effect_huge");
	fire_effect_huge_low = LoadFromEditor("fire_effect_huge_low");
	fire_mesh = LoadFromEditor("fire_mesh");
	burn_area_small = LoadFromEditor("burn_area_small");
	burn_area_middle = LoadFromEditor("burn_area_middle");
	burn_area_large = LoadFromEditor("burn_area_large");
	burn_area_huge = LoadFromEditor("burn_area_huge");
	burn_area_huge_low = LoadFromEditor("burn_area_huge_low");
	airport_lights = LoadFromEditor("airport_lights");
	flash_beam = LoadFromEditor("flash_beam");
	hookreference = LoadFromEditor("Hook_reference");
	currentHook = LoadFromEditor("Hook");
    scrollwheel = LoadFromEditor("ScrollWheelRotatorDummy");
	haze_copy = LoadFromEditor("haze_copy");
	rope_upper = LoadFromEditor("rope_upper");
	rope_lower = LoadFromEditor("rope_lower");
	Handle = LoadFromEditor("Handle");
	Rope_intial_point = LoadFromEditor("Rope_intial_point");
	shoot_effect_muzzle_flash = LoadFromEditor("vfx_barrel_smoke");
	barrel = LoadFromEditor("Muzzle Guard");
	bullet = LoadFromEditor("bullet");
	shell = LoadFromEditor("shell");
	shell_position = LoadFromEditor("shell_position");
	shell_end = LoadFromEditor("shell_end");
	barrel_1 = LoadFromEditor("barrel_1");
	barrel_2 = LoadFromEditor("barrel_2");
	M134 = LoadFromEditor("M134");
	delink = LoadFromEditor("delink");
	delink_position = LoadFromEditor("delink_position");
	BarrelRotatorParent = LoadFromEditor("BarrelRotatorParent");
	bullet_effect = LoadFromEditor("bullet_effect");
	explosion_effect = LoadFromEditor("explosion_oil_tank_effects");
	tracer = LoadFromEditor("tracer");
	oil_tank = LoadFromEditor("oil_tank");
	gun_holder = LoadFromEditor("gun_holder");
	left_hand_glove = LoadFromEditor("Human_LeftHand_Glove");
	right_hand_glove = LoadFromEditor("Human_RightHand_Glove");
	Flir_view_hanger = LoadFromEditor("Flir_view_hanger");
	flir_camera_point1 = LoadFromEditor("flir_camera_point1");
	flir_camera_point2 = LoadFromEditor("flir_camera_point2");
	hmd_reference = LoadFromEditor("hmd_reference");
	leap_motion_camera = LoadFromEditor("leap_motion_camera");
	left_hand_bone = LoadFromEditor("Left_Hand_Bone");;
	right_hand_bone = LoadFromEditor("Right_Hand_Bone");;
	fireball = LoadFromEditor("Fireball");
	//player
	Go_pro_view = PlayerSpectator::cast(LoadFromEditor("Go_pro_view"));
	pilot_view = PlayerDummy::cast(LoadFromEditor("pilot_view"));
	Top_view = PlayerDummy::cast(LoadFromEditor("Top_view"));
	Camera_ThirdPerson = PlayerDummy::cast(LoadFromEditor("Camera_ThirdPerson"));
	player_helicopterPad = PlayerDummy::cast(LoadFromEditor("position_helicopterPad"));
	player_boat = PlayerDummy::cast(LoadFromEditor("position_boat"));
	player_bushfire = PlayerDummy::cast(LoadFromEditor("position_bushfire"));
	player_factory = PlayerDummy::cast(LoadFromEditor("position_factory"));
	player_grass = PlayerDummy::cast(LoadFromEditor("position_grass"));
	player_forest = PlayerDummy::cast(LoadFromEditor("position_forest"));
	///must be position_satellite otherwise doesn't work ,don't konw why
	player_satellite = PlayerDummy::cast(LoadFromEditor("position_satellite"));
	player_flir = PlayerDummy::cast(LoadFromEditor("position_flir"));
	player_flir_reference = PlayerDummy::cast(LoadFromEditor("Flir_view"));

	land = ObjectTerrainGlobal::cast(LoadFromEditor("Land"));


	wall1 = ObjectMeshStatic::cast(LoadFromEditor("wall1"));
	wall2 = ObjectMeshStatic::cast(LoadFromEditor("wall2"));

	haze_mat_dynamic = Materials::get()->findMaterial("library_smoke_billow_haze");
	
	torch_mat = Materials::get()->findMaterial("bulb_glower_mat_0");
	bushfire_mat = Materials::get()->findMaterial("library_fire_middle_v2_mat"); 
	bushfire_mat_huge = Materials::get()->findMaterial("library_fire_middle_v2_mat_huge");
	bushfire_mat_middle = Materials::get()->findMaterial("library_fire_middle_0");
	bushsmoke_mat1 = Materials::get()->findMaterial("library_smoke_billow_light3");
	bushsmoke_huge_0 = Materials::get()->findMaterial("library_smoke_billow_light3_haze");
	bushsmoke_huge_1 = Materials::get()->findMaterial("library_smoke_billow_light3_dark_0");
	haze_mat = Materials::get()->findMaterial("library_smoke_billow_dark");
	haze_mat2 = Materials::get()->findMaterial("library_smoke_billow_dark_test");
	haze_volume_mat = Materials::get()->findMaterial("volume_fog_base_1");

	handle_mat = Materials::get()->findMaterial("handle");
	rope_mat = Materials::get()->findMaterial("rope_upper");
	gun_smoke_mat_1 = Materials::get()->findMaterial("library_tank_wispy_white_dust_0");
	gun_smoke_mat_2 = Materials::get()->findMaterial("library_tank_wispy_white_dust");
	smoke_canister_mat = Materials::get()->findMaterial("smoke_effect");
	greenlight_mat = Materials::get()->findMaterial("green_light");

	caution_mat = Materials::get()->findMaterial("caution_light");
	overheat_mat = Materials::get()->findMaterial("overheat_light");
	star_mat = Materials::get()->findMaterial("star");

	moon = LightWorld::cast(LoadFromEditor("moon"));
	sun = LightWorld::cast(LoadFromEditor("sun"));


	geopivot = GeodeticPivot::cast(LoadFromEditor("GeodeticPivot"));
	water_global = ObjectWaterGlobal::cast(water);


	clouds = ObjectCloudLayer::cast(LoadFromEditor("clouds_lower"));

	ui_text1 = ObjectText::cast(LoadFromEditor("hudAGL"));
	ui_text2 = ObjectText::cast(LoadFromEditor("hudDTR"));
	ui_text3 = ObjectText::cast(LoadFromEditor("AGL"));
	ui_text4 = ObjectText::cast(LoadFromEditor("DSR"));
	bulletNumberText = ObjectText::cast(LoadFromEditor("bullet_number"));

	airflow = ObjectParticles::cast(LoadFromEditor("ObjectParticles_Airflow"));
	waterflow = ObjectParticles::cast(LoadFromEditor("ObjectParticles_Waterflow"));
	haze_particle_copy = ObjectParticles::cast(haze_copy);
	haze_dark_particle = ObjectParticles::cast(LoadFromEditor("haze_dark"));
	haze_particle = ObjectParticles::cast(LoadFromEditor("haze"));
	shoot_effect_barrel_smoke1 = ObjectParticles::cast(LoadFromEditor("vfx_barrel_smoke_2"));
	shoot_effect_barrel_smoke2 = ObjectParticles::cast(LoadFromEditor("vfx_barrel_smoke_1"));
	//bullet_effect = ObjectParticles::cast(LoadFromEditor("explosion_fire_front"));


	sound_fire_loop = SoundSource::cast(LoadFromEditor("fire_sound"));
	sound_fire_up = SoundSource::cast(LoadFromEditor("sound_fire_up"));
	sound_fire_down = SoundSource::cast(LoadFromEditor("sound_fire_down"));

	sound_explosion = AmbientSource::create("sound/oil_tank_explosion.mp3");
	sound_explosion->setLoop(0);
	sound_button = AmbientSource::create("sound/switch_car.mp3");
	sound_button ->setLoop(0);

	LeftHand_Mesh_Glove = ObjectMeshSkinned::cast(LoadFromEditor("Human_LeftHand_Glove"));
	RightHand_Mesh_Glove = ObjectMeshSkinned::cast(LoadFromEditor("Human_RightHand_Glove"));


	LeftHand_Mesh_LeapMotion = ObjectMeshSkinned::cast(LoadFromEditor("Human_LeftHand_LeapMotion"));
	RightHand_Mesh_LeapMotion = ObjectMeshSkinned::cast(LoadFromEditor("Human_RightHand_LeapMotion"));
}
