#include "LeapMotion.h"

#define HAND_COUNT 2

using namespace Unigine;
using namespace Math;

float plam_offset_x = -0.006656;
float plam_offset_y = -0.045173;
float plam_offset_z = -0.103783;



const float CHANGE_INTERVAL_VELOCITY_LEAPMOTION = 0.01f; // the interval between changes of position, in seconds
float elapsed_time_velocity_leapmotion;	// current time left to change current scale of our objects

LeapMotion* LeapMotion::instance;

LeapMotion* LeapMotion::get()
{
	return instance;
}

void LeapMotion::init()
{
	instance = this;

	lm = static_cast<LeapMotionInterface*>(Engine::get()->getPluginData(Engine::get()->findPlugin("LeapMotion")));
	lm->setPolicy(LeapMotionInterface::POLICY_IMAGES);
	lm->setPolicy(LeapMotionInterface::POLICY_OPTIMIZE_HMD);
	
	leap_motion_camera = PlayerDummy::cast(Editor::get()->getNodeByName("leap_motion_camera"));
	leap_motion_camera->setZNear(0.1f);
	//Game::get()->setPlayer(main_camera->getPlayer());

	leap_motion_camera->setWorldParent(eye);
	leap_motion_camera->setPosition(Vec3(0, 0, 0));
	leap_motion_camera->setRotation(quat(0, 0, 0));

	visualizer = Visualizer::get();
	visualizer->setEnabled(1);

	create_joints();

	lefthand_lastposition = palm_bone_left->getWorldPosition();
	righthand_lastposition = palm_bone_right->getWorldPosition();

	grab_init();

}

void LeapMotion::update()
{

	//adjustPlamPosition();


	leap_motion_camera->setPosition(Vec3(plam_offset_x, plam_offset_y, plam_offset_z));
	
	
	if (lm->isConnected() )
	{
		for (auto &hand : lm->getHands())
		{
			draw_leap_motion_hand(hand.data);
		}
		

		//hide the hands
		if (lm->getHands().size() == 0)
		{
			hide_joints_left(0);
			hide_joints_right(0);
			//show golves
			Resource::get()->left_hand_glove->setEnabled(1);
			Resource::get()->right_hand_glove->setEnabled(1);
		}
		else
		if (lm->getHands().size() == 1)
		{
			if (lm->getHands().begin().get()->data.type == 0)
			{
				hide_joints_left(1);
				hide_joints_right(0);
				Resource::get()->left_hand_glove->setEnabled(0);
				Resource::get()->right_hand_glove->setEnabled(1);
			}
			else if (lm->getHands().begin().get()->data.type == 1)
			{
				hide_joints_left(0);
				hide_joints_right(1);
				Resource::get()->left_hand_glove->setEnabled(1);
				Resource::get()->right_hand_glove->setEnabled(0);
			}

		}
		else
		if (lm->getHands().size() == 2)
		{
			hide_joints_left(1);
			hide_joints_right(1);
			Resource::get()->left_hand_glove->setEnabled(0);
			Resource::get()->right_hand_glove->setEnabled(0);
		}


		float distance_left = getDistance(finger_joint_thumb_left[4]->getWorldPosition(), finger_joint_middle_left[4]->getWorldPosition());
		float distance_right = getDistance(finger_joint_thumb_right[4]->getWorldPosition(), finger_joint_middle_right[4]->getWorldPosition());
		float distance_left1 = getDistance(finger_joint_ring_left[4]->getWorldPosition(), finger_joint_middle_left[4]->getWorldPosition());
		float distance_right1 = getDistance(finger_joint_ring_right[4]->getWorldPosition(), finger_joint_middle_right[4]->getWorldPosition());
		float distance_left2 = getDistance(finger_joint_index_left[4]->getWorldPosition(), finger_joint_middle_left[4]->getWorldPosition());
		float distance_right2= getDistance(finger_joint_index_right[4]->getWorldPosition(), finger_joint_middle_right[4]->getWorldPosition());
		float distance_left3 = getDistance(finger_joint_pinky_left[4]->getWorldPosition(), finger_joint_ring_left[4]->getWorldPosition());
		float distance_right3 = getDistance(finger_joint_pinky_right[4]->getWorldPosition(), finger_joint_ring_right[4]->getWorldPosition());

	//	Log::message("%f \n", distance_left);

		if (distance_left1<0.018 || distance_left2<0.018 || distance_left3<0.018 || distance_left<0.04)
		{
			isHoldingLeft = true;
		}
		else
		{
			isHoldingLeft = false;
		}

		if (distance_right1<0.018 || distance_right2<0.018 || distance_right3<0.018 || distance_right<0.04)
		{
			isHoldingRight = true;
		}
		else
		{
			isHoldingRight = false;
		}

		grab_update(0, isHoldingLeft);
		grab_update(1, isHoldingRight);
	
	}
}
void LeapMotion::adjustPlamPosition()
{
	float ratio = 10;

	if (App::get()->getKeyState(119) == 1)
	{
		hand_offset_x += Game::get()->getIFps()*ratio;
	}
	if (App::get()->getKeyState(97) == 1)
	{
		hand_offset_y += Game::get()->getIFps()*ratio;
	}
	if (App::get()->getKeyState(113) == 1)
	{
		hand_offset_z += Game::get()->getIFps()*ratio;
	}
	if (App::get()->getKeyState(115) == 1)
	{
		hand_offset_x -= Game::get()->getIFps()*ratio;
	}
	if (App::get()->getKeyState(100) == 1)
	{
		hand_offset_y -= Game::get()->getIFps()*ratio;
	}
	if (App::get()->getKeyState(101) == 1)
	{
		hand_offset_z -= Game::get()->getIFps()*ratio;
	}

	if (App::get()->clearKeyState(55) == 1)
	{
		x_left_mesh_rotation += 90;
	}
	if (App::get()->clearKeyState(56) == 1)
	{
		y_left_mesh_rotation += 90;
	}

	if (App::get()->clearKeyState(57) == 1)
	{
		z_left_mesh_rotation += 90;
	}

	Log::message(" %f %f %f \n", hand_offset_x, hand_offset_y, hand_offset_z);
}
void LeapMotion::create_widget_leap_img()
{
	widget_leap_img = WidgetSprite::create(gui);
	gui->addChild(widget_leap_img->getWidget(), Gui::ALIGN_OVERLAP);
	widget_leap_img->setPosition(0, 0);
	widget_leap_img->setOrder(-1);
}

void LeapMotion::create_widget_hands()
{
	widget_hands = WidgetSpriteViewport::create(gui, gui->getWidth(), gui->getHeight());
	gui->addChild(widget_hands->getWidget(), Gui::ALIGN_OVERLAP);
	widget_hands->setPosition(0, 0);

	widget_hands->setCamera(leap_motion_camera->getCamera());
	widget_hands->setSkipFlags(0);
	widget_hands->setAspectCorrection(0);
	widget_hands->setBlendFunc(Gui::BLEND_SRC_ALPHA, Gui::BLEND_SRC_ALPHA);
}

void LeapMotion::draw_leap_motion_hand(const Unigine::LeapMotionInterface::Hand &hand)
{
	bool type = hand.type != 0;
	vec4 hand_color(type, 0.5f, !type, 1);

	auto palm_pos = leap_pos_to_world_pos(hand.palm_position);
	Mat4 transform;
	transform.setTranslate(palm_pos);
//plam position
	if (type == 0)
	{
		palm_bone_left->setWorldParent(leap_motion_camera->getNode());
		palm_bone_left->setPosition(Vec3(palm_pos.x, -palm_pos.y, -palm_pos.z));
		palm_bone_left->setRotation(quat(90,0,0));
		palm_bone_left->setEnabled(0);
	}
	else
	{
		palm_bone_right->setWorldParent(leap_motion_camera->getNode());
		palm_bone_right->setPosition(Vec3(palm_pos.x, -palm_pos.y, -palm_pos.z));
		palm_bone_right->setRotation(quat(90, 0, 0));
		palm_bone_right->setEnabled(0);
	}	
	//visualizer->renderSphere(0.005f, transform, hand_color);

	auto wrist_pos = leap_pos_to_world_pos(hand.wrist_position);
	transform.setTranslate(wrist_pos);
	if (type == 0)
	{
		wrist_joint_left->setWorldParent(leap_motion_camera->getNode());
		wrist_joint_left->setPosition(Vec3(wrist_pos.x, -wrist_pos.y, -wrist_pos.z));
		wrist_joint_left->setEnabled(1);
	}
	else
	{
		wrist_joint_right->setWorldParent(leap_motion_camera->getNode());
		wrist_joint_right->setPosition(Vec3(wrist_pos.x, -wrist_pos.y, -wrist_pos.z));
		wrist_joint_right->setEnabled(1);
	}

	//visualizer->renderSphere(0.003f, transform, hand_color);

	auto arm_pos = leap_pos_to_world_pos(hand.arm.center);
	transform.setTranslate(arm_pos);
	
	if (type == 0)
	{
		arm_joint_left->setWorldParent(leap_motion_camera->getNode());
		arm_joint_left->setPosition(Vec3(arm_pos.x, -arm_pos.y, -arm_pos.z));
		arm_joint_left->isEnabled(0);
	}
	else
	{
		arm_joint_right->setWorldParent(leap_motion_camera->getNode());
		arm_joint_right->setPosition(Vec3(arm_pos.x, -arm_pos.y, -arm_pos.z));
		arm_joint_right->isEnabled(0);
	}
	
	//visualizer->renderSphere(0.007f, transform, hand_color);

	//visualizer->renderLine3D(palm_bone_left->getWorldPosition(), wrist_bone_left->getWorldPosition(), hand_color);
	//visualizer->renderLine3D(arm_bone_left->getWorldPosition(), wrist_bone_left->getWorldPosition(), hand_color);

	//visualizer->renderLine3D(palm_bone_right->getWorldPosition(), wrist_bone_right->getWorldPosition(), hand_color);
	//visualizer->renderLine3D(arm_bone_right->getWorldPosition(), wrist_bone_right->getWorldPosition(), hand_color);

	const auto &fingers = hand.fingers;
	for (auto &finger : fingers)
	{
		auto bones = finger.data.bones;
		//	visualizer->renderLine3D(wrist_bone_right->getWorldPosition(), leap_pos_to_world_pos(bones[0].prev_joint), hand_color);
		//	float start_radius = 0.003f;
		//	const float radius_step = -0.0005f;
			for (auto i = 0; i < LeapMotionInterface::Bone::BONE_NUM_TYPES; ++i)
		{
			transform.setTranslate(leap_pos_to_world_pos(bones[i].prev_joint));
			draw_finger_joints(type, finger.data.type,i, leap_pos_to_world_pos(bones[i].prev_joint),hand_color);
			//visualizer->renderSphere(start_radius, transform, hand_color);
		//	visualizer->renderLine3D(leap_pos_to_world_pos(bones[i].prev_joint), leap_pos_to_world_pos(bones[i].next_joint), hand_color);
		}
		Vec3 finger_pos = leap_pos_to_world_pos(bones[LeapMotionInterface::Bone::BONE_NUM_TYPES - 1].next_joint);
		transform.setTranslate(finger_pos);
		draw_finger_joints(type, finger.data.type, LeapMotionInterface::Bone::BONE_NUM_TYPES,finger_pos, hand_color);
		//visualizer->renderSphere(start_radius, transform, hand_color);
		
	}
	
	draw_bones();
}

Vec3 LeapMotion::leap_pos_to_world_pos(const Unigine::Math::vec3 &pos)
{
	Vec3 wpos(pos);
	std::swap(wpos.y, wpos.z);
	wpos.y = -wpos.y;
	wpos.x -= lm->getBaseline() / 2;

	auto iwt = leap_motion_camera->getIWorldTransform();	
	iwt.setRotateZ(180);

	Vec3 position_adjust = iwt*Vec3(wpos);

	return  position_adjust;
}

void LeapMotion::create_joints()
{
	palm_bone_left = Primitives::createSphere(0.08, 16, 16);
	palm_bone_left->release();
	palm_bone_left->setCollider(0);

	palm_bone_right = Primitives::createSphere(0.08, 16, 16);
	palm_bone_right->release();
	palm_bone_right->setCollider(0);

	wrist_joint_left = Primitives::createSphere(0.01, 16, 16);
	wrist_joint_left->release();
	wrist_joint_left->setMaterial("skin", 0);
	wrist_joint_left->setCollider(0);
	wrist_joint_left->setCastShadow(0, 0);
	wrist_joint_left->setCastWorldShadow(0,0);

	wrist_joint_right = Primitives::createSphere(0.01, 16, 16);
	wrist_joint_right->release();
	wrist_joint_right->setMaterial("skin", 0);
	wrist_joint_right->setCollider(0);
	wrist_joint_right->setCastShadow(0, 0);
	wrist_joint_right->setCastWorldShadow(0, 0);

	arm_joint_left = Primitives::createSphere(0.01, 16, 16);
	arm_joint_left->release();
	arm_joint_left->setMaterial("skin", 0);
	arm_joint_left->setCollider(0);
	arm_joint_left->setCastShadow(0, 0);
	arm_joint_left->setCastWorldShadow(0, 0);

	arm_joint_right = Primitives::createSphere(0.01, 16, 16);
	arm_joint_right->release();
	arm_joint_right->setMaterial("skin", 0);
	arm_joint_right->setCollider(0);
	arm_joint_right->setCastShadow(0, 0);
	arm_joint_right->setCastWorldShadow(0, 0);

	float initial_radius = 0.01;

	for (int i = 0; i < LeapMotionInterface::Bone::BONE_NUM_TYPES+1; i++, initial_radius-= 0)
	{
		finger_joint_thumb_left[i] = Primitives::createSphere(initial_radius, 16, 16);
		finger_joint_thumb_left[i]->release();
		finger_joint_thumb_left[i]->setMaterial("skin", 0);
		finger_joint_thumb_left[i]->setCollider(0);
		finger_joint_thumb_left[i]->setCastShadow(0, 0);
		finger_joint_thumb_left[i]->setCastWorldShadow(0, 0);
		finger_joint_thumb_right[i] = Primitives::createSphere(initial_radius, 16, 16);
		finger_joint_thumb_right[i]->release();
		finger_joint_thumb_right[i]->setMaterial("skin", 0);
		finger_joint_thumb_right[i]->setCollider(0);
		finger_joint_thumb_right[i]->setCastShadow(0, 0);
		finger_joint_thumb_right[i]->setCastWorldShadow(0, 0);
		finger_joint_index_left[i] = Primitives::createSphere(initial_radius, 16, 16);
		finger_joint_index_left[i]->release();
		finger_joint_index_left[i]->setMaterial("skin", 0);
		finger_joint_index_left[i]->setCollider(0);
		finger_joint_index_left[i]->setCastShadow(0, 0);
		finger_joint_index_left[i]->setCastWorldShadow(0, 0);
		finger_joint_index_right[i] = Primitives::createSphere(initial_radius, 16, 16);
		finger_joint_index_right[i]->release();
		finger_joint_index_right[i]->setMaterial("skin", 0);
		finger_joint_index_right[i]->setCollider(0);
		finger_joint_index_right[i]->setCastShadow(0, 0);
		finger_joint_index_right[i]->setCastWorldShadow(0, 0);
		finger_joint_middle_left[i] = Primitives::createSphere(initial_radius, 16, 16);
		finger_joint_middle_left[i]->release();
		finger_joint_middle_left[i]->setMaterial("skin", 0);
		finger_joint_middle_left[i]->setCollider(0);
		finger_joint_middle_left[i]->setCastShadow(0, 0);
		finger_joint_middle_left[i]->setCastWorldShadow(0, 0);
		finger_joint_middle_right[i] = Primitives::createSphere(initial_radius, 16, 16);
		finger_joint_middle_right[i]->release();
		finger_joint_middle_right[i]->setMaterial("skin", 0);
		finger_joint_middle_right[i]->setCollider(0);
		finger_joint_middle_right[i]->setCastShadow(0, 0);
		finger_joint_middle_right[i]->setCastWorldShadow(0, 0);
		finger_joint_ring_left[i] = Primitives::createSphere(initial_radius, 16, 16);
		finger_joint_ring_left[i]->release();
		finger_joint_ring_left[i]->setMaterial("skin", 0);
		finger_joint_ring_left[i]->setCollider(0);
		finger_joint_ring_left[i]->setCastShadow(0, 0);
		finger_joint_ring_left[i]->setCastWorldShadow(0, 0);
		finger_joint_ring_right[i] = Primitives::createSphere(initial_radius, 16, 16);
		finger_joint_ring_right[i]->release();
		finger_joint_ring_right[i]->setMaterial("skin", 0);
		finger_joint_ring_right[i]->setCollider(0);
		finger_joint_ring_right[i]->setCastShadow(0, 0);
		finger_joint_ring_right[i]->setCastWorldShadow(0, 0);
		finger_joint_pinky_left[i] = Primitives::createSphere(initial_radius, 16, 16);
		finger_joint_pinky_left[i]->release();
		finger_joint_pinky_left[i]->setMaterial("skin", 0);
		finger_joint_pinky_left[i]->setCollider(0);
		finger_joint_pinky_left[i]->setCastShadow(0, 0);
		finger_joint_pinky_left[i]->setCastWorldShadow(0, 0);
		finger_joint_pinky_right[i] = Primitives::createSphere(initial_radius, 16, 16);
		finger_joint_pinky_right[i]->release();
		finger_joint_pinky_right[i]->setMaterial("skin", 0);
		finger_joint_pinky_right[i]->setCollider(0);
		finger_joint_pinky_right[i]->setCastShadow(0, 0);
		finger_joint_pinky_right[i]->setCastWorldShadow(0, 0);
	}

	///must initialize first otherwise cannot find in interactiveVR vr->render
	left_bone_index_pinky = Primitives::createCapsule(0.01, 0.05);
	right_bone_index_pinky = Primitives::createCapsule(0.01, 0.05);
}

void LeapMotion::draw_finger_joints(int hand_type, int finger_type, int finger_num, Vec3 pos, vec4 hand_color)
{
	if (hand_type == 0)
	{
		if (finger_type == LeapMotionInterface::Finger::TYPE_THUMB)
		{
			finger_joint_thumb_left[finger_num]->setWorldParent(leap_motion_camera->getNode());
			finger_joint_thumb_left[finger_num]->setPosition(Vec3(pos.x, -pos.y, -pos.z));
			//visualizer->renderLine3D(wrist_bone_left->getWorldPosition(), finger_joint_thumb_left[0]->getWorldPosition(), hand_color);
		//	if (finger_num != LeapMotionInterface::Bone::BONE_NUM_TYPES)
		//	{
		//		visualizer->renderLine3D(finger_joint_thumb_left[finger_num]->getWorldPosition(), finger_joint_thumb_left[finger_num + 1]->getWorldPosition(), hand_color);
		//	}
		}
		if (finger_type == LeapMotionInterface::Finger::TYPE_INDEX)
		{
			finger_joint_index_left[finger_num]->setWorldParent(leap_motion_camera->getNode());
			finger_joint_index_left[finger_num]->setPosition(Vec3(pos.x, -pos.y, -pos.z));
		//	if (finger_num != LeapMotionInterface::Bone::BONE_NUM_TYPES)
			//visualizer->renderLine3D(finger_joint_index_left[finger_num]->getWorldPosition(), finger_joint_index_left[finger_num + 1]->getWorldPosition(), hand_color);
		}
		if (finger_type == LeapMotionInterface::Finger::TYPE_MIDDLE)
		{
			finger_joint_middle_left[finger_num]->setWorldParent(leap_motion_camera->getNode());
			finger_joint_middle_left[finger_num]->setPosition(Vec3(pos.x, -pos.y, -pos.z));
		//	if (finger_num != LeapMotionInterface::Bone::BONE_NUM_TYPES)
			//visualizer->renderLine3D(finger_joint_middle_left[finger_num]->getWorldPosition(), finger_joint_middle_left[finger_num + 1]->getWorldPosition(), hand_color);
		}
		if (finger_type == LeapMotionInterface::Finger::TYPE_RING)
		{
			finger_joint_ring_left[finger_num]->setWorldParent(leap_motion_camera->getNode());
			finger_joint_ring_left[finger_num]->setPosition(Vec3(pos.x, -pos.y, -pos.z));
		//	if (finger_num != LeapMotionInterface::Bone::BONE_NUM_TYPES)
			//visualizer->renderLine3D(finger_joint_ring_left[finger_num]->getWorldPosition(), finger_joint_ring_left[finger_num + 1]->getWorldPosition(), hand_color);
		}
		if (finger_type == LeapMotionInterface::Finger::TYPE_PINKY)
		{
			finger_joint_pinky_left[finger_num]->setWorldParent(leap_motion_camera->getNode());
			finger_joint_pinky_left[finger_num]->setPosition(Vec3(pos.x, -pos.y, -pos.z));
			//if (finger_num != LeapMotionInterface::Bone::BONE_NUM_TYPES)
			//visualizer->renderLine3D(finger_joint_pinky_left[finger_num]->getWorldPosition(), finger_joint_pinky_left[finger_num + 1]->getWorldPosition(), hand_color);
		}

	}
	else
	{
		if (finger_type == LeapMotionInterface::Finger::TYPE_THUMB)
		{
			finger_joint_thumb_right[finger_num]->setWorldParent(leap_motion_camera->getNode());
			finger_joint_thumb_right[finger_num]->setPosition(Vec3(pos.x, -pos.y, -pos.z));
			//visualizer->renderLine3D(wrist_bone_right->getWorldPosition(), finger_joint_thumb_right[0]->getWorldPosition(), hand_color);
			//if (finger_num != LeapMotionInterface::Bone::BONE_NUM_TYPES)
			//visualizer->renderLine3D(finger_joint_thumb_right[finger_num]->getWorldPosition(), finger_joint_thumb_right[finger_num + 1]->getWorldPosition(), hand_color);
		}
		if (finger_type == LeapMotionInterface::Finger::TYPE_INDEX)
		{
			finger_joint_index_right[finger_num]->setWorldParent(leap_motion_camera->getNode());
			finger_joint_index_right[finger_num]->setPosition(Vec3(pos.x, -pos.y, -pos.z));
			//if (finger_num != LeapMotionInterface::Bone::BONE_NUM_TYPES)
			//visualizer->renderLine3D(finger_joint_index_right[finger_num]->getWorldPosition(), finger_joint_index_right[finger_num + 1]->getWorldPosition(), hand_color);
		}
		if (finger_type == LeapMotionInterface::Finger::TYPE_MIDDLE)
		{
			finger_joint_middle_right[finger_num]->setWorldParent(leap_motion_camera->getNode());
			finger_joint_middle_right[finger_num]->setPosition(Vec3(pos.x, -pos.y, -pos.z));
			//if (finger_num != LeapMotionInterface::Bone::BONE_NUM_TYPES)
			//visualizer->renderLine3D(finger_joint_middle_right[finger_num]->getWorldPosition(), finger_joint_middle_right[finger_num + 1]->getWorldPosition(), hand_color);
		}
		if (finger_type == LeapMotionInterface::Finger::TYPE_RING)
		{
			finger_joint_ring_right[finger_num]->setWorldParent(leap_motion_camera->getNode());
			finger_joint_ring_right[finger_num]->setPosition(Vec3(pos.x, -pos.y, -pos.z));
			//if (finger_num != LeapMotionInterface::Bone::BONE_NUM_TYPES)
			//visualizer->renderLine3D(finger_joint_ring_right[finger_num]->getWorldPosition(), finger_joint_ring_right[finger_num + 1]->getWorldPosition(), hand_color);
		}
		if (finger_type == LeapMotionInterface::Finger::TYPE_PINKY)
		{
			finger_joint_pinky_right[finger_num]->setWorldParent(leap_motion_camera->getNode());
			finger_joint_pinky_right[finger_num]->setPosition(Vec3(pos.x, -pos.y, -pos.z));
			//if (finger_num != LeapMotionInterface::Bone::BONE_NUM_TYPES)
			//visualizer->renderLine3D(finger_joint_ring_right[finger_num]->getWorldPosition(), finger_joint_ring_right[finger_num + 1]->getWorldPosition(), hand_color);
		}

	}
}

void LeapMotion::create_bones()
{
	float radius = 0.0095;
	//left_thumb bones


	for (int i = 0; i < 4; i++)
	{
	
		length_thumb_left[i] = getDistance(finger_joint_thumb_left[i]->getWorldPosition(), finger_joint_thumb_left[i+1]->getWorldPosition());
		length_index_left[i] = getDistance(finger_joint_index_left[i]->getWorldPosition(), finger_joint_index_left[i+1]->getWorldPosition());
		length_middle_left[i] = getDistance(finger_joint_middle_left[i]->getWorldPosition(), finger_joint_middle_left[i + 1]->getWorldPosition());
		length_ring_left[i] = getDistance(finger_joint_ring_left[i]->getWorldPosition(), finger_joint_ring_left[i + 1]->getWorldPosition());
		length_pinky_left[i] = getDistance(finger_joint_pinky_left[i]->getWorldPosition(), finger_joint_pinky_left[i + 1]->getWorldPosition());

		length_thumb_right[i] = getDistance(finger_joint_thumb_right[i]->getWorldPosition(), finger_joint_thumb_right[i + 1]->getWorldPosition());
		length_index_right[i] = getDistance(finger_joint_index_right[i]->getWorldPosition(), finger_joint_index_right[i + 1]->getWorldPosition());
		length_middle_right[i] = getDistance(finger_joint_middle_right[i]->getWorldPosition(), finger_joint_middle_right[i + 1]->getWorldPosition());
		length_ring_right[i] = getDistance(finger_joint_ring_right[i]->getWorldPosition(), finger_joint_ring_right[i + 1]->getWorldPosition());
		length_pinky_right[i] = getDistance(finger_joint_pinky_right[i]->getWorldPosition(), finger_joint_pinky_right[i + 1]->getWorldPosition());
	
		left_bone_thumb[i] = Primitives::createCapsule(radius, length_thumb_left[i]);
		left_bone_index[i] = Primitives::createCapsule(radius, length_index_left[i]);
		left_bone_middle[i] = Primitives::createCapsule(radius, length_middle_left[i]);
		left_bone_ring[i] = Primitives::createCapsule(radius, length_ring_left[i]);
		left_bone_pinky[i] = Primitives::createCapsule(radius, length_pinky_left[i]);
		
		left_bone_thumb[i]->setMaterial("skin", 0);
		left_bone_index[i]->setMaterial("skin", 0);
		left_bone_middle[i]->setMaterial("skin", 0);
		left_bone_ring[i]->setMaterial("skin", 0);
		left_bone_pinky[i]->setMaterial("skin", 0);
		
		left_bone_thumb[i]->setCollider(0);
		left_bone_index[i]->setCollider(0);
		left_bone_middle[i]->setCollider(0);
		left_bone_ring[i]->setCollider(0);
		left_bone_pinky[i]->setCollider(0);

		left_bone_thumb[i]->setCastShadow(0, 0);
		left_bone_index[i]->setCastShadow(0, 0);
		left_bone_middle[i]->setCastShadow(0, 0);
		left_bone_ring[i]->setCastShadow(0, 0);
		left_bone_pinky[i]->setCastShadow(0, 0);

		left_bone_thumb[i]->setCastWorldShadow(0, 0);
		left_bone_index[i]->setCastWorldShadow(0, 0);
		left_bone_middle[i]->setCastWorldShadow(0, 0);
		left_bone_ring[i]->setCastWorldShadow(0, 0);
		left_bone_pinky[i]->setCastWorldShadow(0, 0);


		right_bone_thumb[i] = Primitives::createCapsule(radius, length_thumb_right[i]);
		right_bone_index[i] = Primitives::createCapsule(radius, length_index_right[i]);
		right_bone_middle[i] = Primitives::createCapsule(radius, length_middle_right[i]);
		right_bone_ring[i] = Primitives::createCapsule(radius, length_ring_right[i]);
		right_bone_pinky[i] = Primitives::createCapsule(radius, length_pinky_right[i]);

		right_bone_thumb[i]->setMaterial("skin", 0);
		right_bone_index[i]->setMaterial("skin", 0);
		right_bone_middle[i]->setMaterial("skin", 0);
		right_bone_ring[i]->setMaterial("skin", 0);
		right_bone_pinky[i]->setMaterial("skin", 0);

		right_bone_thumb[i]->setCollider(0);
		right_bone_index[i]->setCollider(0);
		right_bone_middle[i]->setCollider(0);
		right_bone_ring[i]->setCollider(0);
		right_bone_pinky[i]->setCollider(0);


		right_bone_thumb[i]->setCastShadow(0, 0);
		right_bone_index[i]->setCastShadow(0, 0);
		right_bone_middle[i]->setCastShadow(0, 0);
		right_bone_ring[i]->setCastShadow(0, 0);
		right_bone_pinky[i]->setCastShadow(0, 0);

		right_bone_thumb[i]->setCastWorldShadow(0, 0);
		right_bone_index[i]->setCastWorldShadow(0, 0);
		right_bone_middle[i]->setCastWorldShadow(0, 0);
		right_bone_ring[i]->setCastWorldShadow(0, 0);
		right_bone_pinky[i]->setCastWorldShadow(0, 0);

	}

	length_index_thumb_left = getDistance(finger_joint_thumb_left[2]->getWorldPosition(), finger_joint_index_left[1]->getWorldPosition());
	left_bone_index_thumb = Primitives::createCapsule(radius, length_index_thumb_left);
	left_bone_index_thumb->setMaterial("skin",0);
	left_bone_index_thumb->setCollider(0);
	left_bone_index_thumb->setCastShadow(0, 0);
	left_bone_index_thumb->setCastWorldShadow(0, 0);

	length_arm_left = getDistance(arm_joint_left->getWorldPosition(), wrist_joint_left->getWorldPosition());
	left_bone_arm = Primitives::createCapsule(radius, length_arm_left);
	left_bone_arm->setMaterial("skin",0);
	left_bone_arm->setCollider(0);
	left_bone_arm->setCastShadow(0, 0);
	left_bone_arm->setCastWorldShadow(0, 0);

	length_hand_holder_left = getDistance(finger_joint_index_left[4]->getWorldPosition(), finger_joint_ring_left[4]->getWorldPosition());
	left_bone_index_pinky = Primitives::createCapsule(radius, length_hand_holder_left);
	left_bone_index_pinky->setCollider(0);
	left_bone_index_pinky->setEnabled(0);
	left_bone_index_pinky->setCastShadow(0, 0);
	left_bone_index_pinky->setCastWorldShadow(0, 0);

	length_index_thumb_right = getDistance(finger_joint_thumb_right[2]->getWorldPosition(), finger_joint_index_right[1]->getWorldPosition());
	right_bone_index_thumb = Primitives::createCapsule(radius, length_index_thumb_right);
	right_bone_index_thumb->setMaterial("skin",0);
	right_bone_index_thumb->setCollider(0);
	right_bone_index_thumb->setCastShadow(0, 0);
	right_bone_index_thumb->setCastWorldShadow(0, 0);

	length_arm_right = getDistance(arm_joint_right->getWorldPosition(), wrist_joint_right->getWorldPosition());
	right_bone_arm = Primitives::createCapsule(radius, length_arm_right);
	right_bone_arm->setMaterial("skin",0);
	right_bone_arm->setCollider(0);
	right_bone_arm->setCastShadow(0, 0);
	right_bone_arm->setCastWorldShadow(0, 0);

	length_hand_holder_right = getDistance(finger_joint_index_right[4]->getWorldPosition(), finger_joint_ring_right[4]->getWorldPosition());
	right_bone_index_pinky = Primitives::createCapsule(radius, length_hand_holder_right);
	right_bone_index_pinky->setCollider(0);
	right_bone_index_pinky->setEnabled(0);
	right_bone_index_pinky->setCastShadow(0, 0);
	right_bone_index_pinky->setCastWorldShadow(0, 0);

	left_bone_thumb[0]->setEnabled(0);
	right_bone_thumb[0]->setEnabled(0);
}

void LeapMotion::draw_bones()
{
	create_bones();

	for (int i = 0; i < 4; i++)
	{
	
		left_bone_thumb[i]->setWorldPosition(getMiddlePosition(finger_joint_thumb_left[i+1]->getWorldPosition(), finger_joint_thumb_left[i]->getWorldPosition()));
		left_bone_thumb[i]->setDirection(getEntityDirection(finger_joint_thumb_left[i + 1]->getWorldPosition(), finger_joint_thumb_left[i]->getWorldPosition()), vec3::UP);
		left_bone_index[i]->setWorldPosition(getMiddlePosition(finger_joint_index_left[i + 1]->getWorldPosition(), finger_joint_index_left[i]->getWorldPosition()));
		left_bone_index[i]->setDirection(getEntityDirection(finger_joint_index_left[i + 1]->getWorldPosition(), finger_joint_index_left[i]->getWorldPosition()), vec3::UP);
		left_bone_middle[i]->setWorldPosition(getMiddlePosition(finger_joint_middle_left[i + 1]->getWorldPosition(), finger_joint_middle_left[i]->getWorldPosition()));
		left_bone_middle[i]->setDirection(getEntityDirection(finger_joint_middle_left[i + 1]->getWorldPosition(), finger_joint_middle_left[i]->getWorldPosition()), vec3::UP);
		left_bone_ring[i]->setWorldPosition(getMiddlePosition(finger_joint_ring_left[i + 1]->getWorldPosition(), finger_joint_ring_left[i]->getWorldPosition()));
		left_bone_ring[i]->setDirection(getEntityDirection(finger_joint_ring_left[i + 1]->getWorldPosition(), finger_joint_ring_left[i]->getWorldPosition()), vec3::UP);
		left_bone_pinky[i]->setWorldPosition(getMiddlePosition(finger_joint_pinky_left[i + 1]->getWorldPosition(), finger_joint_pinky_left[i]->getWorldPosition()));
		left_bone_pinky[i]->setDirection(getEntityDirection(finger_joint_pinky_left[i + 1]->getWorldPosition(), finger_joint_pinky_left[i]->getWorldPosition()), vec3::UP);

		right_bone_thumb[i]->setWorldPosition(getMiddlePosition(finger_joint_thumb_right[i + 1]->getWorldPosition(), finger_joint_thumb_right[i]->getWorldPosition()));
		right_bone_thumb[i]->setDirection(getEntityDirection(finger_joint_thumb_right[i + 1]->getWorldPosition(), finger_joint_thumb_right[i]->getWorldPosition()), vec3::UP);
		right_bone_index[i]->setWorldPosition(getMiddlePosition(finger_joint_index_right[i + 1]->getWorldPosition(), finger_joint_index_right[i]->getWorldPosition()));
		right_bone_index[i]->setDirection(getEntityDirection(finger_joint_index_right[i + 1]->getWorldPosition(), finger_joint_index_right[i]->getWorldPosition()), vec3::UP);
		right_bone_middle[i]->setWorldPosition(getMiddlePosition(finger_joint_middle_right[i + 1]->getWorldPosition(), finger_joint_middle_right[i]->getWorldPosition()));
		right_bone_middle[i]->setDirection(getEntityDirection(finger_joint_middle_right[i + 1]->getWorldPosition(), finger_joint_middle_right[i]->getWorldPosition()), vec3::UP);
		right_bone_ring[i]->setWorldPosition(getMiddlePosition(finger_joint_ring_right[i + 1]->getWorldPosition(), finger_joint_ring_right[i]->getWorldPosition()));
		right_bone_ring[i]->setDirection(getEntityDirection(finger_joint_ring_right[i + 1]->getWorldPosition(), finger_joint_ring_right[i]->getWorldPosition()), vec3::UP);
		right_bone_pinky[i]->setWorldPosition(getMiddlePosition(finger_joint_pinky_right[i + 1]->getWorldPosition(), finger_joint_pinky_right[i]->getWorldPosition()));
		right_bone_pinky[i]->setDirection(getEntityDirection(finger_joint_pinky_right[i + 1]->getWorldPosition(), finger_joint_pinky_right[i]->getWorldPosition()), vec3::UP);
	
	
	}
	
	left_bone_index_thumb->setWorldPosition(getMiddlePosition(finger_joint_thumb_left[2]->getWorldPosition(), finger_joint_index_left[1]->getWorldPosition()));
	left_bone_index_thumb->setDirection(getEntityDirection(finger_joint_thumb_left[2]->getWorldPosition(), finger_joint_index_left[1]->getWorldPosition()), vec3::UP);

	left_bone_arm->setWorldPosition(getMiddlePosition(arm_joint_left->getWorldPosition(), wrist_joint_left->getWorldPosition()));
	left_bone_arm->setDirection(getEntityDirection(arm_joint_left->getWorldPosition(), wrist_joint_left->getWorldPosition()), vec3::UP);

	right_bone_index_thumb->setWorldPosition(getMiddlePosition(finger_joint_thumb_right[2]->getWorldPosition(), finger_joint_index_right[1]->getWorldPosition()));
	right_bone_index_thumb->setDirection(getEntityDirection(finger_joint_thumb_right[2]->getWorldPosition(), finger_joint_index_right[1]->getWorldPosition()), vec3::UP);

	right_bone_arm->setWorldPosition(getMiddlePosition(arm_joint_right->getWorldPosition(), wrist_joint_right->getWorldPosition()));
	right_bone_arm->setDirection(getEntityDirection(arm_joint_right->getWorldPosition(), wrist_joint_right->getWorldPosition()), vec3::UP);


	left_bone_index_pinky->setWorldPosition(getMiddlePosition(finger_joint_index_left[4]->getWorldPosition(), finger_joint_pinky_left[4]->getWorldPosition()));
	left_bone_index_pinky->setDirection(getEntityDirection(finger_joint_pinky_left[4]->getWorldPosition(), finger_joint_index_left[4]->getWorldPosition()), vec3::UP);
	

	right_bone_index_pinky->setWorldPosition(getMiddlePosition(finger_joint_index_right[4]->getWorldPosition(), finger_joint_pinky_right[4]->getWorldPosition()));
	right_bone_index_pinky->setDirection(getEntityDirection(finger_joint_pinky_right[4]->getWorldPosition(), finger_joint_index_right[4]->getWorldPosition()), vec3::UP);



	//Vec3 cusPos_left = left_hand_holder->getPosition();
	//Vec3 cusPos_right = right_hand_holder->getPosition();


}

void LeapMotion::hide_joints_left(int hide)
{

	for (int i = 0; i < LeapMotionInterface::Bone::BONE_NUM_TYPES + 1; i++)
	{
		finger_joint_thumb_left[i]->setEnabled(hide);
		finger_joint_index_left[i]->setEnabled(hide);
		finger_joint_middle_left[i]->setEnabled(hide);
		finger_joint_ring_left[i]->setEnabled(hide);
		finger_joint_pinky_left[i]->setEnabled(hide);
	}

	for (int i = 0; i < 4; i++)
	{
		if (left_bone_thumb[i])
		{
			left_bone_thumb[i]->setEnabled(hide);
			left_bone_index[i]->setEnabled(hide);
			left_bone_middle[i]->setEnabled(hide);
			left_bone_ring[i]->setEnabled(hide);
			left_bone_pinky[i]->setEnabled(hide);
		}	
		
	}

	if (left_bone_index_thumb)
	{
		left_bone_index_thumb->setEnabled(hide);
	}
	if (left_bone_arm)
	{
		left_bone_arm->setEnabled(hide);
	}
	if (left_hand_holder)
	{
		left_hand_holder->setEnabled(hide);
	}

	if (wrist_joint_left)
	{
		wrist_joint_left->setEnabled(hide);
	}

	arm_joint_left->setEnabled(hide);
}

void LeapMotion::hide_joints_right(int hide)
{
//palm_bone_right->setEnabled(hide);
//wrist_bone_right->setEnabled(hide);
//arm_bone_right->setEnabled(hide);
	for (int i = 0; i < LeapMotionInterface::Bone::BONE_NUM_TYPES + 1; i++)
	{
		finger_joint_thumb_right[i]->setEnabled(hide);
		finger_joint_index_right[i]->setEnabled(hide);
		finger_joint_middle_right[i]->setEnabled(hide);
		finger_joint_ring_right[i]->setEnabled(hide);
		finger_joint_pinky_right[i]->setEnabled(hide);
	}
	
	for (int i = 0; i < 4; i++)
	{
		if (right_bone_thumb[i])
		{
			right_bone_thumb[i]->setEnabled(hide);
			right_bone_index[i]->setEnabled(hide);
			right_bone_middle[i]->setEnabled(hide);
			right_bone_ring[i]->setEnabled(hide);
			right_bone_pinky[i]->setEnabled(hide);
		}
	}

	if (right_bone_index_thumb)
	{
		right_bone_index_thumb->setEnabled(hide);
	}
	if (right_bone_arm)
	{
		right_bone_arm->setEnabled(hide);
	}	
	if (right_hand_holder)
	{
		right_hand_holder->setEnabled(hide);
	}
	if (wrist_joint_right)
	{
		wrist_joint_right->setEnabled(hide);
	}

	arm_joint_right->setEnabled(hide);
}

//update bone's transform in mesh
void LeapMotion::changeMeshBonesTransform(int leftorright,Mat4 transform, int num)
{
	if (leftorright == 0)
	{
	//	transform.setScale(Vec3(0.01, 0.01, 0.01));
		transform.getTranslate();
		quat current_rotation = Resource::get()->LeftHand_Mesh_LeapMotion->getWorldBoneTransform(num).getRotate();
		Mat4 new_transform;
		composeTransform(new_transform, transform.getTranslate(), current_rotation, vec3(0.01));
		Resource::get()->LeftHand_Mesh_LeapMotion->setWorldBoneTransform(num, new_transform);
	}
	else
	{
	//	transform.setScale(Vec3(0.01, 0.01, 0.01));
		transform.getTranslate();
		quat current_rotation = Resource::get()->RightHand_Mesh_LeapMotion->getWorldBoneTransform(num).getRotate();
		Mat4 new_transform;
		composeTransform(new_transform, transform.getTranslate(), current_rotation, vec3(0.01));
		Resource::get()->RightHand_Mesh_LeapMotion->setWorldBoneTransform(num, new_transform);
		Resource::get()->RightHand_Mesh_LeapMotion->setWorldBoneTransform(num, transform);
	}
	
}


int LeapMotion::getNumHands()
{
	return 2;
}

int LeapMotion::getHandState(int num)
{
	return hand_state[num];
}

void LeapMotion::grab_update(int num, int closed_hand)
{

	// hide outline
	if (last_selected_timer[num] >= 0)
	{
		last_selected_timer[num] -= Game::get()->getIFps() / Game::get()->getScale();
		if (last_selected_timer[num] < 0)
			set_outline(num, 0);
	}

	if (node_grabbed[num] == 0)
	{
		hand_state[num] = HAND_FREE;
		hitObj = last_selected[num];
		// get intersections, find new hitObj
		if (last_selected_timer[num] < 0)
		{
			if (grab_mode == GRAB_MODE::BOUNDBOX)
			{
				hitObj = ObjectPtr(); // null pointer

				intersections.clear();

				CurrentHandNode = getHandNode(num);

				if (CurrentHandNode)
				{
					if (World::get()->getIntersection(CurrentHandNode->getWorldBoundBox(), intersections))
					{

						// find nearest interactive object (can i grab it?)
						int nearest = -1;
						Scalar min_dist = UNIGINE_INFINITY;
						for (int k = 0; k < intersections.size(); k++)
						{
							if (can_i_grab_it(intersections[k]->getNode()))
							{
								Scalar dist = length2(intersections[k]->getWorldPosition() - CurrentHandNode->getWorldPosition());
								if (min_dist > dist)
								{
									min_dist = dist;
									nearest = k;
								}
							}
						}

						// select nearest object
						if (nearest >= 0)
						{
							hitObj = intersections[nearest];

						}
					}
				}

			}
			else if (grab_mode == GRAB_MODE::INTERSECTIONS)
			{
				Vec3 pos1 = CurrentHandNode->getPosition() - Vec3(getWorldDown(CurrentHandNode))* ray_back_length;
				Vec3 pos2 = CurrentHandNode->getPosition() + Vec3(getWorldDown(CurrentHandNode))* ray_forward_length;

				hitObj = World::get()->getIntersection(pos1, pos2, 1, intersection);
				if (!hitObj) // second intersection (for best player's experience)
					hitObj = World::get()->getIntersection(pos1, pos2 + Vec3(getWorldLeft(CurrentHandNode)) * ptr_width * 0.5f, 1, intersection);
			}
		}

		if (hitObj)
		{

			// check if this object grabbed by another controller
			for (int z = 0; z < grab_node.size(); z++)
				if (z != num && node_grabbed[z] && hitObj->getNode() == grab_node[z])
					return;

			// all right. It's movable or switcher?
			if (can_i_grab_it(hitObj->getNode()))
			{
				//	vibrateController(num, 1);
				// show controller's outline
				last_selected[num] = hitObj;
				if (last_selected_timer[num] < 0)
					last_selected_timer[num] = 0.1f;
				set_outline(num, 1);

				// ... and grab it (or touch)
				if (closed_hand)
				{
					last_selected_timer[num] = 0;
					grab_node[num] = hitObj->getNode();
					node_grabbed[num] = 1;
					hand_state[num] = HAND_GRAB;
					isHolding[num] = true;
				}
			}
		}
	}
	else
	{

		if (isHolding[num] && closed_hand)
		{
			// hold
			hand_state[num] = HAND_HOLD;	
		}
		else  if (isHolding[num] && closed_hand == 0)
		{

			//throw!
			node_grabbed[num] = 0;
			hand_state[num] = HAND_THROW;
			isHolding[num] = false;
		}

	}


}



//set the material of the controller and grabbed node
void LeapMotion::set_outline(int num, int enable)
{

	if (node_selected[num] == enable)
		return;

	node_selected[num] = enable;

	// set material state (to handmesh)
	/*
	if (num == 0)
	{
		for (int i = 0; i < 5; i++)
		{
			finger_joint_thumb_left[i]->setMaterialState("auxiliary", enable, 0);
			finger_joint_index_left[i]->setMaterialState("auxiliary", enable, 0);
			finger_joint_middle_left[i]->setMaterialState("auxiliary", enable, 0);
			finger_joint_ring_left[i]->setMaterialState("auxiliary", enable, 0);
			finger_joint_pinky_left[i]->setMaterialState("auxiliary", enable, 0);
			left_bone_thumb[i]->setMaterialState("auxiliary", enable, 0);
			left_bone_index[i]->setMaterialState("auxiliary", enable, 0);
			left_bone_middle[i]->setMaterialState("auxiliary", enable, 0);
			left_bone_ring[i]->setMaterialState("auxiliary", enable, 0);
			left_bone_pinky[i]->setMaterialState("auxiliary", enable, 0);
		}
			left_bone_index_thumb->setMaterialState("auxiliary", enable, 0);
			left_bone_arm->setMaterialState("auxiliary", enable, 0);
	}
	else
		if (num == 1)
		{
			for (int i = 0; i < 5; i++)
			{
				finger_joint_thumb_right[i]->setMaterialState("auxiliary", enable, 0);
				finger_joint_index_right[i]->setMaterialState("auxiliary", enable, 0);
				finger_joint_middle_right[i]->setMaterialState("auxiliary", enable, 0);
				finger_joint_ring_right[i]->setMaterialState("auxiliary", enable, 0);
				finger_joint_pinky_right[i]->setMaterialState("auxiliary", enable, 0);
				right_bone_thumb[i]->setMaterialState("auxiliary", enable, 0);
				right_bone_index[i]->setMaterialState("auxiliary", enable, 0);
				right_bone_middle[i]->setMaterialState("auxiliary", enable, 0);
				right_bone_ring[i]->setMaterialState("auxiliary", enable, 0);
				right_bone_pinky[i]->setMaterialState("auxiliary", enable, 0);
			}
			right_bone_index_thumb->setMaterialState("auxiliary", enable, 0);
			right_bone_arm->setMaterialState("auxiliary", enable, 0);
		}
		*/
	// set material state (to object)
	if (last_selected[num])
		for (int i = 0; i < last_selected[num]->getNumSurfaces(); i++)
			last_selected[num]->setMaterialState("auxiliary", enable, i);

}

//if it has grable
int LeapMotion::can_i_grab_it(const NodePtr &node)
{

	PropertyPtr prop = node->getProperty();
	if (prop)
	{
		int index = prop->findParameter("grable");
		if (index != -1)
			return 1;
	}
	return 0;
}

void LeapMotion::grab_init()
{
	hand_state.clear();
	node_grabbed.clear();
	node_selected.clear();
	last_selected.clear();
	last_selected_timer.clear();
	grab_node.clear();

	NodePtr node_null;
	ObjectPtr obj_null;

	hand_linear_velocity.clear();
	hand_angular_velocity.clear();

	for (int i = 0; i < HAND_COUNT; i++)
	{
		grab_node.append(node_null);
		hand_state.append(0);
		node_grabbed.append(0);
		last_selected.append(obj_null);
		last_selected_timer.append(0);
		node_selected.append(0);
		isHolding.append(0);

		Vector<vec3> lv;
		hand_linear_velocity.append(lv);

		Vector<vec3> av;
		hand_angular_velocity.append(av);
	}


}

vec3 LeapMotion::getHandLinearVelocity(int num)
{
	return linearRegression(hand_linear_velocity[num]);
}
vec3 LeapMotion::getHandAngularVelocity(int num)
{
	return linearRegression(hand_angular_velocity[num]);
}
NodePtr LeapMotion::getHandNode(int num)
{

	if (num == 0)
	{
		CurrentHandNode = left_bone_index_pinky->getNode();
	}
	else
	{
		CurrentHandNode = right_bone_index_pinky->getNode();
	}

	return CurrentHandNode;
}


int LeapMotion::getHandDegreesOfFreedom(int num)
{
	return 6; // x, y, z + pitch, yaw, roll
}


NodePtr LeapMotion::getGrabNode(int num)
{
	return grab_node[num];
}


//get speed of glove

void LeapMotion::push_hand_linear_velocity(int num, const vec3 &velocity)
{
	Vector<vec3> *buffer = &hand_linear_velocity[num];

	if (buffer->size() < 2)
		buffer->append(velocity);
	else
	{
		for (int i = 0; i < buffer->size() - 1; i++)
			buffer->get(i) = buffer->get(i + 1);

		buffer->get(buffer->size() - 1) = velocity;
	}
}


//get controller speed
vec3 LeapMotion::getworldVelocity(NodePtr node, Vec3 lastPosition)
{
	if (elapsed_time_velocity_leapmotion >= 0)
	{
		elapsed_time_velocity_leapmotion -= Game::get()->getIFps();
		//	Log::message("%f   \n",  elapsed_time_velocity);
	}
	if (elapsed_time_velocity_leapmotion <0)
	{
		Vec3 currentposition = node->getWorldPosition();

		float x = currentposition.x - lastPosition.x;
		float y = currentposition.y - lastPosition.y;
		float z = currentposition.z - lastPosition.z;
		lastPosition = currentposition;
		// resetting elapsed time counter
		elapsed_time_velocity_leapmotion = CHANGE_INTERVAL_VELOCITY_LEAPMOTION;
		//0.1 second change to 1s
		return vec3(x * 100, y * 100, z * 100);
	}
	else
		return vec3(0, 0, 0);
}