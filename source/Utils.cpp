#include "Utils.h"

#include <UnigineNodes.h>
#include <UnigineEditor.h>
#include <UnigineGame.h>
#include <UniginePlayers.h>
#include <UnigineApp.h>

#define PI 3.14159265
using namespace Unigine;
using namespace Math;

NodePtr LoadFromEditor(const char * nodename)
{
	//this warning is used to indicate which node is not found in the editor
	//Log::warning("Is loading node  %s  \n", nodename);
	NodePtr node = Editor::get()->getNodeByName(nodename);
	return node;
}



double randMToN(double M, double N)
{
	return M + (rand() / (RAND_MAX / (N - M)));
}

void RotateNode(Unigine::NodePtr node, Unigine::Math::quat rotationDirection)
{
	// getting current node transformation matrix
	Unigine::Math::Mat4 transform = node->getTransform();

	// calculating delta rotation around an arbitrary axis
	Unigine::Math::quat delta_rotation = Unigine::Math::quat(rotationDirection);

	// setting node's scale, rotation and position

	node->setRotation(node->getRotation()*delta_rotation);
}

vec3 getWorldDown(NodePtr node)
{
	Mat4 t = node->getWorldTransform();
	return vec3(node->isPlayer() ? -t.getAxisY() : -t.getAxisZ());
}

vec3 getWorldLeft(NodePtr node)
{
	Mat4 t = node->getWorldTransform();
	return vec3(node->isPlayer() ? -t.getAxisX() : t.getAxisX());
}

vec3 getWorldRight(NodePtr node)
{
	Mat4 t = node->getWorldTransform();
	return	vec3(node->isPlayer() ? t.getAxisX() : -t.getAxisX());
}


vec3 getWorldBack(NodePtr node)
{
	Mat4 t = node->getWorldTransform();
	return vec3(node->isPlayer() ? t.getAxisZ() : t.getAxisY());
}


vec3 getWorldUp(NodePtr node)
{
	Mat4 t = node->getWorldTransform();
	return vec3(node->isPlayer() ? t.getAxisY() : t.getAxisZ());
}


vec3 linearRegression(const Vector<vec3> &values)
{
	if (values.size() <= 0)
		return vec3::ZERO;


	float sumOfX = 0;
	vec3 sumOfY;
	float sumOfXSq = 0;
	vec3 sumOfYSq;
	vec3 sumCodeviates;
	float count = itof(values.size());

	for (int ctr = 0; ctr < count; ctr++)
	{
		float x = itof(ctr);
		vec3 y = values[ctr];
		sumCodeviates += y * x;
		sumOfX += x;
		sumOfY += y;
		sumOfXSq += x * x;
		sumOfYSq += y * y;
	}

	float ssX = sumOfXSq - ((sumOfX * sumOfX) / count);
	vec3 ssY = sumOfYSq - ((sumOfY * sumOfY) / count);
	vec3 sCo = sumCodeviates - ((sumOfY * sumOfX) / count);

	float meanX = sumOfX / count;
	vec3 meanY = sumOfY / count;

	vec3 yintercept = meanY - ((sCo / ssX) * meanX);
	vec3 slope = sCo / ssX;

	return slope * itof(values.size()) + yintercept;



}

//euler to quaternion in BVH coordinate
quat toQuaternion(double angleX, double angleY, double angleZ)
{

	angleX = angleX / 180 * PI;
	angleY = angleY / 180 * PI;
	angleZ = angleZ / 180 * PI;
	//
	quat q;
	// Abbreviations for the various angular functions
	double	c1 = cosf(angleY / 2.0f); //d
	double	c2 = cosf(angleZ / 2.0f); //e
	double	c3 = cosf(angleX / 2.0f);	//c
	double	s1 = sinf(angleY / 2.0f);	//g
	double	s2 = -sinf(angleZ / 2.0f);	//h
	double	s3 = sinf(angleX / 2.0f);	//f

		//YXZ

	q.w = c1*c2*c3 + s1*s2*s3;
	q.x = c1*c2*s3 + s1*s2*c3;
	q.y = s1*c2*c3 - c1*s2*s3;
	q.z = c1*s2*c3 - s1*c2*s3;
	//XYZ
	//q.w = c1*c2*c3 - s1*s2*s3;
	//q.x = c1*c2*s3 + s1*s2*c3;
	//q.y = s1*c2*c3 - c1*s2*s3;
	//q.z = c1*s2*c3 + s1*c2*s3;
	
	//ZXY
//	q.w = c1*c2*c3 - s1*s2*s3;
//	q.x = c1*c2*s3 - s1*s2*c3;
//	q.y = s1*c2*c3 + c1*s2*s3;
//	q.z = c1*s2*c3 + s1*c2*s3;

	///normilization
	double n = sqrt(q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w);
	q.w = q.w / n;
	q.x = q.x / n;
	q.y = q.y / n;
	q.z = q.z / n;
	
	return q;
}

float lerp_clamped(float v0, float v1, float k)
{
	return lerp(v0, v1, clamp(k, 0.0f, 1.0f));
}

enum Axis { X, Y, Z, UP };

void setDirection(const NodePtr &node, bool local, const vec3 &dir, const vec3 &up, int ret0, int v00, int v01, int ret1, int v10, int v11)
{
	mat3 rotate, scale;
	mat3 rotation = mat3(local ? node->getTransform() : node->getWorldTransform());
	orthonormalize(rotate, rotation);
	mul(scale, transpose(rotate), rotation);
	vec3 x, y, z = normalize(dir);
	x = y = z;
	cross(ret0 == X ? x : (ret0 == Y ? y : z), v00 == X ? x : (v00 == Y ? y : (v00 == Z ? z : up)), v01 == X ? x : (v01 == Y ? y : (v01 == Z ? z : up))).normalize();
	cross(ret1 == X ? x : (ret1 == Y ? y : z), v10 == X ? x : (v10 == Y ? y : (v10 == Z ? z : up)), v11 == X ? x : (v11 == Y ? y : (v11 == Z ? z : up))).normalize();
	rotate.setColumn(0, x);
	rotate.setColumn(1, y);
	rotate.setColumn(2, z);
	if (vec3(scale.m00, scale.m11, scale.m22) == vec3::ONE)
		scale = mat3::IDENTITY;
	if (local)
		node->setTransform(Mat4(mul(rotation, rotate, scale), node->getTransform().getColumn3(3)));
	else
		node->setWorldTransform(Mat4(mul(rotation, rotate, scale), node->getWorldTransform().getColumn3(3)));
}

void setWorldDirectionX(const NodePtr &node, const vec3 &dir, const vec3 &up)
{
	setDirection(node, false, dir, up, Y, UP, X, Z, X, Y);
}

void setWorldDirectionY(const NodePtr &node, const vec3 &dir, const vec3 &up)
{
	setDirection(node, false, dir, up, X, Y, UP, Z, X, Y);
}

void setTransformPosition(Mat4 &transform, const Vec3 &pos)
{
	transform.setColumn3(3, pos);
}


void setTransformRotation(Mat4 &transform, const quat &rot)
{
	mat3 rotMat = rot.getMat3();
	transform.setColumn3(0, Vec3(rotMat.getColumn(0)));
	transform.setColumn3(1, Vec3(rotMat.getColumn(1)));
	transform.setColumn3(2, Vec3(rotMat.getColumn(2)));
}


void setParentKeepPosition(const NodePtr &child, const NodePtr &parent)
{
	if (parent)
	{
		Mat4 t = parent->getIWorldTransform() * child->getWorldTransform();
		child->setTransform(t);
		child->setParent(parent);
	}
	else if (child->getParent())
	{
		Mat4 t = child->getParent()->getWorldTransform() * child->getTransform();
		child->setTransform(t);
		child->setParent(parent);
	}
}

void unParentKeepPosition(const NodePtr &child)
{
	Mat4 t = child->getWorldTransform();
	NodePtr null_node;
	child->setParent(null_node);
	child->setWorldTransform(t);
}


float getTerrainHeightAt(ObjectTerrainGlobalPtr terrain, Vec3 player_position)
{

	// auxiliary variables to store topology data
	Vec3 t_point;
	vec3 normal;
	vec3 up;

	// getting local position of the player
	vec4 local_position = vec4(terrain->getIWorldTransform() * Vec4(player_position, 1.0f));

	// fetching topology data for the point that corresponds to player's position
	terrain->fetchTopologyData(local_position.x, local_position.y, t_point, normal, up, 1);

	// returning terrain height at player's position
	return float(t_point.z);
}


vec3 getTerrainNormal(ObjectTerrainGlobalPtr terrain, Vec3 player_position)
{

	// auxiliary variables to store topology data
	Vec3 t_point;
	vec3 normal;
	vec3 up;

	// getting local position of the player
	vec4 local_position = vec4(terrain->getIWorldTransform() * Vec4(player_position, 1.0f));

	// fetching topology data for the point that corresponds to player's position
	terrain->fetchTopologyData(local_position.x, local_position.y, t_point, normal, up, 1);

	// returning terrain height at player's position
	return normal;
}

vec3 getTerrainUp(ObjectTerrainGlobalPtr terrain, Vec3 player_position)
{

	// auxiliary variables to store topology data
	Vec3 t_point;
	vec3 normal;
	vec3 up;

	// getting local position of the player
	vec4 local_position = vec4(terrain->getIWorldTransform() * Vec4(player_position, 1.0f));

	// fetching topology data for the point that corresponds to player's position
	terrain->fetchTopologyData(local_position.x, local_position.y, t_point, normal, up, 1);

	// returning terrain height at player's position
	return up;
}

vec3 getEntityDirection(Vec3 position1, Vec3 position2)
{
	float x1 = position1.x;
	float y1 = position1.y;
	float z1 = position1.z;

	float x2 = position2.x;
	float y2 = position2.y;
	float z2 = position2.z;

	return vec3((x2 - x1), (y2 - y1), (z2 - z1));

}


float getRotateAngle(Vec3 position1, Vec3 position2)
{
	//Log::message("%f y\n", fabsf((position2.y - position1.y)));
	//Log::message("%f x\n", fabsf((position2.x - position1.x)));
	return atanf(fabsf((position2.y - position1.y)) / fabsf((position2.x - position1.x))) * 180/PI;
}


float getDistance(Vec3 position1, Vec3 position2)
{
	float a = position2.x - position1.x;
	float b = position2.y - position1.y;
	float c = position2.z - position1.z;
	
	
	float distance1 = fsqrt(a*a + b*b + c*c);
	return distance1;
}

Vec3 getMiddlePosition(Vec3 position1, Vec3 position2)
{
	return (position2 + position1) / 2;
}
