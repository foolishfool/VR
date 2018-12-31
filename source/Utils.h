#pragma once

#include <UnigineNode.h>
#include <UnigineObjects.h>
#include <UnigineNodes.h>
#include <UnigineMap.h>
#include <UnigineVector.h>
#include <UnigineCamera.h>
#include <functional>
#include <string>
#include <vector>
#include <map>



using namespace Unigine;
using namespace Math;

NodePtr LoadFromEditor(const char * nodename);
double randMToN(double M, double N);
void RotateNode(Unigine::NodePtr node, Unigine::Math::quat rotationDirection);
float getRotateAngle(Vec3 position1, Vec3 position2);


float getTerrainHeightAt(ObjectTerrainGlobalPtr terrain, Vec3 player_position);
vec3 getTerrainNormal(ObjectTerrainGlobalPtr terrain, Vec3 player_position);
vec3 getTerrainUp(ObjectTerrainGlobalPtr terrain, Vec3 player_position);



vec3 getEntityDirection(Vec3 position1, Vec3 position2);

vec3 getWorldDown(NodePtr node);
vec3 getWorldLeft(NodePtr node);
vec3 getWorldRight(NodePtr node);
vec3 getWorldBack(NodePtr node);
vec3 getWorldUp(NodePtr node);
vec3 linearRegression(const Vector<vec3> &values);

quat toQuaternion(double angleX, double angleY, double angleZ);
float getDistance(Vec3 position1, Vec3 position2);
Vec3 getMiddlePosition(Vec3 position1, Vec3 position2);


template<class Sig>

struct Action
{
	// add new listener
	template<typename Functor>
	void operator+=(Functor&& f)
	{
		addListener(f);
	}

	// add new listener with returning id
	template<typename Functor>
	int addListener(Functor&& f)
	{
		if (in_process)
		{
			add_id_queue.push_back(id);
			add_function_queue.push_back(std::forward<Functor>(f));
			return id++;
		}

		_funcs[id] = std::forward<Functor>(f);
		return id++;
	}

	// reserve id for some listener
	// (use addListener(int, Functor) later)
	int reserveId()
	{
		return id++;
	}

	// add listener to specific place 
	// use it when you want to return
	// removed listener (or add reserved)
	template<typename Functor>
	void addListener(int id, Functor&& f)
	{
		if (in_process)
		{
			add_id_queue.push_back(id);
			add_function_queue.push_back(std::forward<Functor>(f));
			return;
		}

		_funcs[id] = std::forward<Functor>(f);
	}

	// remove listener by id
	void removeListener(int id)
	{
		if (in_process)
		{
			remove_queue.push_back(id);
			return;
		}

		_funcs.erase(id);
	}

	// count of listeners
	size_t size() const
	{
		return _funcs.size();
	}

	// remove all listeners from Action
	void clear()
	{
		if (in_process)
		{
			need_clear = true;
			return;
		}

		_funcs.clear();

		// id doesn't set to 0 because we
		// want to safe unique ID for each new functor
	}

	// execute (call to listeners)
	template<class... Args>
	void operator()(Args&&... args)
	{
		// execute
		in_process = true;
		for (auto it = _funcs.begin(); it != _funcs.end(); ++it)
			it->second(args...);
		in_process = false;

		// check add/remove queue
		if (add_id_queue.size() > 0)
		{
			for (int i = 0; i < add_id_queue.size(); i++)
				addListener(add_id_queue[i], add_function_queue[i]);

			add_id_queue.clear();
			add_function_queue.clear();
		}

		if (remove_queue.size() > 0 || need_clear)
		{
			for (int i = 0; i < remove_queue.size(); i++)
				removeListener(remove_queue[i]);
			remove_queue.clear();
			need_clear = false;
		}
	}

private:
	std::map<int, std::function<Sig>> _funcs;
	int id = 0;

	// deferred execution of methods
	// (because we want sometimes add/remove
	// listeners inside called method)
	bool in_process = false;

	std::vector<int> add_id_queue;
	std::vector<std::function<Sig>> add_function_queue;

	std::vector<int> remove_queue;
	bool need_clear = false;
};


/*
Math: Common
*/
float lerp_clamped(float v0, float v1, float k);

/*
Node: Directions
*/

void setWorldDirectionX(const NodePtr &node, const vec3 &dir, const vec3 &up);
void setWorldDirectionY(const NodePtr &node, const vec3 &dir, const vec3 &up);


/*
Transform: Partial changes
*/
void setTransformRotation(Mat4 &transform, const quat &rot);
void setTransformPosition(Mat4 &transform, const Vec3 &pos);
/*
Node: Child/Parent Relationships
*/
void setParentKeepPosition(const NodePtr &child, const NodePtr &parent);
void unParentKeepPosition(const NodePtr &child);


