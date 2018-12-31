#pragma once

#include <UnigineEditor.h>
#include <UnigineGame.h>
#include <UnigineApp.h>
#include <UnigineSounds.h>
#include <UnigineNodes.h>
#include "WorldSyncker.h"
#include "DamageArea.h"

class Explosion
{
public:
	Explosion();
	Explosion(const char *node_path, const Unigine::Math::Vec3 &pos, Unigine::Math::Scalar radius, const Unigine::Math::Vec3 &direction);
	~Explosion();

	void setEnabled(bool enabled);
	void setDamageTerrain(bool damage);
	void setExplosionTime(float explosion_time);

	bool isEnabled() const;

	void update(float ifps);


	NodePtr explosion_node;

private:
	bool is_enabled;
	bool is_exploded;
	bool is_disappear;
	bool is_damaged;
	float explosion_time;
	float damage_time;
	float life_time;
	float current_time;

//	DamageArea damage_area;

	Vector<SoundSourcePtr> explosion_sounds;
	Vector<ObjectParticlesPtr> explosion_particles;
};

