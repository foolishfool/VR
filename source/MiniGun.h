#pragma once
#include <UnigineEngine.h>
#include <UnigineNode.h>
#include <UnigineEditor.h>
#include <UnigineGame.h>
#include <UniginePrimitives.h>
#include <UnigineObjects.h>
#include <UnigineApp.h>
#include <UnigineSounds.h>
#include <UniginePhysicals.h>
#include <UnigineWorld.h>
#include <UnigineBounds.h>
#include "Utils.h"
#include "Resource.h"
#include "Explosion.h"
#include "Bullet.h"
#include "WorldSyncker.h"

class MiniGun
{
public:
	void init();
	void update();

	static MiniGun* instance;
	static MiniGun* get();
	
	void fire();
	int bulletNumber = 0;
	int bullet_effect_number = 0;
	float elapsed_time_num = 0.0f;
private:
	Vec3 currentposition;
	Vec3 lastposition;
	vec3 wind_direction;
	float helicopter_speed;
	Vec3 gun_pos, gun_dir;
	int round = 0;	
	//void create_shell();
	void create_tracer(const vec3 direction, float ratio);
	vec3 getShellDirection(float value);
	Vector <NodePtr> shells;
	Vector < NodePtr> delinks;
	Vector <NodePtr> bullets;
	Vector <ObjectPtr> bullets_hitObjs;
	Vector<NodeReferencePtr> oil_tanks;
//	Map<int, SoundSourcePtr> explosion_sounds;
	
	bool isFiring = false;
	bool isStoped = false;
	bool isStatic = true;

	int currentShellNumber;
	int currentDelinkNumber;
	int currentBulletNumber;

	float bulletSpeed = 853.0f;
	double explosionRadius = 8.0;
	float time_to_explode;
	Vec3 explosionPoint;
	VectorStack<ObjectPtr, 64> hitObjs;
	std::vector<Explosion*> explosions;

	Vector<NodePtr> effects;
	Vector<NodePtr> explosion_nodes;
	Vec3 getBulletDirection();
	void create_explosion(const char *path, Vec3 explosionPoint, double explosionRadius, Vec3 direction, bool damage_terrain);
};

