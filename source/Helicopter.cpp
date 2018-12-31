
#include <UnigineEditor.h>
#include <UnigineNodes.h>
#include "Helicopter.h"
#include "Utils.h"
#include "Resource.h"

//used for helicopter cabin adjust
float adjustSpeed = 0.5;


void Helicopter::init() {

	 airflowMax = 100.0f;
	 airflowMid = 50.0f;
	 airflowMin = 0.0f;
	 minForceAttractor = 29.20f;
	 maxForceAttractor = 49.20f;
	 maxSpawnRate = 600.0f;
	 minSpawnRate = 300.0f;

	 //the child node must be initial after the helicopter otherwise the position will be wrong
	 Resource::get()->ObjectParticles_Airflow->setWorldRotation(quat(0.0f, 0.0f, 0.0f, 0.0f));
	 waterHeight = Resource::get()->water->getWorldPosition().z;

	 minigun = new MiniGun();
	 minigun->init();

}



void Helicopter::update()
{

	minigun->update();
	//get the current landheight
	landHeight = getTerrainHeightAt(Resource::get()->land, Resource::get()->vive_tracker_helicopter->getWorldPosition());
	//air flow is always attached to the ground
	Resource::get()->ObjectParticles_Airflow->setWorldPosition(Vec3(Resource::get()->vive_tracker_helicopter->getWorldPosition().x, Resource::get()->vive_tracker_helicopter->getWorldPosition().y, landHeight - 2));
	Resource::get()->ObjectParticles_Airflow->setWorldRotation(quat(0.0f, 0.0f, 0.0f, 0.0f));
	
	if (landHeight <= 0)
	{
		landHeight = 0;
		//hide the airflow when in the water
		Resource::get()->ObjectParticles_Airflow->setWorldPosition(Vec3(Resource::get()->vive_tracker_helicopter->getWorldPosition().x, Resource::get()->vive_tracker_helicopter->getWorldPosition().y, landHeight - 5));
	}

	//the airflow effect
	float distance = Resource::get()->vive_tracker_0->getWorldPosition().z - landHeight;

	//the waterHeightfield position and rotation
	Resource::get()->fieldHeightWater->setWorldPosition(Vec3(Resource::get()->vive_tracker_0->getWorldPosition().x, Resource::get()->vive_tracker_0->getWorldPosition().y, waterHeight - 2));
	Resource::get()->fieldHeightWater->setWorldRotation(quat(0.0f, 0.0f, 0.0f, 0.0f));

	float height = Resource::get()->vive_tracker_0->getWorldPosition().z - waterHeight;

	if (height > 100)
		Resource::get()->fieldHeightWater->setEnabled(0);
	else if (height < 100 || height == 100)
	{
		Resource::get()->fieldHeightWater->setEnabled(1);
		//range 0.2-1.0
		Resource::get()->waterflow->setLengthStretch(1 - height / 100);
	}

	if (distance <= airflowMid)
	{
		Resource::get()->airflow->setForceAttractor(0, maxForceAttractor);
		Resource::get()->airflow->setSpawnRate(maxSpawnRate);
	}

	if (distance > airflowMid && (distance <= airflowMax))
	{
		Resource::get()->airflow->setForceAttractor(0, minForceAttractor);
		Resource::get()->airflow->setSpawnRate(minSpawnRate);
	}

	if (distance > airflowMax || distance < 0)
	{
		Resource::get()->airflow->setForceAttractor(0, -100.00);
		Resource::get()->airflow->setSpawnRate(0);
	}

	//ajust the box of helicopter press key O to store the position
	adjustHelicopter();
	
	//the rotation of rotor 
	RotateNode(Resource::get()->RotorDummy, Math::quat(0, 0, 1, (10 * -60) *Game::get()->getIFps()));
	RotateNode(Resource::get()->rotor_1, Math::quat(0, 0, 1, (10 * -60) * Game::get()->getIFps()));
	RotateNode(Editor::get()->getNodeByName("rotor_base_show"), Math::quat(0, 0, 1, (10 * -60) *Game::get()->getIFps()));
	//key h hide the gun_holder
	if (App::get()->clearKeyState(104))
	{
		Resource::get()->gun_holder->setEnabled(!isShowGun_Holder);
		isShowGun_Holder = !isShowGun_Holder;
	}
}
//adjust the Helicopter Cabin local position, used to calibrate the cabin position 
void Helicopter::adjustHelicopter()
{
	//adjust Helicopter box position
	if (App::get()->getKeyState(101) == 1)
	{
		Resource::get()->vive_tracker_helicopter->setPosition(Vec3(Resource::get()->vive_tracker_helicopter->getPosition().x, Resource::get()->vive_tracker_helicopter->getPosition().y, Resource::get()->vive_tracker_helicopter->getPosition().z - Game::get()->getIFps()*adjustSpeed));
	}
	if (App::get()->getKeyState(113) == 1)

	{
		Resource::get()->vive_tracker_helicopter->setPosition(Vec3(Resource::get()->vive_tracker_helicopter->getPosition().x, Resource::get()->vive_tracker_helicopter->getPosition().y, Resource::get()->vive_tracker_helicopter->getPosition().z + Game::get()->getIFps()*adjustSpeed));
	}

	if (App::get()->getKeyState(115) == 1)
	{
		Resource::get()->vive_tracker_helicopter->setPosition(Vec3(Resource::get()->vive_tracker_helicopter->getPosition().x, Resource::get()->vive_tracker_helicopter->getPosition().y - Game::get()->getIFps() *adjustSpeed, Resource::get()->vive_tracker_helicopter->getPosition().z));
	}
	if (App::get()->getKeyState(119) == 1)

	{
		Resource::get()->vive_tracker_helicopter->setPosition(Vec3(Resource::get()->vive_tracker_helicopter->getPosition().x, Resource::get()->vive_tracker_helicopter->getPosition().y + Game::get()->getIFps() *adjustSpeed, Resource::get()->vive_tracker_helicopter->getPosition().z));
	}

	if (App::get()->getKeyState(97) == 1)
	{
		Resource::get()->vive_tracker_helicopter->setPosition(Vec3(Resource::get()->vive_tracker_helicopter->getPosition().x - Game::get()->getIFps() *adjustSpeed, Resource::get()->vive_tracker_helicopter->getPosition().y, Resource::get()->vive_tracker_helicopter->getPosition().z));
	}
	if (App::get()->getKeyState(100) == 1)
	{
		Resource::get()->vive_tracker_helicopter->setPosition(Vec3(Resource::get()->vive_tracker_helicopter->getPosition().x + Game::get()->getIFps() *adjustSpeed, Resource::get()->vive_tracker_helicopter->getPosition().y, Resource::get()->vive_tracker_helicopter->getPosition().z));
	}

}


