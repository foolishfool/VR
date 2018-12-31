#include "XmlControl.h"
#include <UnigineGeodetics.h>


using namespace Math;

int demo_airspeed = 30;
const float CHANGE_INTERVAL = 0.8f; // the interval of timekeeping
float elapsed_time_xml = CHANGE_INTERVAL;	// current time left 

const float CHANGE_INTERVAL_COPY = 5.0f; // the interval of timekeeping
float elapsed_time_xml_copy = CHANGE_INTERVAL_COPY;	// current time left 

const float CHANGE_INTERVAL_POSITION = 3.0f; // the interval of timekeeping
float elapsed_time_position = CHANGE_INTERVAL_POSITION;	// current time left 

float windspeedratio = 20;


void XmlControl::init()
{
	winch = new Winch();
	winch->init();
	
	//used for demo mode position 6
	hotpoint6InitialPosition = Resource::get()->player_forest->getWorldPosition();

	initialBoxPosition = Resource::get()->vive_tracker_helicopter->getPosition();
	initialBoxRotation = Resource::get()->vive_tracker_helicopter->getRotation();
	seastate_value = 6;

	//load all the smoke_effect to smoke_effects
	loadSmokeEffects(bushfire_effects, "fire_smoke_effect");
	loadSmokeEffects(haze_effects, "smoke_haze");
	//loadEffectVelocity(bushfire_effects);

	textColor1 = vec4(0.007843, 0.952941, 0.247059, 1.000000);
	//dark green
	textColor2 = vec4(0.000000, 0.309216, 0.089804, 1.000000);

	// load sound settings from a file
	initial_sound();

	//load tracker files
	loadTracker();

	//old version
	weight_basket = LoadFromEditor("Basket_hook_linked");
	weight_strop = LoadFromEditor("Strop");
	weight_stretcher = LoadFromEditor("Stretcher");

}

void XmlControl::update() 
{
	winch->update();
	//key k reload position of helicopter 
	if (App::get()->getKeyState(107) == 1)
	{
		InteractiveVR::get()->vr->loadposition();
		loadPosition();
	}
	//key o  store the position of helicopter
	if (App::get()->getKeyState(111) == 1)
	{
		storePosition();
	}

	loadXML();

	//update local xml file, previous demand
	//updatelocalXML();
	updateAGLtext();
	updateWeight();
	updateMoon();
	///must before NVG as this will change the hgp pow of clouds and smoke, fire mat
	changeEnvironment();
	updateNVG();
	//control hellicopter sound
	change_sound();
	updateAGL();
	update_laser_flash_light();
	updateHosit();
	updateWaterDirection();
	//adjust the dbl of vr
	updateDBL();
	//key , .
	if (App::get()->getKeyState(44) == 1)

	{
		if (Sound::get()->getVolume() > 0.0f) Sound::get()->setVolume(Sound::get()->getVolume() - 0.05f);
	}

	if (App::get()->getKeyState(46) == 1)

	{
		if (Sound::get()->getVolume() != 1.0f) Sound::get()->setVolume(Sound::get()->getVolume() + 0.05f);
	}

		//demo version

	
	if (isdemo == 1)
	{

		Resource::get()->vive_tracker_reference->setWorldParent((Game::get()->getPlayer()->getNode()));
		Resource::get()->vive_tracker_0->setWorldParent((Game::get()->getPlayer()->getNode()));
		Resource::get()->vive_controller_0->setWorldParent((Game::get()->getPlayer()->getNode()));
		Resource::get()->vive_controller_1->setWorldParent((Game::get()->getPlayer()->getNode()));
		Resource::get()->vive_tracker_1->setWorldParent((Game::get()->getPlayer()->getNode()));

		updateHotpoint();
		Resource::get()->vive_tracker_reference->setEnabled(0);
		Resource::get()->vive_tracker_1->setEnabled(1);
		//Editor::get()->getNodeByName("bushfire_1")->setEnabled(0);

		if (hotpoint != currentPoint)
		{
			Resource::get()->vive_tracker_reference->setWorldTransform(InteractiveVR::get()->vive_tracker_transfrom);
		}
		currentPoint = hotpoint;
	}
	else
	{
		Game::get()->setPlayer(player_cigi->getPlayer());
		Resource::get()->vive_tracker_0->setWorldParent((Game::get()->getPlayer()->getNode()));
		Resource::get()->vive_tracker_reference->setWorldParent((Game::get()->getPlayer()->getNode()));
		Resource::get()->vive_controller_0->setWorldParent((Game::get()->getPlayer()->getNode()));
		Resource::get()->vive_controller_1->setWorldParent((Game::get()->getPlayer()->getNode()));
		Resource::get()->vive_tracker_1->setWorldParent((Game::get()->getPlayer()->getNode()));

		Resource::get()->vive_tracker_reference->setWorldPosition(Vec3(player_cigi->getWorldPosition()));
		///reset the local position to zero
		Resource::get()->vive_tracker_reference->setPosition(Vec3(0, 0, 0));
		vec3 euler = decomposeRotationZYX(mat3(player_cigi->getWorldRotation()));
		/// the sequence is very important, euler.z or -euler.z repdresent the direction of rotate along a certain axis
		///-euler.z yaw 
		/// the sequence of quat(0.0f, 0.0f, 1.0f, eulrer.z) * quat(0.0f, -1.0f, 0.0f, eulrer.y) * quat(1.0f, 0.0f, 0.0f, eulrer.x); represent the sequence of yaw pitch roll the most important!
		quat rot = quat(0.0f, 0.0f, 1.0f, euler.z) * quat(0.0f, -1.0f, 0.0f, -euler.y) * quat(1.0f, 0.0f, 0.0f, euler.x);

		// convert from X right, Y forward, Z up to X forward, Y right, Z down
		///just try numous times to find the number, still don't konw the reason
		rot *= quat(180, 90, 180);

		// apply rotation
		///vive_tracker roation should follow vive_tracker_reference, which will execute in VR update()
		Resource::get()->vive_tracker_reference->setWorldRotation(quat::IDENTITY * rot);
		Resource::get()->vive_tracker_reference->setEnabled(1);
		Resource::get()->vive_tracker_1->setEnabled(1);
	}
	
}

void XmlControl::loadPosition()
{
	if (xml->load("position.xml") == 0)
	{
		Log::error("local can't open \"%s\" file\n", "position.xml");
		return;
	}

	if (strcmp(xml->getName(), "Position"))
	{
		Log::error("bad definition file \"%s\"\n", "Position");
		return;
	}

	for (int i = 0; i < xml->getNumChildren(); i++)
	{
		XmlPtr category = xml->getChild(i);
		const char* name = category->getName();

		if (!strcmp(name, "PositionX"))
		{
			positionX = category->getData();
			x = positionX.getFloat();
		}
		if (!strcmp(name, "PositionY"))
		{
			positionY = category->getData();
			y = positionY.getFloat();
		}
		if (!strcmp(name, "PositionZ"))
		{
			positionZ = category->getData();
			z = positionZ.getFloat();
		}
		if (!strcmp(name, "RotationX"))
		{
			rotationX = category->getData();
			Rx = rotationX.getFloat();
		}
		if (!strcmp(name, "RotationY"))
		{
			rotationY = category->getData();
			Ry = rotationY.getFloat();
		}
		if (!strcmp(name, "RotationZ"))
		{
			rotationZ = category->getData();
			Rz = rotationZ.getFloat();
		}
	}

	Resource::get()->vive_tracker_helicopter->setPosition(Vec3(x, y, z));
	demoToCigi = true;

}

//store helicopter position
void XmlControl::storePosition()
{
	if (xml->load("position.xml") == 0)
	{
		Log::error("local can't open \"%s\" file\n", "position.xml");
		return;
	}

	if (!strcmp(xml->getName(), "Position"))
	{
		for (int i = 0; i < xml->getNumChildren(); i++)
		{
			XmlPtr category2 = xml->getChild(i);
			const char* name2 = category2->getName();
			if (!strcmp(name2, "PositionX"))
			{
				category2->setData(String::format("%f", Resource::get()->vive_tracker_helicopter->getPosition().x).get());
			}
			if (!strcmp(name2, "PositionY"))
			{
				category2->setData(String::format("%f", Resource::get()->vive_tracker_helicopter->getPosition().y).get());
			}
			if (!strcmp(name2, "PositionZ"))
			{
				category2->setData(String::format("%f", Resource::get()->vive_tracker_helicopter->getPosition().z).get());
			}
			if (!strcmp(name2, "RotationX"))
			{
				category2->setData(String::format("%f", Resource::get()->vive_tracker_helicopter->getRotation().x).get());
			}
			if (!strcmp(name2, "RotationY"))
			{
				category2->setData(String::format("%f", Resource::get()->vive_tracker_helicopter->getRotation().y).get());
			}
			if (!strcmp(name2, "RotationZ"))
			{
				category2->setData(String::format("%f", Resource::get()->vive_tracker_helicopter->getRotation().z).get());
			}
		}
		xml->save("position.xml", 0);
	}
}

vec3 XmlControl::getHellicopterDirection(float value)
{
	float x1 = Resource::get()->door_handle_3->getWorldPosition().x;
	float y1 = Resource::get()->door_handle_3->getWorldPosition().y;
	float z1 = Resource::get()->door_handle_3->getWorldPosition().z;

	float x2 = Resource::get()->door_handle_2->getWorldPosition().x;
	float y2 = Resource::get()->door_handle_2->getWorldPosition().y;
	float z2 = Resource::get()->door_handle_2->getWorldPosition().z;

	return vec3((x2 - x1)*value, (y2 - y1)*value, (z2 - z1)*value);

}

void XmlControl::loadTracker()
{
	Engine::get()->runWorldFunction(Variable("TrackerWrapper::init")); // initialize track
	Engine::get()->runWorldFunction(Variable("TrackerWrapper::load"), Variable("/tracks/sun_rotation.track")); // loading track
	Engine::get()->runWorldFunction(Variable("TrackerWrapper::load"), Variable("/tracks/weather.track"));
	Engine::get()->runWorldFunction(Variable("TrackerWrapper::load"), Variable("/tracks/day_cycle_rainy.track"));
	Engine::get()->runWorldFunction(Variable("TrackerWrapper::load"), Variable("/tracks/clouds.track"));
}

void XmlControl::initial_sound()
{
	Sound::get()->loadSettings("/sound/sound_settings.settings");
	sound = AmbientSource::create("/sound/xai_smjet03.wav", 0);
	sound->setLoop(1);
	sound->play();
}

void XmlControl::loadXML()
{

	if (elapsed_time_xml <0.0f)
	{
		//"//DESKTOP-H8ULHOU/SharedFolder/InstructorInterfaceData.xml"
		int i = xml->load("//DESKTOP-H8ULHOU/SharedFolder/InstructorInterfaceData.xml");
		if (i == 0)
		{
		//	  Log::error("instructor can't open \"%s\" file\n", "InstructorInterfaceData");
			if (xml->load("InstructorInterfaceData.xml") == 0)
			{
				//Log::error("local can't open \"%s\" file\n", "InstructorInterfaceData");
				return;
			}
		}

		if (strcmp(xml->getName(), "Root"))
		{
			Log::error("bad definition file \"%s\"\n", "Root");
			return;
		}

		for (int i = 0; i < xml->getNumChildren(); i++)
		{
			XmlPtr category = xml->getChild(i);
			const char* name = category->getName();

		//if (!strcmp(name, "CameraNumber"))
		//{
		//	CameraNumber = category->getData();
		//	camera_number = CameraNumber.getInt();
		//}
			if (!strcmp(name, "WeightNumber"))
			{
				currentWeight = category->getData();
				weightNumber = currentWeight.getInt();
			}

			if (!strcmp(name, "HoistFailureNumber"))
			{
				HoistFailureNumber = category->getData();
				hoistfailure_number = HoistFailureNumber.getInt();
			}
			if (!strcmp(name, "PointOfInterest"))
			{
				currentHotpoint = category->getData();
				hotpoint = currentHotpoint.getInt();
			}
			if (!strcmp(name, "Hotpoint6PositionReset"))
			{
				hotpoint6Rest = category->getData();
				hotpointreset = hotpoint6Rest.getInt();
			}

		//	if (!strcmp(name, "LifeRaftVisible"))
		//	{
		//		LifeRaftVisible = category->getData();
		//		liferaft_number = LifeRaftVisible.getInt();
		//	}

		//	if (!strcmp(name, "GroundCasualtyVisible"))
		//	{
		//		GroundCasualtyVisible = category->getData();
		//		groundcasualty_number = GroundCasualtyVisible.getInt();
		//	}


		//	if (!strcmp(name, "SwimmerVisible"))
		//	{
		//		SwimmerVisible = category->getData();
		//		swimmer_number = SwimmerVisible.getInt();
		//	}
		//
			if (!strcmp(name, "FireVisible"))
			{
				FireVisable = category->getData();
				isfireshow = FireVisable.getInt();
			}
			if (!strcmp(name, "FloodWaterLevel"))
			{
				FloodHeight = category->getData();
				floodvalue = FloodHeight.getFloat() / 1000;
			}


			if (!strcmp(name, "DistanceHUDEnabled"))
			{
				hudDTR = category->getData();
				ishudDTR = hudDTR.getInt();
			}

			if (!strcmp(name, "HeightHUDEnabled"))
			{
				hudAGL = category->getData();
				ishudAGL = hudAGL.getInt();
			}

			if (!strcmp(name, "SoundEnable"))
			{
				SoundStop = category->getData();
				isSoundEnable = SoundStop.getInt();
			}

			if (!strcmp(name, "CabinLightsEnabled"))
			{
				GreenLight = category->getData();
				isGreenlight = GreenLight.getInt();
			}
			if (!strcmp(name, "DemoModeEnable"))
			{
				demooption = category->getData();
				isdemo = demooption.getInt();
			}
			if (!strcmp(name, "WeatherTimeOfDay"))
			{
				currentTime = category->getData();
				timevalue = currentTime.getFloat() / 1000.0f;
			}
			if (!strcmp(name, "Haze"))
			{
				HazeData = category->getData();
				hazevalue = HazeData.getFloat() / 1000.0f;
			}

			if (!strcmp(name, "SeaState"))
			{
				SeaStateRange = category->getData();
				seastate_value = SeaStateRange.getInt();
			}
			if (!strcmp(name, "WeatherWindDirectionDeg"))
			{
				WaterDirection = category->getData();
				water_direction_value = 270 - WaterDirection.getFloat();
			}
			if (!strcmp(name, "WindSpeed"))
			{
				currentWind = category->getData();
				windvalue = currentWind.getFloat()  / 1000.0f;
			}


			if (!strcmp(name, "WeatherWindDirectionX"))
			{
				NorthSouth = category->getData();
				northsouthvalue = NorthSouth.getFloat();
			}
			if (!strcmp(name, "WeatherWindDirectionY"))
			{
				WestEast = category->getData();
				westeastvalue = WestEast.getFloat();
			}

			if (!strcmp(name, "NVGModeEnabled"))
			{
				nightvision = category->getData();
				isNightVision = nightvision.getInt();
			}
			if (!strcmp(name, "NVGWhiteEnabled"))
			{
				whitenvg = category->getData();
				isWhiteNVG = whitenvg.getInt();
			}
			if (!strcmp(name, "NVGCurrentBatteryFailure"))
			{
				darkvision = category->getData();
				currentFailure = darkvision.getInt();
			}

			if (!strcmp(name, "NVGCurrentTubeFailure"))
			{
				NVGCurrentTubeFailure = category->getData();
				NVGCurrentTubeFailureValue = NVGCurrentTubeFailure.getInt();
			}

			if (!strcmp(name, "NVGCurrentMoonState"))
			{
				Lunarvalue = category->getData();
				moonValue = Lunarvalue.getInt();
			}
			if (!strcmp(name, "WeatherVisibility"))
			{
				currentClouds = category->getData();
				cloudsvalue = currentClouds.getFloat() / 100.0f;
			}

			if (!strcmp(name, "DistanceBetweenLenses"))
			{
				DistanceBetweenLenses = category->getData();
				distance_lenses = DistanceBetweenLenses.getFloat() / 1000.0f;
			}
		//	if (!strcmp(name, "Rain"))
		//	{
		//		Rain = category->getData();
		//		rainvalue = Rain.getFloat() / 1000.0f;
		//	}
		}
		//reset timekeeping
		elapsed_time_xml = CHANGE_INTERVAL;
	}

	elapsed_time_xml = elapsed_time_xml - Game::get()->getIFps();

}

void XmlControl::updatelocalXML()
{
	if (elapsed_time_xml_copy < 0.0f)
	{

		if (xml2->load("InstructorInterfaceData.xml") == 0)
		{
			Log::error("local can't open \"%s\" file\n", "InstructorInterfaceData.xml.xml");
			elapsed_time_xml_copy = CHANGE_INTERVAL_COPY;
			return;
		}

		if (!strcmp(xml2->getName(), "Root"))
		{
			for (int i = 0; i < xml2->getNumChildren(); i++)
			{
				XmlPtr category2 = xml2->getChild(i);
				const char* name2 = category2->getName();
				if (!strcmp(name2, "CameraNumber"))
				{
					category2->setData(String::format("%d", camera_number).get());
				}
				if (!strcmp(name2, "WeightNumber"))
				{
					category2->setData(String::format("%d", weightNumber).get());
				}
				if (!strcmp(name2, "HoistFailureNumber"))
				{
					category2->setData(String::format("%d", hoistfailure_number).get());
				}
				if (!strcmp(name2, "PointOfInterest"))
				{
					category2->setData(String::format("%d", hotpoint).get());
				}
				if (!strcmp(name2, "Hotpoint6PositionReset"))
				{
					category2->setData(String::format("%d", hotpointreset).get());
				}
				if (!strcmp(name2, "LifeRaftVisible"))
				{
					category2->setData(String::format("%d", liferaft_number).get());
				}
				if (!strcmp(name2, "GroundCasualtyVisible"))
				{
					category2->setData(String::format("%d", groundcasualty_number).get());
				}
				if (!strcmp(name2, "SwimmerVisible"))
				{
					category2->setData(String::format("%d", swimmer_number).get());
				}
				if (!strcmp(name2, "FireVisible"))
				{
					category2->setData(String::format("%d", isfireshow).get());
				}
				if (!strcmp(name2, "FloodWaterLevel"))
				{
					category2->setData(String::format("%f", floodvalue * 1000).get());
				}
				if (!strcmp(name2, "DistanceHUDEnabled"))
				{
					category2->setData(String::format("%d", ishudDTR).get());
				}
				if (!strcmp(name2, "HeightHUDEnabled"))
				{
					category2->setData(String::format("%d", ishudAGL).get());
				}
				if (!strcmp(name2, "SoundEnable"))
				{
					category2->setData(String::format("%d", isSoundEnable).get());
				}

				if (!strcmp(name2, "CabinLightsEnabled"))
				{
					category2->setData(String::format("%d", isGreenlight).get());
				}
				if (!strcmp(name2, "DemoModeEnable"))
				{
					category2->setData(String::format("%d", isdemo).get());
				}
				if (!strcmp(name2, "WeatherTimeOfDay"))
				{
					category2->setData(String::format("%f",		timevalue * 1000).get());
				}
				if (!strcmp(name2, "WeatherVisibility"))
				{
					category2->setData(String::format("%f", cloudsvalue * 1000).get());
				}
				if (!strcmp(name2, "SeaState"))
				{
					category2->setData(String::format("%d", seastate_value).get());
				}
				if (!strcmp(name2, "WindSpeed"))
				{
					category2->setData(String::format("%f", windvalue * 1000).get());
				}
				if (!strcmp(name2, "WeatherWindDirectionX"))
				{
					category2->setData(String::format("%f", northsouthvalue).get());
				}
				if (!strcmp(name2, "WeatherWindDirectionY"))
				{
					category2->setData(String::format("%f", westeastvalue).get());
				}
				if (!strcmp(name2, "NVGModeEnabled"))
				{
					category2->setData(String::format("%d", isNightVision).get());
				}
				if (!strcmp(name2, "NVGWhiteEnabled"))
				{
					category2->setData(String::format("%d", isWhiteNVG).get());
				}
				if (!strcmp(name2, "NVGCurrentBatteryFailure"))
				{
					category2->setData(String::format("%d", currentFailure).get());
				}
				if (!strcmp(name2, "NVGCurrentTubeFailure"))
				{
					category2->setData(String::format("%d", NVGCurrentTubeFailureValue).get());
				}
				if (!strcmp(name2, "NVGCurrentMoonState"))
				{
					category2->setData(String::format("%d", moonValue).get());
				}
			}
			xml2->save("InstructorInterfaceData.xml", 0);
		}
		//reset timekeeping
		elapsed_time_xml_copy = CHANGE_INTERVAL_COPY;
	}
	elapsed_time_xml_copy = elapsed_time_xml_copy - Game::get()->getIFps();
}


void XmlControl::storeTerrainHeight()
{
	if (elapsed_time_xml <0.0f)
	{
		if (xml->load("//DESKTOP-VVRAEK7/SharedFolder/height.xml") == 0)
		{
			//		Log::error("local can't open \"%s\" file\n", "height.xml");
			return;
		}

		if (!strcmp(xml->getName(), "Values"))
		{
			for (int i = 0; i < xml->getNumChildren(); i++)
			{
				XmlPtr category2 = xml->getChild(i);
				const char* name2 = category2->getName();
				if (!strcmp(name2, "HeightAboveTerrain"))
				{
					category2->setData(String::format("%f", Helicopter::get()->landHeight).get());
				}
			}
			xml->save("//DESKTOP-VVRAEK7/SharedFolder/height.xml", 0);
		}
		//reset timekeeping
		elapsed_time_xml = CHANGE_INTERVAL;
	}
	elapsed_time_xml = elapsed_time_xml - Game::get()->getIFps();
}


void XmlControl::updateHotpoint()
{

	switch (hotpoint)
	{
	case 1:
		Game::get()->setPlayer(Resource::get()->player_helicopterPad->getPlayer());
		break;
	case 2:
		Game::get()->setPlayer(Resource::get()->player_boat->getPlayer());
		break;
	case 3:
		Game::get()->setPlayer(Resource::get()->player_bushfire->getPlayer());
		break;
	case 4:
		Game::get()->setPlayer(Resource::get()->player_factory->getPlayer());
		break;
	case 5:
		///cannot use player_flir, as player_flir is the child of vive_tracker, which cannot be applied to the child of player_flir
	//	Game::get()->setPlayer(Resource::get()->player_flir_reference->getPlayer());
		break;
	case 6:
		Game::get()->setPlayer(Resource::get()->player_forest->getPlayer());
		Resource::get()->vive_tracker_0->setWorldParent(Resource::get()->player_forest->getPlayer()->getNode());
		Resource::get()->player_forest->setWorldPosition(Vec3(Resource::get()->player_forest->getWorldPosition().x + getHellicopterDirection(demo_airspeed).x * Game::get()->getIFps(), Resource::get()->player_forest->getWorldPosition().y + getHellicopterDirection(demo_airspeed).y * Game::get()->getIFps(), Resource::get()->player_forest->getWorldPosition().z + getHellicopterDirection(demo_airspeed).z * Game::get()->getIFps()));
		break;
	default:
		break;
	}

	if (hotpointreset == 1 && hotpoint == 6)
	{
		Resource::get()->player_forest->setWorldPosition(hotpoint6InitialPosition);
	}

	if (hotpoint != 6)
	{
		Resource::get()->player_forest->getPlayer()->getNode()->removeWorldChild(Resource::get()->vive_tracker_0);
	}

}

void XmlControl::updateHosit()
{

	switch (hoistfailure_number)
	{
	case 1:
		Resource::get()->caution_mat->setParameterSlider("emission_scale",0.0);
		Resource::get()->overheat_mat->setParameterSlider("emission_scale", 4.0);
		break;
	case 2:
		Resource::get()->caution_mat->setParameterSlider("emission_scale", 3.0);
		Resource::get()->overheat_mat->setParameterSlider("emission_scale", 0.0);
		break;
	case 3:
		Resource::get()->caution_mat->setParameterSlider("emission_scale", 0.0);
		Resource::get()->overheat_mat->setParameterSlider("emission_scale", 0.0);
		break;
	case 4:
		Resource::get()->caution_mat->setParameterSlider("emission_scale", 0.0);
		Resource::get()->overheat_mat->setParameterSlider("emission_scale", 0.0);
		break;
	default:
		Resource::get()->caution_mat->setParameterSlider("emission_scale", 0.0);
		Resource::get()->overheat_mat->setParameterSlider("emission_scale", 0.0);
		break;
	}

}
void XmlControl::updateAGLtext()
{

	if (fabs(timevalue - 0.5) < 0.35)
	{
		Resource::get()->ui_text1->setTextColor(textColor1);
		Resource::get()->ui_text2->setTextColor(textColor1);
		Resource::get()->ui_text3->setTextColor(textColor1);
		Resource::get()->ui_text4->setTextColor(textColor1);
	}
	else
	{
		Resource::get()->ui_text1->setTextColor(textColor2);
		Resource::get()->ui_text2->setTextColor(textColor2);
		Resource::get()->ui_text3->setTextColor(textColor2);
		Resource::get()->ui_text4->setTextColor(textColor2);
	}


	if (isBatteryFailureMode)
	{
		Resource::get()->ui_text1->setEnabled(0);
		Resource::get()->ui_text2->setEnabled(0);
		Resource::get()->ui_text3->setEnabled(0);
		Resource::get()->ui_text4->setEnabled(0);
	}
	else
	{
		Resource::get()->ui_text1->setEnabled(1);
		Resource::get()->ui_text2->setEnabled(1);
		Resource::get()->ui_text3->setEnabled(1);
		Resource::get()->ui_text4->setEnabled(1);
	}
}

void XmlControl::updateWeight()
{
	//dynamic version
	/*
	switch (weightNumber)
	{
	case 1:
		winch->ropes.get(winch->ropes.size()-1)->basket_node->setEnabled(0);
		winch->ropes.get(winch->ropes.size()-1)->weight_strop_node->setEnabled(0);
		winch->ropes.get(winch->ropes.size()-1)->weight_stretcher_node->setEnabled(0);
		break;
	case 2:
		winch->ropes.get(winch->ropes.size()-1)->basket_node->setEnabled(1);
		winch->ropes.get(winch->ropes.size() - 1)->basket_node->getObjectBodyRigid()->getShape(0)->setMass(1);
		winch->ropes.get(winch->ropes.size()-1)->weight_strop_node->setEnabled(0);
		winch->ropes.get(winch->ropes.size()-1)->weight_stretcher_node->setEnabled(0);
		break;
	case 3:
		winch->ropes.get(winch->ropes.size()-1)->weight_strop_node->setEnabled(1);
		winch->ropes.get(winch->ropes.size() - 1)->weight_strop_node->getObjectBodyRigid()->getShape(0)->setMass(1);
		winch->ropes.get(winch->ropes.size()-1)->basket_node->setEnabled(0);
		winch->ropes.get(winch->ropes.size()-1)->weight_stretcher_node->setEnabled(0);
		break;
	case 4:
		winch->ropes.get(winch->ropes.size()-1)->weight_stretcher_node->setEnabled(1);
		winch->ropes.get(winch->ropes.size() - 1)->weight_stretcher_node->getObjectBodyRigid()->getShape(0)->setMass(1);
		winch->ropes.get(winch->ropes.size()-1)->basket_node->setEnabled(0);
		winch->ropes.get(winch->ropes.size()-1)->weight_strop_node->setEnabled(0);
		break;
	default:
		break;
	}

	*/
	switch (weightNumber)
	{
	case 1:
		weight_basket->setEnabled(0);
		weight_strop->setEnabled(0);
		weight_stretcher->setEnabled(0);
		break;
	case 2:
		weight_basket->setEnabled(1);
		weight_strop->setEnabled(0);
		weight_stretcher->setEnabled(0);
		break;
	case 3:
		weight_strop->setEnabled(1);
		weight_basket->setEnabled(0);
		weight_stretcher->setEnabled(0);
		break;
	case 4:
		weight_stretcher->setEnabled(1);
		weight_basket->setEnabled(0);
		weight_strop->setEnabled(0);
		break;
	default:
		break;
	}

}

void XmlControl::updateWaterDirection()
{
	Resource::get()->water->setRotation(quat(0,0,water_direction_value));
}

void XmlControl::updateNVG()
{
	if (timevalue <= 0.3f || timevalue >= 0.7f)
	{
	//	stars->setEnabled(1);
		Resource::get()->smoke_canister_mat->setParameterSlider(Resource::get()->smoke_canister_mat->findParameter("emission_scale"), 0.2f);
		if (Resource::get()->torch_mat->getState("emission") == 1)
		{
			Resource::get()->torch_mat->setParameterSlider("emission", 1.624 );
		}
		isNight = true;
	}
	else
	{
	//	stars->setEnabled(0);
		Resource::get()->smoke_canister_mat->setParameterSlider(Resource::get()->smoke_canister_mat->findParameter("emission_scale"), 0.0f);
		if (Resource::get()->torch_mat->getState("emission") == 1)
		{
			Resource::get()->torch_mat->setParameterSlider("emission", 1.2);
		}
		isNight = false;
	}

	//getcurrent haze_emission_scale
	float haze_emission_scale = Resource::get()->bushsmoke_huge_1->getParameterSlider(Resource::get()->bushsmoke_huge_1->findParameter("emission_scale"));

	if (isNightVision)
	{
		isNVG = true;
		//enable blur
		Render::get()->setDOF(1);
		//when NVG cabin light should be darker
		Resource::get()->greenlight_mat->setParameterSlider(Resource::get()->greenlight_mat->findParameter("emission_scale"), 1.20f);
		Resource::get()->clouds->getMaterial(0)->setParameterSlider("hgp_brightness", -1);
		Resource::get()->sun->setEnabled(0);
		//when NVG bushfire smoke should be darker
		Resource::get()->bushsmoke_mat1->setParameterSlider(Resource::get()->bushsmoke_mat1->findParameter("emission_scale"), 0.00f);
		Resource::get()->bushfire_mat->setParameterSlider(Resource::get()->bushfire_mat->findParameter("emission_scale"), 200);
		Resource::get()->bushfire_mat_huge->setParameterSlider(Resource::get()->bushfire_mat_huge->findParameter("emission_scale"), 15);
		Resource::get()->bushfire_mat_middle->setParameterSlider(Resource::get()->bushfire_mat_middle->findParameter("emission_scale"), 5);
		Resource::get()->bushsmoke_huge_0->setParameterSlider(Resource::get()->bushsmoke_huge_0->findParameter("emission_scale"), 0.00);
		Resource::get()->bushsmoke_huge_1->setParameterSlider(Resource::get()->bushsmoke_huge_1->findParameter("emission_scale"), 0.02);
		Resource::get()->haze_mat->setParameterSlider(Resource::get()->haze_mat->findParameter("emission_scale"), 0.001f);
		Resource::get()->haze_mat2->setParameterSlider(Resource::get()->haze_mat2->findParameter("emission_scale"), 0.001f);
		Resource::get()->haze_volume_mat->setParameterSlider(Resource::get()->haze_volume_mat->findParameter("volume_power"), 4.0f);
		//	Resource::get()->haze_mat2->setParameterSlider(Resource::get()->bushfire_mat->findParameter("diffuse_scale"), 40.0f);
		Resource::get()->star_mat->setParameterSlider(Resource::get()->star_mat->findParameter("emission_scale"), 0.1f);
		Resource::get()->gun_smoke_mat_1->setParameterSlider(Resource::get()->gun_smoke_mat_1->findParameter("emission_scale"), 0.00f);
		Resource::get()->gun_smoke_mat_2->setParameterSlider(Resource::get()->gun_smoke_mat_2->findParameter("emission_scale"), 0.00f);
		Resource::get()->gun_smoke_mat_1->setParameter("diffuse_color", vec4(0.349020, 0.349020, 0.349020, 0.192157));

		if (isWhiteNVG == 0)
		{
			switch (NVGCurrentTubeFailureValue)
			{
			case 1:
				Game::get()->getPlayer()->setPostMaterials("vr_post_filter_selection,post_filter_nightmarker,post_filter_Brightspots");
				break;	
				//2 honeycone changed in 	viewport->renderStereo(left_camera, right_camera, "post_stereo_separate_honeycone");	
				//3 edge				
			case 4:
				Game::get()->getPlayer()->setPostMaterials("vr_post_filter_selection,post_filter_Blackspots,post_filter_nightmarker");
				break;
				//5 chickenwire
				//6 shading
			case 7:
				Game::get()->getPlayer()->setPostMaterials("vr_post_filter_selection,post_sensor_green");
				break;
			default:
				Game::get()->getPlayer()->setPostMaterials("vr_post_filter_selection,post_filter_nightmarker");
				break;
			}
		}
		else
		{
			switch (NVGCurrentTubeFailureValue)
			{
			case 1:
				Game::get()->getPlayer()->setPostMaterials("vr_post_filter_selection,post_filter_nightmarker_whitenvg,post_filter_Brightspots");
				break;
				//2 honeycone changed in 	viewport->renderStereo(left_camera, right_camera, "post_stereo_separate_honeycone");	
				//3 edge				
			case 4:
				Game::get()->getPlayer()->setPostMaterials("vr_post_filter_selection,post_filter_Blackspots,post_filter_nightmarker_whitenvg");
				break;
				//5 chickenwire
				//6 shading
			default:
				Game::get()->getPlayer()->setPostMaterials("vr_post_filter_selection,post_filter_nightmarker_whitenvg");
				break;
			}
		}		
	}
	else if (!isNightVision)
	{
		isNVG = false;

		Render::get()->setDOF(0);
		Resource::get()->clouds->getMaterial(0)->setParameterSlider("hgp_brightness", 1);
		//land->getMaterial(0)->setParameter("albedo_color", vec4(1, 1, 1, 1));
		//land->getMaterial(0)->setParameterSlider("occlusion_on_albedo", 0.85);
		Resource::get()->sun->setEnabled(1);

		Resource::get()->greenlight_mat->setParameterSlider(Resource::get()->greenlight_mat->findParameter("emission_scale"), 1.70f);
		Resource::get()->bushsmoke_huge_0->setParameterSlider(Resource::get()->bushsmoke_huge_0->findParameter("emission_scale"), 0.02);
		Resource::get()->bushsmoke_huge_1->setParameterSlider(Resource::get()->bushsmoke_huge_1->findParameter("emission_scale"), haze_emission_scale);
		Resource::get()->star_mat->setParameterSlider(Resource::get()->star_mat->findParameter("emission_scale"), 0.6f);
		Resource::get()->gun_smoke_mat_1->setParameterSlider(Resource::get()->star_mat->findParameter("emission_scale"), 0.064f);
		Resource::get()->gun_smoke_mat_2->setParameterSlider(Resource::get()->star_mat->findParameter("emission_scale"), 0.064f);
		Resource::get()->gun_smoke_mat_1->setParameter("diffuse_color", vec4(0.549020, 0.549020, 0.549020, 0.392157));
		Resource::get()->haze_volume_mat->setParameterSlider(Resource::get()->haze_volume_mat->findParameter("volume_power"), 2.0f);
		if (timevalue <= 0.3f || timevalue >= 0.7f)
		{
			Resource::get()->clouds->getMaterial(0)->setParameterSlider("hgp_brightness", 0);
		}
		else
			Resource::get()->clouds->getMaterial(0)->setParameterSlider("hgp_brightness", 1);
	}

	switch (currentFailure)
	{
	case 1:
		break;
	case 2:
		break;
	case 3:
		isBatteryFailureMode = true;
		break;
	default:
		break;
	}

	if (!currentFailure)
	{
		isBatteryFailureMode = false;
		if (!isNightVision)
		{
			Game::get()->getPlayer()->setPostMaterials("vr_post_filter_selection");
		}

	}

}


void XmlControl::update_laser_flash_light()
{
	if (isNight)
	{
		InteractiveVR::get()->allShowRay(InteractiveVR::get()->laserpoints);
		InteractiveVR::get()->allClear(InteractiveVR::get()->flashlights);
	}
	else
	{
		//turn off ray and dim flashlight
		InteractiveVR::get()->allHideRay(InteractiveVR::get()->laserpoints);
		InteractiveVR::get()->allDim(InteractiveVR::get()->flashlights);
	}

	if (isNVG)
	{
		InteractiveVR::get()->allDim(InteractiveVR::get()->flashlights);

	}
	else
	{
		InteractiveVR::get()->allClear(InteractiveVR::get()->flashlights);
	}

}

void XmlControl::updateMoon()
{
	switch (moonValue)
	{
	case 1:
		Resource::get()->moon->setIntensity(0.15);
		break;
	case 2:
		Resource::get()->moon->setIntensity(0.45);
		break;
	case 3:
		Resource::get()->moon->setIntensity(1.5);
		break;
	default:
		Resource::get()->moon->setIntensity(1.0);
		break;
	}

}

void XmlControl::changeEnvironment()
{
	//changeRain();
	/// must before change weather clouds as wave_change will change the weather
	beaufort_changed();
	change_weather_and_time();
	change_clouds();
	//change wind speed
	changewindSpeed();
	openGreenLight();
	changefloodHieght();
	changewindDirection();
	showFire();
}

void XmlControl::beaufort_changed()
{
	if (Resource::get()->water_global)
	{
		Resource::get()->water_global->setMaterial(String::format("water_global_beaufort_inherit_%d", seastate_value).get(), 0);
	}
}

void XmlControl::change_weather_and_time()
{
	day_cycle_time = clamp(timevalue * 24, 0.0f, 24.0f);
	//change time
	Engine::get()->runWorldFunction(Variable("TrackerWrapper::blend"), Variable(0), Variable(day_cycle_time), Variable(1.0));
	Engine::get()->runWorldFunction(Variable("TrackerWrapper::blend"), Variable(1), Variable(hazevalue), Variable(1.0));
}



void XmlControl::change_clouds()
{
	Engine::get()->runWorldFunction(Variable("TrackerWrapper::blend"), Variable(3), Variable(cloudsvalue), Variable(1.0));
}



void XmlControl::changewindSpeed()
{
	MaterialPtr mat = Materials::get()->findMaterial("clouds_base_0");
	if (mat)
	{
		mat->setParameterSlider(mat->findParameter("wind_x"), -westeastvalue * windvalue *  windspeedratio);
		mat->setParameterSlider(mat->findParameter("wind_y"), -northsouthvalue * windvalue * windspeedratio);
	}
}

void XmlControl::openGreenLight()
{
	if (isGreenlight)
	{
		Resource::get()->greenlight->setEnabled(1);
	}
	else
	{
		Resource::get()->greenlight->setEnabled(0);
	}
}

void XmlControl::changefloodHieght()
{
	float floodheight = lerp_clamped(-1, 90, floodvalue);
	Resource::get()->flood->setWorldPosition(Vec3(Resource::get()->flood->getWorldPosition().x, Resource::get()->flood->getWorldPosition().y, floodheight));
	//the color of flood is different with ocean so needs to be hidden when there is no flood
	if (floodheight < 5)
	{
		Resource::get()->flood->setEnabled(0);
		Helicopter::get()->waterHeight = Resource::get()->water->getWorldPosition().z;
		Resource::get()->water->setEnabled(1);
	}
	else
	{
		Resource::get()->flood->setEnabled(1);
		Resource::get()->water->setEnabled(0);
		Helicopter::get()->waterHeight = Resource::get()->flood->getWorldPosition().z;
	}

}

void XmlControl::changewindDirection()
{
	seteffectGravity(bushfire_effects, 4, 4, 1);
	seteffectGravity(haze_effects, 10, 10, 10);
	//setforce direction of the haze
	setParticleForce(haze_effects,80);
	//seteffectSpawnRate(haze_effects, 5, 2);
	//seteffectVelocity(bushfire_effects,-0.5);
	//seteffectGravity(InteractiveVR::get()->smoke_effects, 1.0f, 1, 1);
	//seteffectSpawnRate(InteractiveVR::get()->smoke_effects, 1.0f, 25);

}



//different particle use different ratio
//zGravityis the z gravity when there is no wind 
void XmlControl::seteffectGravity(Map<int, ObjectParticlesPtr> effects, float ratio, float normalGravity, float zGravity)
{
	for (auto it = effects.begin(); it != effects.end(); ++it)
	{
		it->data->setGravity(vec3(westeastvalue* windvalue * ratio, northsouthvalue* windvalue *ratio, normalGravity));

		if (westeastvalue == 0 && northsouthvalue == 0)
		{
			it->data->setGravity(vec3(0, 0, zGravity));
		
		}
	}
}

void XmlControl::setParticleForce(Map<int, ObjectParticlesPtr> effects, float ratio)
{
	for (auto it = effects.begin(); it != effects.end(); ++it)
	{
		dvec3 position = it->data->getForceTransform(1).getTranslate();
		quat rotation = it->data->getForceTransform(1).getRotate();
		vec4  new_position = vec4(ratio * westeastvalue , ratio * northsouthvalue , position.z, 1);
		mat4 transfrom = composeTransform(transfrom, new_position, rotation);
		it->data->setForceTransform(1,Mat4(transfrom));
	}
}

//different particle use different ratio change the spawnrate
//initialNumber is the initial spawnrate of particle
void XmlControl::seteffectSpawnRate(Map<int, ObjectParticlesPtr> effects, float ratio, int initialNumber)
{
	for (auto it = effects.begin(); it != effects.end(); ++it)
	{
		it->data->setSpawnRate(fabsf(windvalue) * ratio + initialNumber);
	}
}
//storeInitialVelocity
void XmlControl::loadEffectVelocity(Map<int, ObjectParticlesPtr> effects)
{
	for (auto it = effects.begin(); it != effects.end(); ++it)
	{
		bushfire_effects_velocities.append(it->key,it->data->getVelocityMean());
	}
}

void XmlControl::seteffectVelocity(Map<int, ObjectParticlesPtr> effects, float ratio)
{
	for (auto it = effects.begin(); it != effects.end(); ++it)
	{
		//it->data->setVelocity( windvalue *ratio + 1), 0);
		//for (auto  vel = bushfire_effects_velocities.begin(); vel != bushfire_effects_velocities.end(); ++vel)
		//{
		//	if (it->key == vel->key)
		//	{
		//		//when the windvalue is 0, the speed is initial values
		//		it->data->setVelocity(vel->data * (windvalue *ratio + 1) , 0);
		//	}
		//}
	}
}

int XmlControl::checkSmokeEffect(const PropertyPtr &prop, const char * property_name)
{
	if (std::string(prop->getName()) == property_name)
		return 1;
}

//update the Map of smoke_effects
void XmlControl::updateSmokeEffects(NodePtr node, Map<int, ObjectParticlesPtr> &smoke_effects, const char * property_name)
{
	PropertyPtr prop = node->getProperty();
	if (prop)
	{
		if (checkSmokeEffect(prop, property_name) == 1)
		{
			if (smoke_effects.size() == 0)
			{
				smoke_effects.append(node->getID(), ObjectParticles::cast(node));
			}
			else
			{
				for (auto it = smoke_effects.begin(); it != smoke_effects.end(); ++it)
				{
					//if already has the node in the map
					if (it->key == node->getID())
						return;
					else
					{
						smoke_effects.append(node->getID(), ObjectParticles::cast(node));
					}
				}
			}
		}
	}

	for (int i = 0; i < node->getNumChildren(); i++)
	{
		updateSmokeEffects(node->getChild(i), smoke_effects, property_name);
	}
}


void XmlControl::loadSmokeEffects(Map<int, ObjectParticlesPtr> &smoke_effects, const char * property_name)
{
	Editor *editor = Editor::get();
	for (int i = 0; i < editor->getNumNodes(); i++)
	{
		NodePtr node = editor->getNode(i);
		if (!node->getParent()) // if it is root object
		{
			updateSmokeEffects(node, smoke_effects, property_name);
			for (int i = 0; i < node->getNumChildren(); i++)
			{
				updateSmokeEffects(node->getChild(i), smoke_effects, property_name);
			}
		}

	}
}

void XmlControl::showFire()
{
	if (isfireshow == 1)
	{
		Resource::get()->bushfire->setEnabled(1);
	}
	else
	{
		Resource::get()->bushfire->setEnabled(0);
	}
}

void XmlControl::change_sound() {

	if (isSoundEnable == 0)
	{
		sound->stop();
	}
	else
	{
		sound->play();
	}
}

void XmlControl::updateAGL()
{
	//hide the ui in hud
	if (ishudAGL == 0)
	{
		Resource::get()->hudAGL_text->setEnabled(0);
	}
	else
	{
		Resource::get()->hudAGL_text->setEnabled(1);
	}

	if (ishudDTR == 0)
	{
		Resource::get()->hudDTR_text->setEnabled(0);
	}
	else
	{
		Resource::get()->hudDTR_text->setEnabled(1);
	}
}


void XmlControl::updateDBL()
{
	MaterialPtr mat = Materials::get()->findMaterial("post_stereo_separate");
	MaterialPtr mat3 = Materials::get()->findMaterial("post_stereo_leftdark");
	MaterialPtr mat4 = Materials::get()->findMaterial("post_stereo_rightdark");
	MaterialPtr mat5 = Materials::get()->findMaterial("post_stereo_separate_honeycone_left");
	MaterialPtr mat6 = Materials::get()->findMaterial("post_stereo_separate_honeycone_right");
	MaterialPtr mat7 = Materials::get()->findMaterial("post_stereo_separate_honeycone");
	MaterialPtr mat8 = Materials::get()->findMaterial("post_stereo_separate_edgeglow_left");
	MaterialPtr mat9 = Materials::get()->findMaterial("post_stereo_separate_edgeglow_right");
	MaterialPtr mat10 = Materials::get()->findMaterial("post_stereo_separate_edgeglow");
	MaterialPtr mat11 = Materials::get()->findMaterial("post_stereo_separate_chickenwire_left");
	MaterialPtr mat12= Materials::get()->findMaterial("post_stereo_separate_chickenwire_right");
	MaterialPtr mat13 = Materials::get()->findMaterial("post_stereo_separate_chickenwire");
	MaterialPtr mat14 = Materials::get()->findMaterial("post_stereo_separate_shading_left");
	MaterialPtr mat15 = Materials::get()->findMaterial("post_stereo_separate_shading_right");
	MaterialPtr mat16 = Materials::get()->findMaterial("post_stereo_separate_shading");


	mat->setParameterSlider(mat->findParameter("distance_between_lenses"), distance_lenses);
	mat3->setParameterSlider(mat3->findParameter("distance_between_lenses"), distance_lenses);
	mat4->setParameterSlider(mat4->findParameter("distance_between_lenses"), distance_lenses);
	mat5->setParameterSlider(mat5->findParameter("distance_between_lenses"), distance_lenses);
	mat6->setParameterSlider(mat6->findParameter("distance_between_lenses"), distance_lenses);
	mat7->setParameterSlider(mat7->findParameter("distance_between_lenses"), distance_lenses);
	mat8->setParameterSlider(mat8->findParameter("distance_between_lenses"), distance_lenses);
	mat9->setParameterSlider(mat9->findParameter("distance_between_lenses"), distance_lenses);
	mat10->setParameterSlider(mat10->findParameter("distance_between_lenses"), distance_lenses);
	mat11->setParameterSlider(mat11->findParameter("distance_between_lenses"), distance_lenses);
	mat12->setParameterSlider(mat12->findParameter("distance_between_lenses"), distance_lenses);
	mat13->setParameterSlider(mat13->findParameter("distance_between_lenses"), distance_lenses);
	mat14->setParameterSlider(mat14->findParameter("distance_between_lenses"), distance_lenses);
	mat15->setParameterSlider(mat15->findParameter("distance_between_lenses"), distance_lenses);
	mat16->setParameterSlider(mat16->findParameter("distance_between_lenses"), distance_lenses);
}