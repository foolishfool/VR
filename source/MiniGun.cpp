#include "MiniGun.h"

const float CHANGE_INTERVAL = 0.01f; // the interval of timekeeping
float elapsed_time_fire = CHANGE_INTERVAL;	// current time left 
const float CHANGE_INTERVAL_EFFECT = 0.06f;
float elapsed_time_effect = CHANGE_INTERVAL_EFFECT;	// current time left 

const float CHANGE_INTERVAL_SPEED = 0.1f; // the interval between changes of position, in seconds
float elapsed_time_speed = CHANGE_INTERVAL_SPEED;	// current time left to change current scale of our objects

namespace Paths
{
	const char *terrain_explosion_path = "nodes/explosion_gun_effects.node";
	const char *oil_tank_explosion_path = "nodes/explosion_oil_tank_effects.node";
	const char *water_explosion_path = "nodes/explosion_water_effects.node";
}


MiniGun* MiniGun::instance;

MiniGun* MiniGun::get()
{
	return instance;
}

void MiniGun::init()
{

	instance = this;

	Resource::get()->shoot_effect_muzzle_flash->setEnabled(0);
	Resource::get()->shoot_effect_barrel_smoke1->setEnabled(1);
	Resource::get()->shoot_effect_barrel_smoke1->setEmitterEnabled(0);
	Resource::get()->shoot_effect_barrel_smoke2->setEnabled(1);
	Resource::get()->shoot_effect_barrel_smoke2->setEmitterEnabled(0);

	auto editor = Editor::get();

	for (int i = 0; i < editor->getNumNodes(); i++)
	{
		auto node = editor->getNode(i);
		if (!node->isEnabled())
			continue;

		auto node_name = node->getName();
		if (!node_name)
			continue;
		if (std::string(node_name) == "oil_tank")
		{
			NodeReferencePtr node_ref = NodeReference::cast(node);
			oil_tanks.append(node_ref);
		}

	}
	//used to calcuate the minigun moving speed
	lastposition = Resource::get()->vive_tracker_0->getWorldPosition();
}

void MiniGun::update() 
{

	currentposition = Resource::get()->vive_tracker_0->getWorldPosition();
	// checking if it's time to change current position
	if (elapsed_time_speed < 0.0f)
	{
		helicopter_speed = sqrtf(powf((currentposition.x - lastposition.x), 2) + powf((currentposition.y - lastposition.y), 2) + powf((currentposition.z - lastposition.z), 2));
		lastposition = currentposition;
		// resetting elapsed time counter
		elapsed_time_speed = CHANGE_INTERVAL_SPEED;
	}
	elapsed_time_speed -= Game::get()->getIFps();
	if (isFiring && !isStoped)
	{
		isFiring = false;
		isStoped = true;
		isStatic = false;
		Resource::get()->sound_fire_loop->stop();
		Resource::get()->shoot_effect_barrel_smoke1->setEmitterEnabled(0);
		Resource::get()->shoot_effect_barrel_smoke2->setEmitterEnabled(0);
		Resource::get()->sound_fire_down->play();
	}

//update explosion
	for (size_t i = 0, maxi = explosions.size(); i < maxi;)
	{
		auto ref = explosions[i];
		if (!ref->isEnabled())
		{
			delete ref;
			explosions.erase(explosions.begin() + i);
			--maxi;
			continue;
		}
		ref->update(Game::get()->getIFps());
		++i;
	}

	if (isStoped && !isFiring && !isStatic)
	{
		round -= 15;

		if (round > 0)
		{
			RotateNode(Resource::get()->BarrelRotatorParent, Math::quat(0, 1, 0, round * (-360) * Game::get()->getIFps()));
		}
		else
		{
			isStoped = false;
			isStatic = true;
			round = 0;
		}
	}
	//reset oil_tank key Y

	if (App::get()->getKeyState(121) == 1)
	{
		for (int i = 0; i < oil_tanks.size(); i++)
		{
			if (oil_tanks[i]->getReference()->getChild(0)->isEnabled() == 1)
			{
				ObjectPtr obj = Object::cast(oil_tanks[i]->getReference());
				for (int i = 0; i < obj->getNumSurfaces(); i++)
				{
					obj->setEnabled(1, i);
				}
				oil_tanks[i]->getReference()->getChild(0)->setEnabled(0);
			}
		}
	}

	Resource::get()->bulletNumberText->setText((std::to_string(bulletNumber)).c_str());	
}

void MiniGun::fire()
{
	gun_dir = getBulletDirection();
	gun_pos = Resource::get()->barrel_1->getWorldPosition();

	const Vec3 &p0 = gun_pos;
	const Vec3 &p1 = p0 + gun_dir * 5000.0f;
	const Vec3 &direction = normalize(p1 - p0);
	const char *path = Paths::terrain_explosion_path;
	int intersection = World::get()->getIntersection(p0, p1, hitObjs);
	
	//create_shell();

	if (round < 600)
	{
		round += 15;
		Resource::get()->sound_fire_loop->play();
	}

	Resource::get()->shoot_effect_barrel_smoke1->setEmitterEnabled(1);
	Resource::get()->shoot_effect_barrel_smoke2->setEmitterEnabled(1);

	RotateNode(Resource::get()->BarrelRotatorParent, Math::quat(0, 1, 0, round *  (-360) * Game::get()->getIFps()));

	isFiring = true;
	isStatic = false;
	isStoped = false;

	if (elapsed_time_fire < 0.0f)
	{
		elapsed_time_fire = CHANGE_INTERVAL;
		
		bullet_effect_number += 1;
		
		if (intersection)
		{
			create_tracer(vec3(direction),1);

			ObjectIntersectionPtr object_intersection = ObjectIntersection::create();
			for (auto hitObj : hitObjs)
			{
			
				const vec3 &lp0 = (vec3)(hitObj->getIWorldTransform()* p0);
				const vec3 &lp1 = (vec3)(hitObj->getIWorldTransform()* p1);
				if (!hitObj->getIntersection(lp0, lp1, ~0, object_intersection, nullptr))
					continue;
				explosionPoint = hitObj->getWorldTransform() * (Vec3)(object_intersection->getPoint());
				time_to_explode = static_cast<float>(length(explosionPoint - gun_pos) / bulletSpeed);
				bool damage_terrain = false;

				/// the sequence is important the last is water

				if (hitObj->getType() == Node::OBJECT_MESH_STATIC )
				{
					if (std::string( hitObj->getNode()->getName()) == "oil_tank")
					{
					//	path = Paths::oil_tank_explosion_path;
					//	damage_terrain = true;	
					//	ReliefEditor::get()->addCrater(hitObj->getNode()->getWorldPosition() , Math::Vec2(8, 8), static_cast<float>(4) * 0.2f);
					//	NodePtr effect = Resource::get()->explosion_effect->clone(); 
					//	effect->setWorldPosition(hitObj->getNode()->getWorldPosition());
						hitObj->getNode()->getChild(0)->setEnabled(1);
						Resource::get()->sound_explosion->play();
						for (int i = 0; i < hitObj->getNumSurfaces(); i++)
						{
							hitObj->setEnabled(0,i);
						}
						hitObj->getNode()->setEnabled(1);
					}				
				}
				if (hitObj->getType() == Node::OBJECT_TERRAIN_GLOBAL)
				{
					path = Paths::terrain_explosion_path;
					//	damage_terrain = true
				} 
				if (hitObj->getType() == Node::OBJECT_WATER_GLOBAL)
				{
					path = Paths::water_explosion_path;
				
					//	damage_terrain = true;
				}	

				create_explosion(path, explosionPoint, explosionRadius, direction, damage_terrain);
		
				
			}
		}
	}

	if (bullet_effect_number >0 && bullet_effect_number % 2 == 0 )
	{
		elapsed_time_effect = CHANGE_INTERVAL_EFFECT;
		NodePtr new_effect = Resource::get()->shoot_effect_muzzle_flash->clone();	
		new_effect->release();
		//synchronize to the slave
		WorldSyncker::get()->syncker_master->createNode(new_effect, 1,1);

		//only tracer will be synchronized
		WorldSyncker::get()->syncker_master->createNode(new_effect->getChild(1), 1, 1);

		new_effect->setEnabled(1);
		Resource::get()->shoot_effect_muzzle_flash->getParent()->removeWorldChild(new_effect);
		effects.append(new_effect);
		//if helicopter too fast the effect doesn't show
		if (helicopter_speed >1)
		{
			new_effect->setEnabled(0);
		}

	}

	elapsed_time_fire -= Game::get()->getIFps();
	elapsed_time_effect -= Game::get()->getIFps();
	
	elapsed_time_num += Game::get()->getIFps();
	bulletNumber = int(elapsed_time_num * 100);

	//keep the size of the daynamically created nodes
	if (effects.size()>30)
	{
		WorldSyncker::get()->syncker_master->removeSyncNode(WorldSyncker::get()->syncker_master->getSyncNodeNum(effects[0]));
		///only created the child1 so only delete the child 1 not 0
		WorldSyncker::get()->syncker_master->removeSyncNode(WorldSyncker::get()->syncker_master->getSyncNodeNum(effects[0]->getChild(1)));
			
		Editor::get()->removeNode(effects[0]);
		effects.remove(0);
	}
	
	///as everytime the number of explosions created is different, so use loop 
		while (explosion_nodes.size()>60)
		{
			for (int i = 0; i < explosion_nodes.size(); i++)
			{
				WorldSyncker::get()->syncker_master->removeSyncNode(WorldSyncker::get()->syncker_master->getSyncNodeNum(explosion_nodes[i]));
				for (int j = 0; j < explosion_nodes[i]->getNumChildren(); j++)
				{
					WorldSyncker::get()->syncker_master->removeSyncNode(WorldSyncker::get()->syncker_master->getSyncNodeNum(explosion_nodes[i]->getChild(j)));
				}
				Editor::get()->removeNode(explosion_nodes[i]);
				explosion_nodes.remove(i);
			}
		}
	
}

void MiniGun::create_explosion(const char *path, Vec3 explosionPoint, double explosionRadius, Vec3 direction, bool damage_terrain)
{
	Explosion* new_explosion = new Explosion(path, explosionPoint, Math::Scalar(explosionRadius), direction);
	explosions.emplace_back(new_explosion);
	explosions.back()->setExplosionTime(time_to_explode);
	explosions.back()->setEnabled(true);
	explosions.back()->setDamageTerrain(damage_terrain);
	explosion_nodes.append(new_explosion->explosion_node);
}
void MiniGun::create_tracer(const vec3 direction, float ratio)
{
	Resource::get()->tracer->setEnabled(1);
	NodePtr new_tracker = Resource::get()->tracer->clone();
	new_tracker->setEnabled(1);
}

vec3 MiniGun::getShellDirection(float value)
{
	float x1 = Resource::get()->shell->getWorldPosition().x;
	float y1 = Resource::get()->shell->getWorldPosition().y;
	float z1 = Resource::get()->shell->getWorldPosition().z;

	float x2 = Resource::get()->shell_end->getWorldPosition().x;
	float y2 = Resource::get()->shell_end->getWorldPosition().y;
	float z2 = Resource::get()->shell_end->getWorldPosition().z;

	return vec3((x2 - x1)*value, (y2 - y1)*value, (z2 - z1)*value);

}

Vec3 MiniGun::getBulletDirection()
{
	float x1 = Resource::get()->barrel_1->getWorldPosition().x;
	float y1 = Resource::get()->barrel_1->getWorldPosition().y;
	float z1 = Resource::get()->barrel_1->getWorldPosition().z;

	float x2 = Resource::get()->barrel_2->getWorldPosition().x;
	float y2 = Resource::get()->barrel_2->getWorldPosition().y;
	float z2 = Resource::get()->barrel_2->getWorldPosition().z;

	return Vec3((x2 - x1), (y2 - y1), (z2 - z1));
}

//previous demand
/*
	void MiniGun::create_shell()
	{
	NodePtr shell_node = Resource::get()->shell->clone();
	shell_node->release();
	Editor::get()->addNode(shell_node);
	shell_node->setEnabled(1);
	Resource::get()->M134->removeWorldChild(shell_node);
	shell_node->setWorldPosition(Resource::get()->shell_position->getWorldPosition());
	shell_node->setWorldRotation(Resource::get()->shell_position->getWorldRotation());
	
	shell_node->getObjectBodyRigid()->getShape(0)->setMass(1);
	int random = rand() % 10;
	int random2 = rand() % 10;
	
	shell_node->getObjectBodyRigid()->addImpulse(vec3((random - 3) / 100.0f), getShellDirection(30 + random2));
	
	if (currentShellNumber % 6 == 0)
	{
	Resource::get()->shoot_effect_muzzle_flash->setEnabled(1);
	//the effecto of bullet
	Bullet * bullet  = new Bullet();
	bullet->init();
	}
	else
	{
	Resource::get()->shoot_effect_muzzle_flash->setEnabled(0);
	}
	
	
	if (shells.size() != 100)
	{
	shells.push_back(shell_node);
	currentShellNumber += 1;
	}
	else
	{
	shells.get(currentShellNumber % 100)->setEnabled(0);
	shells.removeFast(currentShellNumber % 100);
	shells.append(currentShellNumber % 100, shell_node);
	currentShellNumber += 1;
	}
	
	NodePtr delink_node = Resource::get()->delink->clone();
	delink_node->release();
	Editor::get()->addNode(delink_node);
	delink_node->setEnabled(1);
	Resource::get()->M134->removeWorldChild(delink_node);
	delink_node->setWorldPosition(Resource::get()->delink_position->getWorldPosition());
	delink_node->setWorldRotation(Resource::get()->delink_position->getWorldRotation());
	
	delink_node->getObjectBodyRigid()->getShape(0)->setMass(1);
	
	
	if (delinks.size() != 100)
	{
	delinks.push_back(delink_node);
	currentDelinkNumber += 1;
	}
	else
	{
	delinks.get(currentDelinkNumber % 100)->setEnabled(0);
	delinks.removeFast(currentDelinkNumber % 100);
	delinks.append(currentDelinkNumber % 100, delink_node);
	currentDelinkNumber += 1;
	}
	
	}
*/