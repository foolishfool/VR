#include "Explosion.h"

using namespace Unigine;

Explosion::Explosion()
{
}

Explosion::Explosion(const char *node_path, const Math::Vec3 &pos, Math::Scalar radius, const Math::Vec3 &direction)
	: is_enabled(false)
	, is_exploded(false)
	, is_damaged(false)
	, explosion_time(0.0f)
	, damage_time(0.15f)
	, life_time(10.0f)
	, current_time(0.0f)
	//, damage_area(UNIGINE_BOUND_SPHERE(pos, radius), direction)
{
	explosion_node = World::get()->loadNode(node_path);
	WorldSyncker::get()->syncker_master->createNode(explosion_node, 1,1);
	for (int i = 0; i < explosion_node->getNumChildren(); i++)
	{
		WorldSyncker::get()->syncker_master->createNode(explosion_node->getChild(i), 1, 1);
	}
	explosion_node->setWorldPosition(pos);
	
	for (int i = 0; i < explosion_node->getNumChildren(); i++)
	{
		NodePtr child = explosion_node->getChild(i);
		if (child->getType() == Node::OBJECT_PARTICLES)
		{
			ObjectParticlesPtr particles = ObjectParticles::cast(child);
			particles->setEnabled(0);
			explosion_particles.append(particles);
		}
	}
}

Explosion::~Explosion()
{
}

void Explosion::setEnabled(bool enabled)
{
	is_enabled = enabled;
	if (!is_enabled)
		return;

	is_exploded = false;
	is_damaged = false;
	current_time = 0.0f;
//	damage_area.setEnabled(true);
}

void Explosion::setDamageTerrain(bool damage)
{
//	damage_area.setDamageTerrain(damage);
}

void Explosion::setExplosionTime(float time)
{
	float delta = time - explosion_time;
	explosion_time = time;
	damage_time += delta;
	life_time += delta;
}

bool Explosion::isEnabled() const
{
	return is_enabled;
}

void Explosion::update(float ifps)
{
	if (!is_enabled)
		return;

	current_time += ifps;

	if (!is_exploded && current_time >= explosion_time)
	{
		is_exploded = true;
		for (auto explosion_particle : explosion_particles)
		{
			explosion_particle->setEnabled(1);		
			is_disappear = true;
		}
		return;
	}

	if (!is_damaged && current_time >= damage_time)
	{
		is_damaged = true;
		//damage_area.update();
		return;
	}

	if (current_time >= life_time)
	{
		is_enabled = false;
		explosion_node->setEnabled(0);
		return;
	}
}
