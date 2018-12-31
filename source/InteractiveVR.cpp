#include "InteractiveVR.h"
#include <UnigineConsole.h>
#include <UnigineApp.h>
#include <UnigineEditor.h>
#include <UnigineGame.h>
#include "VR.h"
#include "Resource.h"

InteractiveVR* InteractiveVR::instance;

InteractiveVR* InteractiveVR::get()
{
	return instance;
}

void InteractiveVR::init()
{

	instance = this;
	//dynamically create the rope version, previous version, to ask the rope to reset
	//handle = Winch::get()->ropes.get(Winch::get()->ropes.size() - 1)->handle_objmovable;
	//hook = Winch::get()->ropes.get(Winch::get()->ropes.size() - 1)->hook_objmovable;

	handle = new ObjMovable(LoadFromEditor("Handle"));
	hook = new ObjMovable(LoadFromEditor("Hook"));
	door = new ObjMovable(LoadFromEditor("door_handle"));
	door1 = new ObjMovable(LoadFromEditor("door_handle_1"));


	vr = new VR();
	vr->init();
	vr->getPlayer()->setPostMaterials("vr_post_filter_selection");

	Console::get()->run("render_auxiliary 1");

	effect_smoke_particle = ObjectParticles::cast(Resource::get()->effect_reference);

	initialPos_smoke = Resource::get()->smoke_reference->getTransform();
	initialPos_Lid = Resource::get()->lid_reference->getTransform();
	initialPos_Tab = Resource::get()->tab_reference->getTransform();
	initialPos_seamark = Resource::get()->seamark_node->getTransform();
	initialPos_laserpoint = Resource::get()->laserpoint_node->getTransform();
	initialPos_flashlight = Resource::get()->flashlight_node->getTransform();

	createNewItems();

}

void InteractiveVR::update()
{

	vr->update();	

	//get speed of gloves
	Glove::get()->push_hand_linear_velocity(0, (Glove::get()->getworldVelocity(Glove::get()->middlefinger3_left, Glove::get()->lefthand_lastposition)));
	Glove::get()->lefthand_lastposition =Glove::get()->middlefinger3_left->getWorldPosition();
	Glove::get()->push_hand_linear_velocity(1,(Glove::get()->getworldVelocity(Glove::get()->middlefinger3_right, Glove::get()->righthand_lastposition)));
	Glove::get()->righthand_lastposition = Glove::get()->middlefinger3_right->getWorldPosition();

	Glove::get()->push_fireball_linear_velocity(0, (Glove::get()->getworldVelocity(Resource::get()->vive_tracker_1, Glove::get()->lefthand_fireball_lastposition)));
	Glove::get()->lefthand_fireball_lastposition = Resource::get()->vive_tracker_1->getWorldPosition();
	Glove::get()->push_fireball_linear_velocity(1, (Glove::get()->getworldVelocity(Resource::get()->vive_tracker_2, Glove::get()->righthand_fireball_lastposition)));
	Glove::get()->righthand_fireball_lastposition = Resource::get()->vive_tracker_2->getWorldPosition();

	//get speed of leapmotion

	LeapMotion::get()->push_hand_linear_velocity(0, vr->player->getRotation()*(LeapMotion::get()->getworldVelocity(LeapMotion::get()->palm_bone_left->getNode(), LeapMotion::get()->lefthand_lastposition)* LeapMotion::get()->hand_force_multiplier));
	LeapMotion::get()->lefthand_lastposition = LeapMotion::get()->palm_bone_left->getWorldPosition();
	LeapMotion::get()->push_hand_linear_velocity(1, vr->player->getRotation()*(LeapMotion::get()->getworldVelocity(LeapMotion::get()->palm_bone_right->getNode(), LeapMotion::get()->righthand_lastposition)* LeapMotion::get()->hand_force_multiplier));
	LeapMotion::get()->righthand_lastposition = LeapMotion::get()->palm_bone_right->getWorldPosition();

	//use torch
	if (vr->useTorch)
	{
		Resource::get()->flash_beam->setEnabled(1);
		Resource::get()->torch_mat->setState("emission", 1);
	}
	else
	{
		Resource::get()->flash_beam->setEnabled(0);
		Resource::get()->torch_mat->setState("emission", 0);
	}
	//F|| App::get()->getKeyState(102) == 1
	if (vr->fire )
	{
		MiniGun::get()->fire();
	}
	//` clear the bullet number

	if (App::get()->getKeyState(114) == 1)
	{
		MiniGun::get()->bulletNumber = 0;
		MiniGun::get()->elapsed_time_num = 0;
	}


	vive_tracker_transfrom = vr->controller_tracker[3]->getWorldTransform();


	// smoke, lid, tab->update()
	if (lids.size()!= 0)
	{
		updateSmokeMap(lids);
	}
	if (tabs.size()!=0)
	{
		updateSmokeMap(tabs);
	}
	//reset smoke z value
	if (smokes.size() != 0)
	{
		updateSmokeMap(smokes);
	}
	//update seamark
	if (seamarks.size() != 0)
	{
		updateMovableObjMap(seamarks);
	}
	//update laserpointer
	if (laserpoints.size() != 0)
	{
		updateMovableObjMap(laserpoints);
	}
	//update flashlight
	if (flashlights.size() != 0)
	{
		updateMovableObjMap(flashlights);
	}

}

void InteractiveVR::createNewItems()
{
	if (smokes.size() == 0)
	{
		createNewSmoke();
	}
	else if (allTrownSmoke(smokes))
	{
		createNewSmoke();
	}
	//if (flashlights.size()==0)
	//{
	//	createNewFlashlight();
	//}
	//else if (allTrownMovableObj(flashlights))
	//{
	//	createNewFlashlight();
	//}
	if (seamarks.size() == 0)
	{
		createNewSeamark();
	}
	else if (allTrownMovableObj(seamarks))
	{
		createNewSeamark();
	}
	if (laserpoints.size() == 0)
	{
		createNewLaserPoint();
	}
	else if (allTrownMovableObj(laserpoints))
	{
		createNewLaserPoint();
	}
}

void InteractiveVR::createNewSmoke()
{
	NodePtr new_somke_node = Resource::get()->smoke_node->clone();
	NodePtr new_lid_node = Resource::get()->lid_node->clone();
	NodePtr new_tab_node = Resource::get()->tab_node->clone();
	new_somke_node->setEnabled(1);
	new_lid_node->setEnabled(1);
	new_tab_node->setEnabled(1);
	Smoke *newSmoke = new Smoke(new_somke_node);
	Smoke *newlid = new Smoke(new_lid_node);
	Smoke *newtab = new Smoke(new_tab_node);
	ObjectParticlesPtr neweffect = ObjectParticles::cast(new_somke_node->getChild(0));
	neweffect->setEnabled(0);
	neweffect->setEmitterEnabled(0);
	newtab->smoke_effect = neweffect;

	smoke_effects.append(new_somke_node->getChild(0)->getID(),neweffect);

	newSmoke->smoke_node_type = 1;
	newlid->smoke_node_type = 2;
	newtab->smoke_node_type = 3;
	//establish connection with smoke, tab and lid
	newSmoke->tab = newtab;
	newSmoke->lid = newlid;
	newlid->smoke = newSmoke;
	newlid->tab = newtab;
	newtab->smoke = newSmoke;
	newtab->lid = newlid;

	//store new smoke ojb and nodeid in maps'
	smokes.append(new_somke_node->getID(), newSmoke);
	lids.append(new_lid_node->getID(), newlid);
	tabs.append(new_tab_node->getID(), newtab);

	new_somke_node->setParent(Resource::get()->box);
	new_somke_node->setTransform(initialPos_smoke);
	new_somke_node->getObjectBodyRigid()->getShape(0)->setMass(0);
	new_lid_node->setParent(new_somke_node);
	new_lid_node->setTransform(initialPos_Lid);
	new_lid_node->getObjectBodyRigid()->getShape(0)->setMass(0);
	new_tab_node->setParent(new_somke_node);
	new_tab_node->setTransform(initialPos_Tab);
	new_tab_node->getObjectBodyRigid()->getShape(0)->setMass(0);

	newtab->initialPos_tab = initialPos_Tab;
	newlid->initialPos_lid = initialPos_Lid;

}

void InteractiveVR::createNewSeamark()
{
	NodePtr new_seamark_node = Resource::get()->seamark_node->clone();
	new_seamark_node->setEnabled(1);
	ObjMovable *newseamark = new ObjMovable(new_seamark_node);
	/// new_effect_node doesn't clone the particle must use the child node that already is a partice 
	ObjectParticlesPtr news_eamarkeffect = ObjectParticles::cast(new_seamark_node->getChild(0));
	news_eamarkeffect->setEnabled(0);
	news_eamarkeffect->setEmitterEnabled(0);
	newseamark->seamark_effect = news_eamarkeffect;

	seamarks.append(new_seamark_node->getID(), newseamark);

	new_seamark_node->setParent(Resource::get()->box);
	new_seamark_node->setTransform(initialPos_seamark);
	new_seamark_node->getObjectBodyRigid()->getShape(0)->setMass(0);
}

void InteractiveVR::createNewLaserPoint()
{
	NodePtr new_laserpoint_node = Resource::get()->laserpoint_node->clone();
	new_laserpoint_node->setEnabled(1);
	ObjMovable *newlaserposint = new ObjMovable(new_laserpoint_node);

	laserpoints.append(new_laserpoint_node->getID(), newlaserposint);

	new_laserpoint_node->setParent(Resource::get()->box);
	new_laserpoint_node->setTransform(initialPos_laserpoint);
	new_laserpoint_node->getObjectBodyRigid()->getShape(0)->setMass(0);
}

void InteractiveVR::createNewFlashlight()
{
	NodePtr new_flashlight_node = Resource::get()->flashlight_node->clone();
	new_flashlight_node->setEnabled(1);
	ObjMovable *newflashlight = new ObjMovable(new_flashlight_node);

	flashlights.append(new_flashlight_node->getID(), newflashlight);
	new_flashlight_node->getObjectBodyRigid()->getShape(0)->setMass(0);
}
//smoker object update
void InteractiveVR::updateSmokeMap(Map<int, Smoke*> map)
{
	for (auto it = map.begin(); it != map.end(); ++it)
	{
		it->data->update();
		if (it->data->isTimingEffect)
		{
			if (it->data->elapsed_time_smoke > 0)
			{
				it->data->elapsed_time_smoke -= Game::get()->getIFps();
			}

			if (it->data->elapsed_time_smoke < 0)
			{
				it->data->smoke_effect->setEnabled(1);
				it->data->smoke_effect->setEmitterEnabled(1);
				it->data->elapsed_time_smoke = -1;
			}
			//3 min later disable smoke_effect
			if (it->data->isTimingEffectClosed)
			{
				if (it->data->elapsed_time_close > 0)
				{
					it->data->elapsed_time_close -= Game::get()->getIFps();
				}

				if (it->data->elapsed_time_close < 0)
				{
					it->data->smoke_effect->setEnabled(0);
					it->data->smoke_effect->setEmitterEnabled(0);
					it->data->elapsed_time_close = -1;
				}
			}
		}
	}

	
}

void InteractiveVR::updateMovableObjMap(Map<int, ObjMovable*> map)

{
	for (auto it = map.begin(); it != map.end(); ++it)
	{
		it->data->update();
		//set direction of ray 
		it->data->setRayDirection(it->data->laser_ray, vr->getHead()->getWorldPosition());
		//3 min later disable smoke_effect
		if (std::string(it->data->node->getName() )== "SeaMark")
		{
			if (it->data->isTimingEffectClosed)
			{
				if (it->data->elapsed_time_close > 0)
				{
					it->data->elapsed_time_close -= Game::get()->getIFps();
				}
		
				if (it->data->elapsed_time_close < 0)
				{
					it->data->seamark_effect->setEnabled(0);
					it->data->seamark_effect->setEmitterEnabled(0);
					it->data->elapsed_time_close = -1;
				}
			}
		}
	}

}

bool InteractiveVR::allTrownSmoke(Map<int, Smoke*> map) 
{
	for (auto it = map.begin(); it != map.end(); ++it)
	{
		if (it->data->status != Smoke::THROWN)
		{
			return false;
		}
	}
	return true;
}

Smoke* InteractiveVR::findunThrownSmoke(Map<int, Smoke*> map)
{
	for (auto it = map.begin(); it != map.end(); ++it)
	{
		if (it->data->status == Smoke::CONNECTED)
		{
			return it->data;
		}
	}
	return nullptr;
}

bool InteractiveVR::allTrownMovableObj(Map<int, ObjMovable*> map)
{
	for (auto it = map.begin(); it != map.end(); ++it)
	{
		if (it->data->status != ObjMovable::THROWN)
		{
			return false;
		}
	}
	return true;
}

ObjMovable* InteractiveVR::findunThrownMovableObj(Map<int, ObjMovable*> map)
{
	for (auto it = map.begin(); it != map.end(); ++it)
	{
		if (it->data->status == ObjMovable::CONNECTED)
		{
		
			return it->data;
		}
	}

	return nullptr;
}

void InteractiveVR::resetPosition()
{
	if (findunThrownSmoke(smokes) != nullptr)
	{
		findunThrownSmoke(smokes)->node->setTransform(initialPos_smoke);
	}
	if (findunThrownMovableObj(seamarks) != nullptr)
	{
		findunThrownMovableObj(seamarks)->node->setTransform(initialPos_seamark);
	}
	if (findunThrownMovableObj(laserpoints) != nullptr)
	{
		findunThrownMovableObj(laserpoints)->node->setTransform(initialPos_laserpoint);
	}
	
	if (findunThrownMovableObj(flashlights) != nullptr)
	{
		findunThrownMovableObj(flashlights)->node->setTransform(initialPos_flashlight);
	}
}

void InteractiveVR::destroy()
{
	vr->shutdown();
	delete vr;
}

Smoke* InteractiveVR::findSmokeinMaps(Map<int, Smoke*> map, NodePtr node)
{
	auto it = map.find(node->getID());
	if (it != map.end())
		return it->data;
	else
		return nullptr;
}

ObjMovable* InteractiveVR::findMovableObjinMaps(Map<int, ObjMovable*> map, NodePtr node)
{
	auto it = map.find(node->getID());
	if (it != map.end())
		return it->data;
	else
		return nullptr;
}

void InteractiveVR::allShowRay(Map<int, ObjMovable*> map)
{
	for (auto i = map.begin(); i != map.end(); ++i)
		 i->data->showRay();
}

void InteractiveVR::allHideRay(Map<int, ObjMovable*> map)
{
	for (auto i = map.begin(); i != map.end(); ++i)
		i->data->hideRay();
}

void InteractiveVR::allClear(Map<int, ObjMovable*> map)
{
	for (auto i = map.begin(); i != map.end(); ++i)
		i->data->clear();
}

void InteractiveVR::allDim(Map<int, ObjMovable*> map)
{
	for (auto i = map.begin(); i != map.end(); ++i)
		i->data->dim();
}

void InteractiveVR::render()
{	
	if ((Resource::get()->WinchTrigger->getScale().x > 2.8f  || App::get()->clearKeyState(106) == 1 ))
		{
			createNewItems();
			resetPosition();	
		}
	//interaction with glove
	if (vr )
	{	
		for (int k = 0; k < Glove::get()->getNumHands(); k++)
		{ 			
			int hand_state = Glove::get()->getHandState(k);
			vec3 lin_vel = Glove::get()->getHandLinearVelocity(k);

			if (hand_state != VR::HAND_FREE)
			{
				///Left_Hand_Bone or Human_RightHand_Glove
				NodePtr hand_node = Glove::get()->getHandNode(k)->getChild(0);
				
				int dof = Glove::get()->getHandDegreesOfFreedom(k);
				///must involve the speed of helicopter
				vec3 lin_vel = Glove::get()->getHandLinearVelocity(k);
				vec3 ang_vel = Glove::get()->getHandAngularVelocity(k);
			
				switch (hand_state)
 				{	
					case VR::HAND_GRAB:
						/// must convert to string
						if (std::string(Glove::get()->getGrabNode(k)->getName()) == "Handle")
						{
							handle->grabIt(Glove::get(),hand_node, k, dof, lin_vel, ang_vel);
						}
						else if (std::string(Glove::get()->getGrabNode(k)->getName()) == "Hook")
						{
							handle->getBody()->getShape(0)->setMass(0);
							if (oneTimeFlag)
							{
								hook->isHandlePositionReset = true;
								oneTimeFlag = false;
							}
							/// use hook not handle
							//handle follow the hook
							hook->followtheHook(hand_node);
							hook->grabIt(Glove::get(),hand_node, k, dof, lin_vel, ang_vel);
						}
						else if (std::string(Glove::get()->getGrabNode(k)->getName()) == "door_handle")
						{
							door->grabIt(Glove::get(), hand_node, k, dof, lin_vel, ang_vel);
						
						}
						else if (std::string(Glove::get()->getGrabNode(k)->getName()) == "door_handle_1")
						{
							door1->grabIt(Glove::get(), hand_node, k, dof, lin_vel, ang_vel);
						}
						//smoke
						else if (findSmokeinMaps(smokes, Glove::get()->getGrabNode(k)))
						{
							findSmokeinMaps(smokes, Glove::get()->getGrabNode(k))->grabIt(Glove::get(), hand_node, k, dof, lin_vel, ang_vel);
						}
						//seamark
						else if (findMovableObjinMaps(seamarks, Glove::get()->getGrabNode(k)))
						{
							findMovableObjinMaps(seamarks, Glove::get()->getGrabNode(k))->grabIt(Glove::get(), hand_node, k, dof, lin_vel, ang_vel);
						}
						//lid
						else if (findSmokeinMaps(lids, Glove::get()->getGrabNode(k)))
						{
							if (findSmokeinMaps(lids, Glove::get()->getGrabNode(k))->smoke->status == Smoke::DISCONNECTED)
							{
								findSmokeinMaps(lids, Glove::get()->getGrabNode(k))->grabIt(Glove::get(), hand_node, k, dof, lin_vel, ang_vel);
							}
							
						}
	
						//tab
						else if (findSmokeinMaps(tabs, Glove::get()->getGrabNode(k)))
						{
							if (findSmokeinMaps(tabs, Glove::get()->getGrabNode(k))->lid->status == Smoke::THROWN)
							{
								findSmokeinMaps(tabs, Glove::get()->getGrabNode(k))->grabIt(Glove::get(), hand_node, k, dof, lin_vel, ang_vel);
							}

						}
						//laserpointer
						else if (findMovableObjinMaps(laserpoints, Glove::get()->getGrabNode(k)))
						{
							findMovableObjinMaps(laserpoints, Glove::get()->getGrabNode(k))->grabIt(Glove::get(), hand_node, k, dof, lin_vel, ang_vel);
						}
						//flashlighter
						else if (findMovableObjinMaps(flashlights, Glove::get()->getGrabNode(k)))
						{
							findMovableObjinMaps(flashlights, Glove::get()->getGrabNode(k))->grabIt(Glove::get(), hand_node, k, dof, lin_vel, ang_vel);
						}

					if (grab_callback) grab_callback->run(Glove::get()->getGrabNode(k));
					break;
				case VR::HAND_HOLD:	
					//doesn't operate door
						if (std::string(Glove::get()->getGrabNode(k)->getName()) == "Handle")
						{
							handle->holdIt(Glove::get(), hand_node, k, dof, lin_vel, ang_vel);
						}
						else if (std::string(Glove::get()->getGrabNode(k)->getName()) == "Hook")
						{
							//handle positon and rotation setting

							hook->followtheHook(hand_node);
							//hook position set
							hook->holdIt(Glove::get(), hand_node, k, dof, lin_vel, ang_vel);
			
						}
						else if (std::string(Glove::get()->getGrabNode(k)->getName()) == "door_handle")
						{
							door->holdIt(Glove::get(), hand_node, k, dof, lin_vel, ang_vel);	
						}
						else if (std::string(Glove::get()->getGrabNode(k)->getName()) == "door_handle_1")
						{
							door1->holdIt(Glove::get(), hand_node, k, dof, lin_vel, ang_vel);
						}
						//smoke
						else if (findSmokeinMaps(smokes, Glove::get()->getGrabNode(k)))
						{
							findSmokeinMaps(smokes, Glove::get()->getGrabNode(k))->holdIt(Glove::get(), hand_node, k, dof, lin_vel, ang_vel);
						}
						//seamark
						else if (findMovableObjinMaps(seamarks, Glove::get()->getGrabNode(k)))
						{
							findMovableObjinMaps(seamarks, Glove::get()->getGrabNode(k))->holdIt(Glove::get(), hand_node, k, dof, lin_vel, ang_vel);
						}
						//lid
						else if (findSmokeinMaps(lids, Glove::get()->getGrabNode(k)))
						{
							if (findSmokeinMaps(lids, Glove::get()->getGrabNode(k))->smoke->status == Smoke::DISCONNECTED)
							{
								findSmokeinMaps(lids, Glove::get()->getGrabNode(k))->holdIt(Glove::get(), hand_node, k, dof, lin_vel, ang_vel);
							}

						}
						//tab
						else if (findSmokeinMaps(tabs, Glove::get()->getGrabNode(k)))
						{
							if (findSmokeinMaps(tabs, Glove::get()->getGrabNode(k))->lid->status == Smoke::THROWN)
							{
								findSmokeinMaps(tabs, Glove::get()->getGrabNode(k))->holdIt(Glove::get(), hand_node, k, dof, lin_vel, ang_vel);
							}

						}
						//laserpointer 
						else if (findMovableObjinMaps(laserpoints, Glove::get()->getGrabNode(k)))
						{
							findMovableObjinMaps(laserpoints, Glove::get()->getGrabNode(k))->holdIt(Glove::get(), hand_node, k, dof, lin_vel, ang_vel);
							//use or turn off it
							if (k==0  || Glove::get()->isUseLightLeft)
							{
								findMovableObjinMaps(laserpoints, Glove::get()->getGrabNode(k))->useIt(Glove::get()->isUseLightLeft);
							//	if (use_callback) use_callback->run(Glove::get()->getGrabNode(k));
							}
					
							if (k == 1 || Glove::get()->isUseLightRight)
							{
								findMovableObjinMaps(laserpoints, Glove::get()->getGrabNode(k))->useIt(Glove::get()->isUseLightRight);
							//	if (use_callback) use_callback->run(Glove::get()->getGrabNode(k));
							}
						}
						//flashlight 
						else if (findMovableObjinMaps(flashlights, Glove::get()->getGrabNode(k)))
						{
							findMovableObjinMaps(flashlights, Glove::get()->getGrabNode(k))->holdIt(Glove::get(), hand_node, k, dof, lin_vel, ang_vel);
							//use or turn off it
							if (vr->getControllerButtonDown(k, VR::BUTTON::MENU))
							{
								findMovableObjinMaps(flashlights, Glove::get()->getGrabNode(k))->useIt(vr->getControllerButtonDown(k, VR::BUTTON::MENU));
								if (use_callback) use_callback->run(Glove::get()->getGrabNode(k));
							}
						}

					if (hold_callback) hold_callback->run(Glove::get()->getGrabNode(k));
					break;

				case VR::HAND_THROW:
						if (std::string(Glove::get()->getGrabNode(k)->getName()) == "Handle")
						{
							handle->throwIt(Glove::get(), hand_node, k, dof, lin_vel, ang_vel);
						}
						else if (std::string(Glove::get()->getGrabNode(k)->getName()) == "Hook")
						{
							handle->getBody()->getShape(0)->setMass(100);
							hook->throwIt(Glove::get(), hand_node, k, dof, lin_vel, ang_vel);
							oneTimeFlag = true;
						}
						else if (std::string(Glove::get()->getGrabNode(k)->getName()) == "door_handle")
						{
							door->throwIt(Glove::get(), hand_node, k, dof, lin_vel, ang_vel);
						}
						else if (std::string(Glove::get()->getGrabNode(k)->getName()) == "door_handle_1")
						{
							door1->throwIt(Glove::get(), hand_node, k, dof, lin_vel, ang_vel);
						}
						//seamark
						else if (findMovableObjinMaps(seamarks, Glove::get()->getGrabNode(k)))
						{
							findMovableObjinMaps(seamarks, Glove::get()->getGrabNode(k))->throwIt(Glove::get(), hand_node, k, dof, lin_vel, ang_vel);
						}

						//smoke
						else if (findSmokeinMaps(smokes, Glove::get()->getGrabNode(k)))
						{
							findSmokeinMaps(smokes, Glove::get()->getGrabNode(k))->throwIt(Glove::get(), hand_node, k, dof, lin_vel, ang_vel);
						}
	
						//lid
						else if (findSmokeinMaps(lids, Glove::get()->getGrabNode(k)))
						{
							if ((findSmokeinMaps(lids, Glove::get()->getGrabNode(k))->smoke->status == Smoke::DISCONNECTED
								|| findSmokeinMaps(lids, Glove::get()->getGrabNode(k))->smoke->status == Smoke::THROWN)
								&& findSmokeinMaps(lids, Glove::get()->getGrabNode(k))->status == Smoke::DISCONNECTED)
							{
								findSmokeinMaps(lids, Glove::get()->getGrabNode(k))->throwIt(Glove::get(), hand_node, k, dof, lin_vel, ang_vel);
								findSmokeinMaps(lids, Glove::get()->getGrabNode(k))->node->setEnabled(0);
							}

						}
			
						//tab
						else if (findSmokeinMaps(tabs, Glove::get()->getGrabNode(k)))
						{
							if (findSmokeinMaps(tabs, Glove::get()->getGrabNode(k))->lid->status == Smoke::THROWN)
							{
								findSmokeinMaps(tabs, Glove::get()->getGrabNode(k))->throwIt(Glove::get(), hand_node, k, dof, lin_vel, ang_vel);
								findSmokeinMaps(tabs, Glove::get()->getGrabNode(k))->node->setEnabled(0);
							}
						}
						//laserpointer
						else if (findMovableObjinMaps(laserpoints, Glove::get()->getGrabNode(k)))
						{
							findMovableObjinMaps(laserpoints, Glove::get()->getGrabNode(k))->throwIt(Glove::get(), hand_node, k, dof, lin_vel, ang_vel);
						}
						//flashlights
						else if (findMovableObjinMaps(flashlights, Glove::get()->getGrabNode(k)))
						{
							findMovableObjinMaps(flashlights, Glove::get()->getGrabNode(k))->throwIt(Glove::get(), hand_node, k, dof, lin_vel, ang_vel);
						}

						createNewItems();

					if (throw_callback) throw_callback->run(Glove::get()->getGrabNode(k));
					break;
				default:
					break;
				}

				hand_node->setEnabled(0);
			}

		}
		vr->update_button_states();


	}
	//interaction with leapmotion
	///interactin use glove, leapmotion is just a graphic show 
/*
	if (vr)
	{
		for (int k = 0; k < LeapMotion::get()->getNumHands(); k++)
		{
			int hand_state = LeapMotion::get()->getHandState(k);
			vec3 lin_vel = LeapMotion::get()->getHandLinearVelocity(k);

			if (hand_state != VR::HAND_FREE)
			{
				NodePtr hand_node = LeapMotion::get()->getHandNode(k);
				
				if (hand_node)
				{
					int dof = LeapMotion::get()->getHandDegreesOfFreedom(k);
					///must involve the speed of helicopter
					vec3 lin_vel = LeapMotion::get()->getHandLinearVelocity(k);
					vec3 ang_vel = LeapMotion::get()->getHandAngularVelocity(k);

					switch (hand_state)
					{
					case VR::HAND_GRAB:
						/// must convert to string
						if (std::string(LeapMotion::get()->getGrabNode(k)->getName()) == "Handle")
						{
							handle->grabIt(LeapMotion::get(), hand_node, k, dof, lin_vel, ang_vel);
						}
						else if (std::string(LeapMotion::get()->getGrabNode(k)->getName()) == "Hook")
						{
							handle->getBody()->getShape(0)->setMass(0);
							if (oneTimeFlag)
							{
								hook->isHandlePositionReset = true;
								oneTimeFlag = false;
							}
							/// use hook not handle
							//handle follow the hook
							hook->followtheHook(hand_node);
							hook->grabIt(LeapMotion::get(), hand_node, k, dof, lin_vel, ang_vel);
						}
						else if (std::string(LeapMotion::get()->getGrabNode(k)->getName()) == "door_handle")
						{
							door->grabIt(LeapMotion::get(), hand_node, k, dof, lin_vel, ang_vel);

						}
						else if (std::string(LeapMotion::get()->getGrabNode(k)->getName()) == "door_handle_1")
						{
							door1->grabIt(LeapMotion::get(), hand_node, k, dof, lin_vel, ang_vel);
						}
						//smoke
						else if (findSmokeinMaps(smokes, LeapMotion::get()->getGrabNode(k)))
						{
							findSmokeinMaps(smokes, LeapMotion::get()->getGrabNode(k))->grabIt(LeapMotion::get(), hand_node, k, dof, lin_vel, ang_vel);
						}
						//seamark
						else if (findMovableObjinMaps(seamarks, LeapMotion::get()->getGrabNode(k)))
						{
							findMovableObjinMaps(seamarks, LeapMotion::get()->getGrabNode(k))->grabIt(LeapMotion::get(), hand_node, k, dof, lin_vel, ang_vel);
						}
						//lid
						else if (findSmokeinMaps(lids, LeapMotion::get()->getGrabNode(k)))
						{
							if (findSmokeinMaps(lids, LeapMotion::get()->getGrabNode(k))->smoke->status == Smoke::DISCONNECTED)
							{
								findSmokeinMaps(lids, LeapMotion::get()->getGrabNode(k))->grabIt(LeapMotion::get(), hand_node, k, dof, lin_vel, ang_vel);
							}

						}

						//tab
						else if (findSmokeinMaps(tabs, LeapMotion::get()->getGrabNode(k)))
						{
							if (findSmokeinMaps(tabs, LeapMotion::get()->getGrabNode(k))->lid->status == Smoke::THROWN)
							{
								findSmokeinMaps(tabs, LeapMotion::get()->getGrabNode(k))->grabIt(LeapMotion::get(), hand_node, k, dof, lin_vel, ang_vel);
							}

						}
						//laserpointer
						else if (findMovableObjinMaps(laserpoints, LeapMotion::get()->getGrabNode(k)))
						{
							findMovableObjinMaps(laserpoints, LeapMotion::get()->getGrabNode(k))->grabIt(LeapMotion::get(), hand_node, k, dof, lin_vel, ang_vel);
						}
						//flashlighter
						else if (findMovableObjinMaps(flashlights, LeapMotion::get()->getGrabNode(k)))
						{
							findMovableObjinMaps(flashlights, LeapMotion::get()->getGrabNode(k))->grabIt(LeapMotion::get(), hand_node, k, dof, lin_vel, ang_vel);
						}

						if (grab_callback) grab_callback->run(LeapMotion::get()->getGrabNode(k));
						break;
					case VR::HAND_HOLD:

						if (std::string(LeapMotion::get()->getGrabNode(k)->getName()) == "Handle")
						{
							handle->holdIt(LeapMotion::get(), hand_node, k, dof, lin_vel, ang_vel);
						}
						else if (std::string(LeapMotion::get()->getGrabNode(k)->getName()) == "Hook")
						{
							//handle positon and rotation setting

							hook->followtheHook(hand_node);
							//hook position set
							hook->holdIt(LeapMotion::get(), hand_node, k, dof, lin_vel, ang_vel);

						}
						else if (std::string(LeapMotion::get()->getGrabNode(k)->getName()) == "door_handle")
						{
							door->holdIt(LeapMotion::get(), hand_node, k, dof, lin_vel, ang_vel);
						}
						else if (std::string(LeapMotion::get()->getGrabNode(k)->getName()) == "door_handle_1")
						{
							door1->holdIt(LeapMotion::get(), hand_node, k, dof, lin_vel, ang_vel);
						}
						//smoke
						else if (findSmokeinMaps(smokes, LeapMotion::get()->getGrabNode(k)))
						{
							findSmokeinMaps(smokes, LeapMotion::get()->getGrabNode(k))->holdIt(LeapMotion::get(), hand_node, k, dof, lin_vel, ang_vel);
						}
						//seamark
						else if (findMovableObjinMaps(seamarks, LeapMotion::get()->getGrabNode(k)))
						{
							findMovableObjinMaps(seamarks, LeapMotion::get()->getGrabNode(k))->holdIt(LeapMotion::get(), hand_node, k, dof, lin_vel, ang_vel);
						}
						//lid
						else if (findSmokeinMaps(lids, LeapMotion::get()->getGrabNode(k)))
						{
							if (findSmokeinMaps(lids, LeapMotion::get()->getGrabNode(k))->smoke->status == Smoke::DISCONNECTED)
							{
								findSmokeinMaps(lids, LeapMotion::get()->getGrabNode(k))->holdIt(LeapMotion::get(), hand_node, k, dof, lin_vel, ang_vel);
							}

						}
						//tab
						else if (findSmokeinMaps(tabs, LeapMotion::get()->getGrabNode(k)))
						{
							if (findSmokeinMaps(tabs, LeapMotion::get()->getGrabNode(k))->lid->status == Smoke::THROWN)
							{
								findSmokeinMaps(tabs, LeapMotion::get()->getGrabNode(k))->holdIt(LeapMotion::get(), hand_node, k, dof, lin_vel, ang_vel);
							}

						}
						//laserpointer 
						else if (findMovableObjinMaps(laserpoints, LeapMotion::get()->getGrabNode(k)))
						{
							findMovableObjinMaps(laserpoints, LeapMotion::get()->getGrabNode(k))->holdIt(LeapMotion::get(), hand_node, k, dof, lin_vel, ang_vel);
							//use or turn off it
							if (vr->getControllerButtonDown(k, VR::BUTTON::MENU))
							{
								findMovableObjinMaps(laserpoints, LeapMotion::get()->getGrabNode(k))->useIt();
								if (use_callback) use_callback->run(LeapMotion::get()->getGrabNode(k));
							}
						}
						//flashlight 
						else if (findMovableObjinMaps(flashlights, LeapMotion::get()->getGrabNode(k)))
						{
							findMovableObjinMaps(flashlights, LeapMotion::get()->getGrabNode(k))->holdIt(LeapMotion::get(), hand_node, k, dof, lin_vel, ang_vel);
							//use or turn off it
							if (vr->getControllerButtonDown(k, VR::BUTTON::MENU))
							{
								findMovableObjinMaps(flashlights, LeapMotion::get()->getGrabNode(k))->useIt();
								if (use_callback) use_callback->run(LeapMotion::get()->getGrabNode(k));
							}
						}

						if (hold_callback) hold_callback->run(LeapMotion::get()->getGrabNode(k));
						break;

					case VR::HAND_THROW:

						if (std::string(LeapMotion::get()->getGrabNode(k)->getName()) == "Handle")
						{
							handle->throwIt(LeapMotion::get(), hand_node, k, dof, lin_vel, ang_vel);
						}
						else if (std::string(LeapMotion::get()->getGrabNode(k)->getName()) == "Hook")
						{
							handle->getBody()->getShape(0)->setMass(100);
							hook->throwIt(LeapMotion::get(), hand_node, k, dof, lin_vel, ang_vel);
							oneTimeFlag = true;
						}
						else if (std::string(LeapMotion::get()->getGrabNode(k)->getName()) == "door_handle")
						{
							door->throwIt(LeapMotion::get(), hand_node, k, dof, lin_vel, ang_vel);
						}
						else if (std::string(LeapMotion::get()->getGrabNode(k)->getName()) == "door_handle_1")
						{
							door1->throwIt(LeapMotion::get(), hand_node, k, dof, lin_vel, ang_vel);
						}
						//seamark
						else if (findMovableObjinMaps(seamarks, LeapMotion::get()->getGrabNode(k)))
						{
							findMovableObjinMaps(seamarks, LeapMotion::get()->getGrabNode(k))->throwIt(LeapMotion::get(), hand_node, k, dof, lin_vel, ang_vel);
						}

						//smoke
						else if (findSmokeinMaps(smokes, LeapMotion::get()->getGrabNode(k)))
						{
							findSmokeinMaps(smokes, LeapMotion::get()->getGrabNode(k))->throwIt(LeapMotion::get(), hand_node, k, dof, lin_vel, ang_vel);
						}

						//lid
						else if (findSmokeinMaps(lids, LeapMotion::get()->getGrabNode(k)))
						{
							if ((findSmokeinMaps(lids, LeapMotion::get()->getGrabNode(k))->smoke->status == Smoke::DISCONNECTED
								|| findSmokeinMaps(lids, LeapMotion::get()->getGrabNode(k))->smoke->status == Smoke::THROWN)
								&& findSmokeinMaps(lids, LeapMotion::get()->getGrabNode(k))->status == Smoke::DISCONNECTED)
							{
								findSmokeinMaps(lids, LeapMotion::get()->getGrabNode(k))->throwIt(LeapMotion::get(), hand_node, k, dof, lin_vel, ang_vel);
								findSmokeinMaps(lids, LeapMotion::get()->getGrabNode(k))->node->setEnabled(0);
							}

						}

						//tab
						else if (findSmokeinMaps(tabs, LeapMotion::get()->getGrabNode(k)))
						{
							if (findSmokeinMaps(tabs, LeapMotion::get()->getGrabNode(k))->lid->status == Smoke::THROWN)
							{
								findSmokeinMaps(tabs, LeapMotion::get()->getGrabNode(k))->throwIt(LeapMotion::get(), hand_node, k, dof, lin_vel, ang_vel);
								findSmokeinMaps(tabs, LeapMotion::get()->getGrabNode(k))->node->setEnabled(0);
							}
						}
						//laserpointer
						else if (findMovableObjinMaps(laserpoints, LeapMotion::get()->getGrabNode(k)))
						{
							findMovableObjinMaps(laserpoints, LeapMotion::get()->getGrabNode(k))->throwIt(LeapMotion::get(), hand_node, k, dof, lin_vel, ang_vel);
						}
						//flashlights
						else if (findMovableObjinMaps(flashlights, LeapMotion::get()->getGrabNode(k)))
						{
							findMovableObjinMaps(flashlights, LeapMotion::get()->getGrabNode(k))->throwIt(LeapMotion::get(), hand_node, k, dof, lin_vel, ang_vel);
						}

						if (throw_callback) throw_callback->run(LeapMotion::get()->getGrabNode(k));
						break;
					default:
						break;
					}

					hand_node->setEnabled(0);
				}

			}

		}
		vr->update_button_states();


	}


	*/
	//interaction with controller
	/*
	if (vr)
{
	for (int k = 0; k < vr->getNumHands(); k++)
	{
		int hand_state = vr->getHandState(k);
		vec3 lin_vel = vr->getHandLinearVelocity(k);

		if (hand_state != VR::HAND_FREE)
		{
			int grab_node_id = vr->getGrabNode(k)->getID();
			NodePtr hand_node = vr->getHandNode(k);
			int dof = vr->getHandDegreesOfFreedom(k);
			///must involve the speed of helicopter
			vec3 lin_vel = vr->getHandLinearVelocity(k);

			//	Log::message("%f %f %f \n", lin_vel.x*3, lin_vel.y * 3, lin_vel.z * 3);
			//	Log::message("%f %f %f helicopterSpeed\n", helicopterSpeedX, helicopterSpeedY, helicopterSpeedZ);
			vec3 ang_vel = vr->getHandAngularVelocity(k);

			switch (hand_state)
			{
			case VR::HAND_GRAB:
				/// must convert to string
				if (handle->node->getID() == vr->getGrabNode(k)->getID())
				{
					handle->grabIt(vr, hand_node, k, dof, lin_vel, ang_vel);
				}
				else if (hook->node->getID() == vr->getGrabNode(k)->getID())
				{
					handle->getBody()->getShape(0)->setMass(0);
					if (oneTimeFlag)
					{
						hook->isHandlePositionReset = true;
						oneTimeFlag = false;
					}
					/// use hook not handle
					//handle follow the hook
					hook->followtheHook(hand_node, handle->node);
					hook->grabIt(vr, hand_node, k, dof, lin_vel, ang_vel);
				}
				else if (std::string(vr->getGrabNode(k)->getName()) == "door_handle")
				{
					door->grabIt(vr, hand_node, k, dof, lin_vel, ang_vel);
				}
				else if (std::string(vr->getGrabNode(k)->getName()) == "door_handle_1")
				{
					door1->grabIt(vr, hand_node, k, dof, lin_vel, ang_vel);
				}
				//smoke
				else if (findSmokeinMaps(smokes, vr->getGrabNode(k)))
				{
					findSmokeinMaps(smokes, vr->getGrabNode(k))->grabIt(vr, hand_node, k, dof, lin_vel, ang_vel);
				}
				//seamark
				else if (findMovableObjinMaps(seamarks, vr->getGrabNode(k)))
				{
					findMovableObjinMaps(seamarks, vr->getGrabNode(k))->grabIt(vr, hand_node, k, dof, lin_vel, ang_vel);
				}
				//lid
				else if (findSmokeinMaps(lids, vr->getGrabNode(k)))
				{
					if (findSmokeinMaps(lids, vr->getGrabNode(k))->smoke->status == Smoke::DISCONNECTED)
					{
						findSmokeinMaps(lids, vr->getGrabNode(k))->grabIt(vr, hand_node, k, dof, lin_vel, ang_vel);
					}

				}

				//tab
				else if (findSmokeinMaps(tabs, vr->getGrabNode(k)))
				{
					if (findSmokeinMaps(tabs, vr->getGrabNode(k))->lid->status == Smoke::THROWN)
					{
						findSmokeinMaps(tabs, vr->getGrabNode(k))->grabIt(vr, hand_node, k, dof, lin_vel, ang_vel);
					}

				}
				//laserpointer
				else if (findMovableObjinMaps(laserpoints, vr->getGrabNode(k)))
				{
					findMovableObjinMaps(laserpoints, vr->getGrabNode(k))->grabIt(vr, hand_node, k, dof, lin_vel, ang_vel);
				}
				//flashlighter
				else if (findMovableObjinMaps(flashlights, vr->getGrabNode(k)))
				{
					findMovableObjinMaps(flashlights, vr->getGrabNode(k))->grabIt(vr, hand_node, k, dof, lin_vel, ang_vel);
				}

				if (grab_callback) grab_callback->run(vr->getGrabNode(k));
				break;
			case VR::HAND_HOLD:
				//doesn't operate door

				if (handle->node->getID() == vr->getGrabNode(k)->getID())
				{
					handle->holdIt(vr, hand_node, k, dof, lin_vel, ang_vel);
				}
				else if (hook->node->getID() == vr->getGrabNode(k)->getID())
				{
					//handle positon and rotation setting

					hook->followtheHook(hand_node, handle->node);
					//hook position set
					hook->holdIt(vr, hand_node, k, dof, lin_vel, ang_vel);
					//if touch the weight
					//	if (hook->hit_detect(hook->weights))
					//	{
					//		if (vr->attachWeight)
					//			hook->attach(hook->weight);
					//		if (vr->detachWeight)
					//			hook->detach(hook->weight);
					//	}
				}
				else if (std::string(vr->getGrabNode(k)->getName()) == "door_handle")
				{
					door->holdIt(vr, hand_node, k, dof, lin_vel, ang_vel);
				}
				else if (std::string(vr->getGrabNode(k)->getName()) == "door_handle_1")
				{
					door1->holdIt(vr, hand_node, k, dof, lin_vel, ang_vel);
				}
				//smoke
				else if (findSmokeinMaps(smokes, vr->getGrabNode(k)))
				{
					findSmokeinMaps(smokes, vr->getGrabNode(k))->holdIt(vr, hand_node, k, dof, lin_vel, ang_vel);
				}
				//seamark
				else if (findMovableObjinMaps(seamarks, vr->getGrabNode(k)))
				{
					findMovableObjinMaps(seamarks, vr->getGrabNode(k))->holdIt(vr, hand_node, k, dof, lin_vel, ang_vel);
				}
				//lid
				else if (findSmokeinMaps(lids, vr->getGrabNode(k)))
				{
					if (findSmokeinMaps(lids, vr->getGrabNode(k))->smoke->status == Smoke::DISCONNECTED)
					{
						findSmokeinMaps(lids, vr->getGrabNode(k))->holdIt(vr, hand_node, k, dof, lin_vel, ang_vel);
					}

				}
				//tab
				else if (findSmokeinMaps(tabs, vr->getGrabNode(k)))
				{
					if (findSmokeinMaps(tabs, vr->getGrabNode(k))->lid->status == Smoke::THROWN)
					{
						findSmokeinMaps(tabs, vr->getGrabNode(k))->holdIt(vr, hand_node, k, dof, lin_vel, ang_vel);
					}

				}
				//laserpointer 
				else if (findMovableObjinMaps(laserpoints, vr->getGrabNode(k)))
				{
					findMovableObjinMaps(laserpoints, vr->getGrabNode(k))->holdIt(vr, hand_node, k, dof, lin_vel, ang_vel);
					//use or turn off it
					if (vr->getControllerButtonDown(k, VR::BUTTON::MENU))
					{
						findMovableObjinMaps(laserpoints, vr->getGrabNode(k))->useIt();
						if (use_callback) use_callback->run(vr->getGrabNode(k));
					}
				}
				//flashlight 
				else if (findMovableObjinMaps(flashlights, vr->getGrabNode(k)))
				{
					findMovableObjinMaps(flashlights, vr->getGrabNode(k))->holdIt(vr, hand_node, k, dof, lin_vel, ang_vel);
					//use or turn off it
					if (vr->getControllerButtonDown(k, VR::BUTTON::MENU))
					{
						findMovableObjinMaps(flashlights, vr->getGrabNode(k))->useIt();
						if (use_callback) use_callback->run(vr->getGrabNode(k));
					}
				}

				if (hold_callback) hold_callback->run(vr->getGrabNode(k));
				break;

			case VR::HAND_THROW:

				if (handle->node->getID() == vr->getGrabNode(k)->getID())
				{
					handle->throwIt(vr, hand_node, k, dof, lin_vel, ang_vel);
					handle->getBody()->getShape(0)->setMass(Resource::get()->handle_mass);
				}
				else if (hook->node->getID() == vr->getGrabNode(k)->getID())
				{
					hook->throwIt(vr, hand_node, k, dof, lin_vel, ang_vel);
					hook->getBody()->getShape(0)->setMass(Resource::get()->hook_mass);
					handle->getBody()->getShape(0)->setMass(Resource::get()->handle_mass);
					oneTimeFlag = true;
				}
				else if (std::string(vr->getGrabNode(k)->getName()) == "door_handle")
				{
					door->throwIt(vr, hand_node, k, dof, lin_vel, ang_vel);
				}
				else if (std::string(vr->getGrabNode(k)->getName()) == "door_handle_1")
				{
					door1->throwIt(vr, hand_node, k, dof, lin_vel, ang_vel);
				}
				//seamark
				else if (findMovableObjinMaps(seamarks, vr->getGrabNode(k)))
				{
					findMovableObjinMaps(seamarks, vr->getGrabNode(k))->throwIt(vr, hand_node, k, dof, lin_vel, ang_vel);
				}

				//smoke
				else if (findSmokeinMaps(smokes, vr->getGrabNode(k)))
				{
					findSmokeinMaps(smokes, vr->getGrabNode(k))->throwIt(vr, hand_node, k, dof, lin_vel, ang_vel);
				}

				//lid
				else if (findSmokeinMaps(lids, vr->getGrabNode(k)))
				{
					if ((findSmokeinMaps(lids, vr->getGrabNode(k))->smoke->status == Smoke::DISCONNECTED
						|| findSmokeinMaps(lids, vr->getGrabNode(k))->smoke->status == Smoke::THROWN)
						&& findSmokeinMaps(lids, vr->getGrabNode(k))->status == Smoke::DISCONNECTED)
					{
						findSmokeinMaps(lids, vr->getGrabNode(k))->throwIt(vr, hand_node, k, dof, lin_vel, ang_vel);
						findSmokeinMaps(lids, vr->getGrabNode(k))->node->setEnabled(0);
					}

				}

				//tab
				else if (findSmokeinMaps(tabs, vr->getGrabNode(k)))
				{
					if (findSmokeinMaps(tabs, vr->getGrabNode(k))->lid->status == Smoke::THROWN)
					{
						findSmokeinMaps(tabs, vr->getGrabNode(k))->throwIt(vr, hand_node, k, dof, lin_vel, ang_vel);
						findSmokeinMaps(tabs, vr->getGrabNode(k))->node->setEnabled(0);
					}
				}
				//laserpointer
				else if (findMovableObjinMaps(laserpoints, vr->getGrabNode(k)))
				{
					findMovableObjinMaps(laserpoints, vr->getGrabNode(k))->throwIt(vr, hand_node, k, dof, lin_vel, ang_vel);
				}
				//flashlights
				else if (findMovableObjinMaps(flashlights, vr->getGrabNode(k)))
				{
					findMovableObjinMaps(flashlights, vr->getGrabNode(k))->throwIt(vr, hand_node, k, dof, lin_vel, ang_vel);
				}

				if (throw_callback) throw_callback->run(vr->getGrabNode(k));
				break;
			default:
				break;
			}

		}
	}
	vr->update_button_states();
}
*/
	
	
	 vr->controller_tracker[3]->setEnabled(1);
}





