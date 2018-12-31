#pragma once

#include <UnigineWidgets.h>
#include <UnigineGui.h>
#include <UnigineEngine.h>
#include <UnigineEditor.h>
#include <UnigineGame.h>
#include <UnigineApp.h>
#include <UnigineNode.h>
#include <UnigineConsole.h>
#include <UnigineRender.h>
#include <UnigineUserInterface.h>
#include <iostream> 
#include <UnigineMaterial.h>
#include <UnigineMaterials.h>

using namespace Unigine;


class UI
{
public:
	 void init();

	 void shutdown();
	

	 void updateOperation();
	 void GetFlirDistance();
	
	 int weightType;

	 WidgetButtonPtr hideClouds;
	 WidgetButtonPtr followHelicopter;

	 WidgetEditTextPtr LongitudeText;
	 WidgetEditTextPtr AltitudeText;
	 WidgetEditTextPtr FovText;
	 WidgetEditTextPtr LockText;
	 WidgetSpritePtr aim;

	
private:
	GuiPtr gui;
	WidgetSpriteViewportPtr viewport1;
	WidgetSpriteViewportPtr viewport2;
	WidgetSpriteViewportPtr viewport3;
	CameraPtr camera1;
	CameraPtr camera2;
	CameraPtr camera3;
	WidgetWindowPtr window1;
	WidgetWindowPtr window2;
	WidgetSpritePtr logo1;
	WidgetSpritePtr logo2;
	WidgetButtonPtr thirdpersonview;
	WidgetButtonPtr pilotview;
	
	WidgetComboBoxPtr weightBox;
	WidgetButtonPtr weightbutton;


	/* interface */

	UserInterfacePtr mainMenu;
	WidgetVBoxPtr main_vbox;

	WidgetButtonPtr environment_b;
	WidgetVBoxPtr environment_vbox;
	WidgetLabelPtr construction_l;
	WidgetSliderPtr construction_sl;
	WidgetSliderPtr time_sl;
	WidgetSliderPtr weather_sl;

	WidgetButtonPtr quality_b;
	WidgetVBoxPtr quality_vbox;
	WidgetComboBoxPtr clouds_sampling_quality_cb;
	WidgetComboBoxPtr clouds_lighting_quality_cb;
	WidgetComboBoxPtr clouds_downsampling_cb;
	WidgetSliderPtr wind_speed_sl;

	
	VectorStack<ObjectPtr, 64> hitObjs;



	float longitude = 0;
	float altitude = 0;
	float fov = 0;

	bool isLock;

 	void CreateWeightBox()
	{	
		weightBox = WidgetComboBox::create(gui);
		gui->addChild(weightBox->getWidget());
		
		int y = weightbutton->getScreenPositionY() + 32;
		weightBox->setPosition(10, y);


		weightBox->addItem("Weight1");
		weightBox->addItem("Weight2");
		weightBox->addItem("Weight3");

		// setting the first item as currently selected one
		weightBox->setCurrentItem(0);
		weightBox->setToolTip("Weight Option");
		weightBox->arrange();

		// adding created combobox widget to the system GUI
		Gui::get()->addChild(weightBox->getWidget(), Gui::ALIGN_OVERLAP);
		// setting onComboBoxChanged function to handle CHANGED event
		weightBox->setCallback0(Gui::CHANGED, MakeCallback(this, &UI::ChangeWeight));
	}

	void ChangeWeight() 
	{
		weightType = weightBox->getCurrentItem();

	}

	void show_panel(WidgetButtonPtr button, WidgetVBoxPtr vbox) {
		if (button) {
			vbox->arrange();
			int y = button->getScreenPositionY() + 32;
			vbox->setPosition(10, y);
			gui->addChild(vbox->getWidget(), Gui::ALIGN_OVERLAP);
		}
		
		if (&environment_b!= &button && environment_b->isToggled()) 
			environment_b->setToggled(0);
		if (&quality_b != &button && quality_b->isToggled()) 
			quality_b->setToggled(0);
	}

	void quit_clicked() {
		Console::get()->run("quit");
	}

	void lock();


};

