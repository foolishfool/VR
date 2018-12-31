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

class Bullet
{
public:
	void init();
	void update();

	NodePtr bullet_node;

	vec3 getBulletDirection(float value);
	BodyPtr body;
private:
	void on_contact(BodyPtr body, int num);
};

