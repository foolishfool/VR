#include "Bullet.h"



void Bullet::init()
{
	bullet_node = Resource::get()->bullet->clone();
	bullet_node->release();
	Editor::get()->addNode(bullet_node);
	bullet_node->setEnabled(1);
	Resource::get()->M134->removeWorldChild(bullet_node);
	bullet_node->setWorldPosition(Resource::get()->bullet->getWorldPosition());
	bullet_node->setWorldRotation(Resource::get()->bullet->getWorldRotation());
//	bullet_node->getObjectBodyRigid()->setEnabled(1);
//	bullet_node->setCollider(1);
	bullet_node->getObjectBodyRigid()->getShape(0)->setMass(0.1);
	bullet_node->getObjectBodyRigid()->addImpulse(vec3(0.0f), getBulletDirection(1000));

//	body = bullet_node->getObjectBodyRigid()->getBody();
//	body->setContactCallback(MakeCallback(this, &Bullet::on_contact));



}


void Bullet::update()
{
}


vec3 Bullet::getBulletDirection(float value)
{
	float x1 = Resource::get()->barrel_1->getWorldPosition().x;
	float y1 = Resource::get()->barrel_1->getWorldPosition().y;
	float z1 = Resource::get()->barrel_1->getWorldPosition().z;

	float x2 = Resource::get()->barrel_2->getWorldPosition().x;
	float y2 = Resource::get()->barrel_2->getWorldPosition().y;
	float z2 = Resource::get()->barrel_2->getWorldPosition().z;
	//	Log::message("%f %f %f \n", (x2 - x1)*value, (y2 - y1)*value, (z2 - z1)*value);
	return vec3((x2 - x1)*value, (y2 - y1)*value, (z2 - z1)*value);

}

void Bullet::on_contact(BodyPtr body, int num)
{
	if (num >= body->getNumContacts())
		return;
	BodyPtr body_1 = bullet_node->getObjectBodyRigid()->getBody();
	Vec3 position = body_1->getContactPoint(num);
	NodePtr bull_effect = Resource::get()->bullet_effect->clone();
	ObjectParticlesPtr bullet_effect_particle = ObjectParticles::cast(bull_effect);
	bull_effect->setWorldPosition(position);
	bull_effect->setEnabled(1);
	bullet_effect_particle->setEmitterEnabled(1);
		
	Log::message("%d  %f,%f,%f bullet_node\n", num, position.x, position.y, position.z);
		//body_1->renderContacts();
		
	//	Log::message("%s\n", body_1->getContactObject(num)->getNode()->getName());


	//	ObjectParticlesPtr bullet_effect_particle = ObjectParticles::cast(bullet_effect);	
	
	//	Log::message("%f,%f,%f bullet_node\n", bullet_node->getWorldPosition().x, bullet_node->getWorldPosition().y, bullet_node->getWorldPosition().z);
	//	Log::message("%f,%f,%f bullet_node\n", bullet_effect->getWorldPosition().x, bullet_effect->getWorldPosition().y, bullet_effect->getWorldPosition().z);
	//	Log::message("%d \n", num);
		
		//bullet_effect_particle->setEmitterEnabled(1);
	//	bullet_node->getObjectBodyRigid()->getShape(0)->setMass(0);
	//	bullet_node->getObjectBodyRigid()->addImpulse(vec3(0.0f), getBulletDirection(0));
	////	bullet_node->setScale(vec3(5, 5, 5));
	//	bullet_node->getObjectBodyRigid()->setImmovable(1);
	//	bullet_node->getChild(0)->setEnabled(1);
	//	ObjectParticlesPtr bullet_effect_particle = ObjectParticles::cast(bullet_node->getChild(0));
	//	bullet_effect_particle->setEmitterEnabled(1);
	//	bullet_node->setCollider(0);
	
	

}
