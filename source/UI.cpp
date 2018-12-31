#include "UI.h"
#include "Resource.h"
#include <UnigineWidgets.h>
#include <UnigineGui.h>
#include <UnigineEngine.h>
#include <UniginePlayers.h>
#include <UnigineEditor.h>
#include <UnigineApp.h>
#include <UnigineViewport.h>


#include <UnigineUserInterface.h>
#include <UnigineInterpreter.h>
#include <UnigineMaterial.h>
#include <UnigineMaterials.h>
using namespace Unigine;
using namespace Math;



float clouds_pos_z;
float clouds_height;
float longitude_land;
float altitude_land;
float longitude_water;
float altitude_water;

vec3 flir_dir;
vec3 camera_dir;
Vec3 targetPointPosition;
Vec3 GeodeticPosition;
Vec3 lockPosition;

void UI::init()
{
	// get gui
	gui = Gui::get();


	// button of views

//	logo1 = WidgetSprite::create(gui, "Images/becker-helicopters.png");
//	gui->addChild(logo1->getWidget());

	hideClouds = WidgetButton::create(gui, "Clouds");
	hideClouds->setPosition(12, 14);
	hideClouds->arrange();
	hideClouds->setToggleable(1);

	followHelicopter = WidgetButton::create(gui, "Helicopter");
	followHelicopter->setPosition(72, 14);
	followHelicopter->arrange();
	followHelicopter->setToggleable(1);

	gui->addChild(hideClouds->getWidget(), Gui::ALIGN_OVERLAP | Gui::ALIGN_FIXED);
	gui->addChild(followHelicopter->getWidget(), Gui::ALIGN_OVERLAP | Gui::ALIGN_FIXED);
	
	LongitudeText = WidgetEditText::create(gui, "");
	AltitudeText = WidgetEditText::create(gui,  "");
	FovText = WidgetEditText::create(gui, "");
	LockText = WidgetEditText::create(gui, "");
	//hide the background
	LongitudeText->setBackground(0);
	AltitudeText->setBackground(0);
	FovText->setBackground(0);
	LockText->setBackground(0);
	//font size

	LongitudeText->setFontSize(20);
	AltitudeText->setFontSize(20);
	FovText->setFontSize(20);
	LockText->setFontSize(20);

	gui->addChild(LongitudeText->getWidget(), Gui::ALIGN_OVERLAP | Gui::ALIGN_BOTTOM);
	gui->addChild(AltitudeText->getWidget(), Gui::ALIGN_OVERLAP | Gui::ALIGN_BOTTOM);
	gui->addChild(FovText->getWidget(), Gui::ALIGN_OVERLAP | Gui::ALIGN_BOTTOM);
	gui->addChild(LockText->getWidget(), Gui::ALIGN_OVERLAP | Gui::ALIGN_BOTTOM);
	
	LongitudeText->setPosition(20, 0);
	AltitudeText->setPosition(220, 0);
	FovText->setPosition(420,0);
	LockText->setPosition(580, 0);
	
	LockText->setFontColor(vec4(1.0f,1.0f, 0.0f, 1));

	aim = WidgetSprite::create(gui, "textures/ui/aim.png");

	gui->addChild(aim->getWidget(), Gui::ALIGN_CENTER);


}


void UI::updateOperation() {

	if (hideClouds->isToggled() == 1)
	{
		Resource::get()->clouds->setEnabled(1);
	}
	else
	{
		Resource::get()->clouds->setEnabled(0);
	}

	if (followHelicopter->isToggled() == 1)
	{
		Game::get()->getPlayer()->setWorldPosition(Vec3(Resource::get()->vive_tracker_helicopter->getWorldPosition().x, Resource::get()->vive_tracker_helicopter->getWorldPosition().y, Resource::get()->vive_tracker_helicopter->getWorldPosition().z + 50));
		Resource::get()->player_satellite->setWorldPosition(Vec3(Resource::get()->vive_tracker_helicopter->getWorldPosition().x, Resource::get()->vive_tracker_helicopter->getWorldPosition().y, Resource::get()->player_satellite->getWorldPosition().z));
	}
	
}

void UI::shutdown() {
	// clear window
	//interface_plugin->releaseWindow(interface_window1);
	//interface_plugin->releaseWindow(interface_window2);
	//pilotview->release();

	//gui->removeChild(weightbutton->getWidget());
	//gui->removeChild(thirdpersonview->getWidget());
	//gui->removeChild(pilotview->getWidget());
	//gui->removeChild(logo1->getWidget());
	//gui->removeChild(logo2->getWidget());

	gui->removeChild(aim->getWidget());
	gui->removeChild(hideClouds->getWidget());
	gui->removeChild(followHelicopter->getWidget());
}

void UI::GetFlirDistance()
{

	flir_dir = getEntityDirection(Resource::get()->flir_camera_point1->getWorldPosition(), Resource::get()->flir_camera_point2->getWorldPosition());
	
	const Vec3 &p0 = Resource::get()->flir_camera_point1->getWorldPosition();
	const Vec3 &p1 = p0 + Vec3(flir_dir) * 5000.0f;
	const Vec3 &direction = normalize(p1 - p0);

	int intersection = World::get()->getIntersection(p0, p1, hitObjs);

	if (intersection)
	{
		ObjectIntersectionPtr object_intersection = ObjectIntersection::create();
		bool hitTerrain = false;
		int hitObjNum = 0;
		for (auto hitObj : hitObjs)
		{
			//hitObj->getIWorldTransform()
			const vec3 &lp0 = (vec3)(hitObj->getIWorldTransform()* p0);
			const vec3 &lp1 = (vec3)(hitObj->getIWorldTransform()* p1);
			if (!hitObj->getIntersection(lp0, lp1, ~0, object_intersection, nullptr))
				continue;

			if (hitObj->getType() == Node::OBJECT_WATER_GLOBAL)
			{
				targetPointPosition = hitObj->getWorldTransform() * (Vec3)(object_intersection->getPoint());
				GeodeticPosition = Resource::get()->geopivot->mapFlatToGeodetic(targetPointPosition);
				longitude_water = GeodeticPosition.x;
				altitude_water = GeodeticPosition.y;
				hitObjNum += 1;
			}
			else 
		    if (hitObj->getType() == Node::OBJECT_TERRAIN_GLOBAL)
			{
				targetPointPosition = hitObj->getWorldTransform() * (Vec3)(object_intersection->getPoint());
				GeodeticPosition = Resource::get()->geopivot->mapFlatToGeodetic(targetPointPosition);
				longitude_land = GeodeticPosition.x;
				altitude_land = GeodeticPosition.y;
				hitTerrain = true;
				hitObjNum += 1;
			}
		}
		//if hit both water and land
		if (hitTerrain)
		{
			longitude = longitude_land;
			altitude = altitude_land;
		}
		else if (!hitTerrain)
		{
			longitude = longitude_water;
			altitude = altitude_water;
		}

	}

	int fov_int_dem = int(Resource::get()->player_flir->getFov() * 100);
	int fov_int = int(fov_int_dem / 100);
	int fov_dem = int(fov_int_dem - fov_int * 100);

	if (longitude == 0 && altitude == 0)
	{
		LongitudeText->setLineText(0, "L:NULL");
		AltitudeText->setLineText(0, "A:NULL");
		FovText->setLineText(0, ("FOV:" + std::to_string(fov_int) + "." + std::to_string(fov_dem)).c_str());
	}
	else
	{
		LongitudeText->setLineText(0, ("L:" + std::to_string(longitude)).c_str());
		AltitudeText->setLineText(0, ("A:" + std::to_string(altitude)).c_str());
		FovText->setLineText(0, ("FOV:" + std::to_string(fov_int) + "." + std::to_string(fov_dem)).c_str());
	}

	lock();
}




void UI::lock()
{
	//key l lock the position
	if (App::get()->clearKeyState(108) == 1)
	{
		isLock = !isLock;
		if (isLock == true)
		{
			lockPosition = targetPointPosition;
			LockText->setLineText(0, "LOCKED");
		}
		else
		{
			LockText->setLineText(0, "");
		}
	}

	if (isLock)
	{
		camera_dir = getEntityDirection(Resource::get()->flir_camera_point1->getWorldPosition(), lockPosition);
		Resource::get()->player_flir_reference ->setWorldDirection(camera_dir, vec3::UP);
	}

}