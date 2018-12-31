#pragma once

#include <UnigineEditor.h>
#include <UnigineGame.h>
#include <UnigineApp.h>
#include <UnigineNode.h>
#include <UnigineMathlib.h>
#include <UnigineLights.h>
#include <Math.h>
#include "Winch.h"
#include <plugins/CigiClientInterface.h>
#include <plugins/SynckerInterface.h>
#include "Utils.h"
#include "MiniGun.h"
#include "Resource.h"
#define PI 3.14159265

using namespace Unigine;
using namespace Math;

class Helicopter
{

private:

	float airflowMax;
	float airflowMid;
	float airflowMin;
	float minForceAttractor;
	float maxForceAttractor;
	
	float maxSpawnRate;
	float minSpawnRate;

	bool isGoProView;
	bool isPilotView = true;
	bool isTailView ;
	bool isTopview;
	bool isShowGun_Holder = true;
public:
	

	friend class Resource;

	float waterHeight;

	static Helicopter *get()
	{
		static Helicopter instance;
		return &instance;
	}


	MiniGun * minigun;
    float landHeight;
	void init();
	void update();
	void flush();
	void shutdown();

	//rotate along the world axises
	int RotateNodeWorld(Unigine::NodePtr node, Unigine::Math::quat rotationDirection)
	{
		// getting current node transformation matrix
		Unigine::Math::Mat4 transform = node->getWorldTransform();

		// calculating delta rotation around an arbitrary axis
		Unigine::Math::quat delta_rotation = Unigine::Math::quat(rotationDirection);

		// setting node's scale, rotation and position

		node->setWorldRotation(delta_rotation * node->getWorldRotation());

		return 1;
	}


	int TranslateNode(NodePtr node, Vec3 translateDirection)
	{
		// getting current node transformation matrix
		Math::Mat4 transform = node->getWorldTransform();
		node->setWorldPosition(node->getWorldPosition() + (-transform.getAxisY() * (translateDirection.x * 50) * Game::get()->getIFps()));


		return 1;
	}


	void SetAirflowNormal(ObjectTerrainPtr terrain, Vec3 helicopter_position, ObjectParticlesPtr airflow)

	{
		vec4 local_position = vec4(terrain->getIWorldTransform() * Vec4(helicopter_position, 1.0f));
		if (local_position.x > 0 && local_position.y > 0)
		{
			// calculating x,y coordinates of terrain point under the player and updating height value
			int x = ftoi(local_position.x / terrain->getStep());
			int y = ftoi(local_position.y / terrain->getStep());

			if (x != NULL || y != NULL) //in case there is no terrain
			{

				int normalX = terrain->getNormal(x, y).x * 1000;
				int normalY = terrain->getNormal(x, y).y * 1000;
				int normalZ = terrain->getNormal(x, y).z * 1000;

				float cos_reslut = sqrt((int)pow(normalX, 2) + (int)pow(normalY, 2)) / sqrt((int)pow(normalX, 2) + (int)pow(normalY, 2) + (int)pow(normalZ, 2));
				float angle = Math::acos(static_cast<float> (cos_reslut)) * 180 / PI;
				quat r = quat(terrain->getNormal(x, y));
				// if the angle of the slope is less than 40 degree then ariflow will attach to the surface of terrain
				if (50 < angle)
				{
					airflow->getNode()->setWorldRotation(r);
				}

			}

		}
	}

	void ControlCamera()
	{

		if (isGoProView)
		{
			//camera change
			Resource::get()->Go_pro_view->setWorldTransform(Resource::get()->Go_pro_view->getParent()->getWorldTransform());
			isGoProView = false;
			isPilotView = true;
			isTailView = false;
			isTopview = false;
			Game::get()->setPlayer(Resource::get()->Go_pro_view->getPlayer());
			
		}

		else if (isPilotView)
		{
			Resource::get()->pilot_view->setWorldTransform(Resource::get()->pilot_view->getParent()->getWorldTransform());
			Resource::get()->pilot_view->setRotation(quat(Resource::get()->pilot_view->getRotation().x, Resource::get()->pilot_view->getRotation().y - 90, Resource::get()->pilot_view->getRotation().z - 90));
			
			isGoProView = false;
			isPilotView = false;
			isTailView = true;
			isTopview = false;

			Game::get()->setPlayer(Resource::get()->pilot_view->getPlayer());
		}

	   else if (isTailView)
			{
				
				 isGoProView = false;
				 isPilotView = false;
				 isTailView = false;
				 isTopview = true;
				 Game::get()->setPlayer(Resource::get()->Camera_ThirdPerson->getPlayer());
			}
	   else if (isTopview)
	   {

		   isGoProView = true;
		   isPilotView = false;
		   isTailView = false;
		   isTopview = false;
		   Game::get()->setPlayer(Resource::get()->Top_view->getPlayer());
	   }
	}

	void adjustHelicopter();


};
