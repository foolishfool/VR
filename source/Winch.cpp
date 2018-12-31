
#include <UnigineEngine.h>
#include <UnigineGame.h>
#include <UnigineGui.h>
#include <UnigineObjects.h>
#include <UniginePrimitives.h>
#include <UniginePhysics.h>
#include <UniginePhysicals.h>
#include <UnigineApp.h>
#include <UnigineMaterials.h>
#include <UnigineConsole.h>
#include "Winch.h"
#include "Resource.h"
#include <math.h>


using namespace Unigine; 
using namespace Math;


//the speed of winch to rotate
float winchRotSpeed = 25.0f;
float handleMass_original = 100;
float hookMass_original = 1;

const float CHANGE_INTERVAL = 0.04f; // the interval between changes of position, in seconds
float elapsed_time = CHANGE_INTERVAL;	// current time left to change current scale of our objects


Winch* Winch::instance;

Winch* Winch::get()
{
	return instance;
}

void Winch::init()
{
	instance = this;

	joystick = ControlsJoystick::create(0);

	// the falling speed of the weight
	Physics::get()->setGravity(vec3(0.0f, 0.0f, -9.8f * 2.0f));

	hookInitialPosition = Resource::get()->hookreference->getWorldPosition();

	wincharm = LoadFromEditor("WinchArm");
	
	number = ObjectText::cast(LoadFromEditor("pendant_number"));

	//initialize wind
	//wind = PhysicalWind::cast(LoadFromEditor("Wind"));
	//wind->setLinearDamping(20.0f);
	//wind->setAngularDamping(20.0f);
	//wind->setVelocity(vec3(-1.0f, 00.0f, 0.0f));

	lastposition = Resource::get()->vive_tracker_0->getWorldPosition();

	winchsound = AmbientSource::create("sound/WinchMotor.wav", 0);
	winchsound->setLoop(0);
	cutsound = AmbientSource::create("sound/WinchCableSnap.wav", 0);

	//create the first hope
	//dynamically creating the rope version
/*
	Rope* rope_initial = new Rope();

	ropes.append(rope_initial);	
	
	Rope_intial_point = ropes.get(ropes.size()-1)->Rope_intial_point_node;
	
	ropebody1 = ropes.get(ropes.size()-1)->ropebody1;
	ropebody2 = ropes.get(ropes.size()-1)->ropebody2;

	hookbody = ropes.get(ropes.size() - 1)->hookBody;
	handlebody = ropes.get(ropes.size() - 1)->handlebody;
*/
	//************* old version of rope

	Rope_intial_point_obj = ObjectDummy::cast(LoadFromEditor("Rope_intial_point"));
	BodyDummy::create(Rope_intial_point_obj->getObject());

	hook = ObjectMeshDynamic::cast(LoadFromEditor("Hook"));
	handle = ObjectMeshDynamic::cast(LoadFromEditor("Handle"));

	hookBody = BodyRigid::create(hook->getObject());
	//only affected by the wind
	hookBody->setPhysicalMask(8000);
	handlebody = BodyRigid::create(handle->getObject());

	hookBody->getBody()->addShape(ShapeBox::create(vec3(0.1f))->getShape(), translate(-0.005f, 0.0f, -0.012f));
	handlebody->getBody()->addShape(ShapeBox::create(vec3(0.2f))->getShape(), translate(0.0f, 0.0f, 0.0f));
	// the mass of weight cannot be too lower than the rope
	hookBody->getBody()->getShape(0)->setMass(100);
	handlebody->getBody()->getShape(0)->setMass(100);

	//create uppper winchrope the initial longth is fixed, if changed the ropeNode1 and ropeNode2 position should also change first	
	upper_winchrope = ObjectMeshDynamic::cast(LoadFromEditor("rope_upper"));
	lower_winchrope = ObjectMeshDynamic::cast(LoadFromEditor("rope_lower"));

	CreateWinchRope();
}


void Winch::update()

{
	//dynamically creating the rope version
	/*
	ropes.get(ropes.size() - 1)->update();
	//get current hook_reference node
	
	//feet
	currenthook = ropes.get(ropes.size() - 1)->hook_node;
	ropelength = sqrtf(powf((currenthook->getWorldPosition().x - hookInitialPosition.x), 2) + powf((currenthook->getWorldPosition().y - hookInitialPosition.y), 2) + powf((currenthook->getWorldPosition().z - hookInitialPosition.z), 2))* 3.28084;
	
	handlebody->getShape(0)->setMass(Resource::get()->handle_mass);
	currenthook->setRotation(quat(0.275, 359.750, 0.241));
	
	*/

	if (!joystick)
	{
		Log::error("No USB Joystick!!!!!!!!!!!\n");
	}
	///if not using joystick, disable the following comment, otherwise low fps
	//joystick->updateEvents();
	
	//joystick control
	if (joystick->isAvailable())
	{
		//Yaxis
		val1 = joystick->getAxis(6);

		if (val1 < 0)
		{
			float roate = lerp(180.0f, 220.0f, -val1);
			Resource::get()->scrollwheel->setRotation(quat(270.000, 0, roate));
		}
		else if (val1 > 0)
		{
			float roate = lerp(180.0f, 140.0f, val1);
			Resource::get()->scrollwheel->setRotation(quat(270.000, 0, roate));
		}
		else
		{
			Resource::get()->scrollwheel->setRotation(quat(270.000, 0, 180.00));
		}
	
		if (val1 > 0.2)
		{
			isUp = true;
			isDown = false;
		}
		else if (val1 < -0.2)
		{
			isDown = true;
			isUp = false;
		}
		else
		{
			isUp = false;
			isDown = false;
			Physics::get()->setGravity(vec3(0.0f, 0.0f, -9.8f * 2.0f));
		}
		if (joystick->getButton(0) == 1)
		{
			isRotateRight = true;
		}
		else
		{
			isRotateRight = false;
		}
		if (joystick->getButton(1) == 1)
		{
			isRotateLeft = true;
		}
		else
		{
			isRotateLeft = false;
		}

		if (joystick->getButton(2) == 1)
		{
			isCutRope = true;
		}
		else
		{
			isCutRope = false;
		}		
	}
	else
	{
		isUp = false;
		isDown = false;
		isRotateRight = false;
		isRotateLeft = false;
		val1 = 0.67;
	}

	currentposition = Resource::get()->vive_tracker_0->getWorldPosition();
	// checking if it's time to change current position
	if (elapsed_time < 0.0f)
	{
		helicopter_speed = sqrtf(powf((currentposition.x - lastposition.x), 2) + powf((currentposition.y - lastposition.y), 2) + powf((currentposition.z - lastposition.z), 2));
		lastposition = currentposition;
		// resetting elapsed time counter
		elapsed_time = CHANGE_INTERVAL;
	}

	// decreasing the time counter to the next change 
	elapsed_time -= Game::get()->getIFps();
	// the velocity of wind
	//wind->setVelocity(vec3(-1 * helicopter_speed, 00.0f, 0.0f));
	
	//the maxvalue of ropelength is 250 feet
	if (ropelength > 250)
	{
		ropelength = 250;
	}
	
	//adjust the speed of moving rope under 20 feets constant speed then viralbe speed
	if (ropelength <= 8.6)
	{
		rope_speed = 3;
	}
	else
	{
		rope_speed = 10;
	}

	//hide the upper_rope
	//if (ropelength <3)
	//{
	//	Resource::get()->rope_mat->setParameterSlider("transparent", 0);
	//}
	//else
	//{
	//	Resource::get()->rope_mat->setParameterSlider("transparent", 1);
	//}

	int integer = ((int)(ropelength * 100 + .5) / 100.0);
	int decimals = ((int)((ropelength - integer) * 10000) / 100);

	number->setText((std::to_string(integer)).c_str());
	std::string f = (std::to_string(integer) + '.' + std::to_string(decimals)).c_str();

	currentRotAngle = wincharm->getRotation().getAngle(vec3(0, 1, 0));

	// winchTrigger is controlled by the VR controller
	if ((Resource::get()->WinchTrigger->getScale().y > 1.8f) && Resource::get()->WinchTrigger->getScale().y < 2.8f)
	{
		isVRDown = true;
	}
	else
	{
		isVRDown = false;
	}

	if ((Resource::get()->WinchTrigger->getScale().x > 1.8f) && (Resource::get()->WinchTrigger->getScale().x < 2.8f))
	{
		isVRUp = true;
	}
	else
	{
		isVRUp = false;
	}
	
	//if ((winchTrigger->getScale().x > 2.8f))
	//{
	//	isVRCutRope = true;
	//}
	//else
	//{
	//	isVRCutRope = false;
	//}

	if ((Resource::get()->WinchTrigger->getScale().y > 2.8f) && (Resource::get()->WinchTrigger->getScale().y < 3.8f))
	{
		isVRWinchLeftRotate  = true;
	}
	else
	{
		isVRWinchLeftRotate = false;
	}

	if ((Resource::get()->WinchTrigger->getScale().z > 1.8f) && (Resource::get()->WinchTrigger->getScale().z < 2.8f))
	{
		isVRWinchRightRotate = true;
	}
	else
	{
		isVRWinchRightRotate = false;
	}

	/* //dynamically creating the rope version
	//key 9
	if (( App::get()->getKeyState(57) == 1 || isDown ||isVRDown ))
	{
		Rope_intial_point = ropes.get(ropes.size() - 1)->Rope_intial_point_node;
		ropebody1 = ropes.get(ropes.size() - 1)->ropebody1;
		ropebody2 = ropes.get(ropes.size() - 1)->ropebody2;

		if (Rope_intial_point->getPosition().y >= -0.135)
		{
			Rope_intial_point->setPosition(Vec3(Rope_intial_point->getPosition().x, Rope_intial_point->getPosition().y - Game::get()->getIFps()*0.1, Rope_intial_point->getPosition().z));
		}
	
		if (ropebody1->getLinearStretch() < 5 )
		{
			ropebody1->setLinearStretch(ropebody1->getLinearStretch() + Game::get()->getIFps() * rope_speed);
		}
		else if (ropebody1->getLinearStretch() >= 5 )
		{
			ropebody2->setLinearStretch(ropebody2->getLinearStretch() + Game::get()->getIFps() * rope_speed );
		}

	if (ropebody2->getLinearStretch()  <=22 )
	{
		Physics::get()->setGravity(vec3(0.0f, 0.0f, -9.8f * 2.0f));
	//	handlebody->getShape(0)->setMass(handleMass_original);
	}
	else if (ropebody2->getLinearStretch() > 22 && ropebody2->getLinearStretch()< 75)
	{
		Physics::get()->setGravity(vec3(0.0f, 0.0f, -9.8f * 4.0f));
		currenthook->setRotation(quat(0.275, 359.750, 0.241));
		handlebody->getShape(0)->setMass(0);

	}
	else if (ropebody2->getLinearStretch() >= 75)
	{
		handlebody->getShape(0)->setMass(0);
		Physics::get()->setGravity(vec3(0.0f, 0.0f, -9.8f * 4.0f));
	//	ropebody2->setLinearStretch(ropebody2->getLinearStretch() + Game::get()->getIFps() * 1.5 * rope_speed);
	}

	currenthook->setRotation(quat(0.275, 359.750, 0.241));
		//roatate
	RopeRotate(20);
	}
	
	if (ropebody2->getLinearStretch() > 30)
	{
		RopeRotate(5);
	}
		//KEY:0
	if (App::get()->getKeyState(48) == 1 || isUp|| isVRUp )
	{
		Rope_intial_point = ropes.get(ropes.size() - 1)->Rope_intial_point_node;
		ropebody1 = ropes.get(ropes.size() - 1)->ropebody1;
		ropebody2 = ropes.get(ropes.size() - 1)->ropebody2;

		NodePtr handle = ropes.get(ropes.size() - 1)->handle_node;
	
		if (ropebody2->getLinearStretch() >= 22)
		{
			handlebody->getShape(0)->setMass(0);
			currenthook->setRotation(quat(0.275, 359.750, 0.241));
			ropebody2->setLinearStretch(ropebody2->getLinearStretch() - Game::get()->getIFps() * rope_speed * 3 );

		}
		else
		{
			ropebody2->setLinearStretch(ropebody2->getLinearStretch() - Game::get()->getIFps() * rope_speed );
			handlebody->getShape(0)->setMass(Resource::get()->handle_mass);
		}

		if (ropebody2->getLinearStretch() == 0 && ropebody1->getLinearStretch() > 0)
		{
			ropebody1->setLinearStretch(ropebody1->getLinearStretch() - Game::get()->getIFps() * rope_speed  );
		}
		if (ropebody2->getLinearStretch() == 0 && ropebody1->getLinearStretch() <= 0 )
		{
			
			if (ropelength > 0.35)
			{
				Rope_intial_point->setPosition(Vec3(Rope_intial_point->getPosition().x, Rope_intial_point->getPosition().y + Game::get()->getIFps() * 0.1, Rope_intial_point->getPosition().z));
			}
		}

		currenthook->setRotation(quat(0.275, 359.750, 0.241));

		RopeRotate(-20);
	}
	
	//change the color of handle
	if (ropebody1->getLinearStretch()>5)
	{
		Resource::get()->handle_mat->setParameter("albedo_color", vec4(1, 1, 1, 1));
	}
	else
		Resource::get()->handle_mat->setParameter("albedo_color", vec4(1, 1, 0, 1));
	*/

	//rotate winch KEY:-	
	if (App::get()->getKeyState(45) == 1 || isRotateLeft || isVRWinchLeftRotate)
	{
		//range 33.4--67
		if (currentRotAngle  > -68 && currentRotAngle < 33)
		{
			Rotate(wincharm->getNode(), Math::quat(0, 1, 0, winchRotSpeed*Game::get()->getIFps()));
			winchsound->play();
		}

	}

	//rotate winch KEY:=
	else if (App::get()->getKeyState(61) == 1 || isRotateRight ||isVRWinchRightRotate )
	{
		if (currentRotAngle < 34 && currentRotAngle > -67)
		{
			Rotate(wincharm->getNode(), Math::quat(0, 1, 0, -winchRotSpeed*Game::get()->getIFps()));
			winchsound->play();
		}
	}
	else
	{
		winchsound->stop();
	}
	//KEY:i cut rope 
	/*
	//dynamically creating the rope version
	if (App::get()->getKeyState(99) == 1 || isCutRope)
	{
		if (Winch::get()->isCut == false)
		{
			CutRope();
			Winch::get()->isCut = true;
		}
	}
	//Log::message("%d \n", Winch::get()->isCut);

	if ((Resource::get()->WinchTrigger->getScale().x > 2.8f) && (Resource::get()->WinchTrigger->getScale().x < 3.8f))
	{
		if (Winch::get()->isCut == true)
		{
			Rope* new_rope = new Rope();
			ropes.append(new_rope);	
			Winch::get()->isCut = false;
		}
	}
	*/

	//**** old version rope
	//adjust the speed of moving rope under 20 feets constant speed then viralbe speed
	if (ropelength <= 20 * 0.305)
	{
		rope_speed = 10;
	}
	else
		rope_speed = fabs(val1 - 0.37) * 15;


	if (App::get()->getKeyState(57) == 1 || isDown || isVRDown)
	{
		if (Rope_intial_point_obj->getPosition().y >= -0.135)
		{
			Rope_intial_point_obj->setPosition(Vec3(Rope_intial_point_obj->getPosition().x, Rope_intial_point_obj->getPosition().y - Game::get()->getIFps()*0.25, Rope_intial_point_obj->getPosition().z));
		}

		if (ropebody1->getLinearStretch() < 15)
		{
			ropebody1->setLinearStretch(ropebody1->getLinearStretch() + Game::get()->getIFps() * rope_speed);
		}

		else
		{

			ropebody2->setLinearStretch(ropebody2->getLinearStretch() + Game::get()->getIFps() * rope_speed);
		}

		//	Log::message("%f \n", ropebody1->getLinearStretch());
	}

	//KEY:0
	if (App::get()->getKeyState(48) == 1 || isUp || isVRUp)
	{
		if (ropebody2->getLinearStretch() > 0.0)
		{
			ropebody2->setLinearStretch(ropebody2->getLinearStretch() - Game::get()->getIFps() * rope_speed * 2);
		}
		if (ropebody2->getLinearStretch() == 0 && ropebody1->getLinearStretch() > 1.9)
		{
			ropebody1->setLinearStretch(ropebody1->getLinearStretch() - Game::get()->getIFps() * rope_speed * 1.5);
		}
		if (ropebody2->getLinearStretch() == 0 && ropebody1->getLinearStretch() >= 0.0)
		{

			if (Rope_intial_point_obj->getPosition().y < 0.366)
			{
				Rope_intial_point_obj->setPosition(Vec3(Rope_intial_point_obj->getPosition().x, Rope_intial_point_obj->getPosition().y + Game::get()->getIFps() * 0.25, Rope_intial_point_obj->getPosition().z));
			}
		}
	}
	
	if (ropebody1->getLinearStretch()>15)
	{
		Resource::get()->handle_mat->setParameter("albedo_color", vec4(1, 1, 1, 1));
	}
	else
		Resource::get()->handle_mat->setParameter("albedo_color", vec4(1, 1, 0, 1));
	
	return;
}

void Winch::shutdown()
{
	return;
}

void Winch::RopeRotate(float rotateSpeed)
{
		RotateNode(ropes.get(ropes.size() - 1)->basket_node, Math::quat(0, 0, 1, rotateSpeed * Game::get()->getIFps()));
		RotateNode(ropes.get(ropes.size() - 1)->weight_strop_node, Math::quat(0, 0, 1, rotateSpeed * Game::get()->getIFps()));
		RotateNode(ropes.get(ropes.size() - 1)->weight_stretcher_node, Math::quat(0, 0, 1, rotateSpeed * Game::get()->getIFps()));
}

