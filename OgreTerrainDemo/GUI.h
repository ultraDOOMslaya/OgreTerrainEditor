#pragma once
#ifndef __GUI_h_
#define __GUI_h_

#include <Ogre.h>
#include <OgreApplicationContext.h>
#include <OgreBitesConfigDialog.h>

#include <OgreOverlayManager.h>
#include <OgreOverlay.h>
#include <OgreOverlayContainer.h>
#include <OgreOverlayElementFactory.h>

#include <OgreTrays.h>

class GUI
{
public:
	GUI(OgreBites::TrayManager* trayManager);
	~GUI();

	void toggleGameObjectDelete();
	void itemSelectedEvent(OgreBites::SelectMenu* menu);
	Ogre::String GUI::currentAlteration();
	void showTextures(bool show);
	void showGameObjects(bool show);
	void showDodads(bool show);

	bool deleteGameObject();


	OgreBites::TrayManager* mTrayMgr;

	OgreBites::CheckBox* mMoveableCamera;
	OgreBites::Label* mInfoLabel = nullptr;
	OgreBites::Slider* mElevationSlider;

	/** Alteration type selection **/
	OgreBites::SelectMenu* mSelectAlterationSM;

	/** Ground Alterations **/
	OgreBites::Slider* mBrushSizeSlider;
	OgreBites::Slider* mTexture01Depth;
	OgreBites::Slider* mTexture02Depth;

	/** Game Objects **/
	OgreBites::SelectMenu* mSelectGameObject;
	//OgreBites::Slider* mGameObjectScale;

	/** Dodad Alterations **/
	OgreBites::SelectMenu* mSelectGrassDodad;
	OgreBites::Slider* mDodadHeight;
	OgreBites::Slider* mDodadWidth;
	OgreBites::Slider* mDodadRotation;
	OgreBites::CheckBox* mDodadDelete;

	OgreBites::TextBox* mCoordsBox;
	OgreBites::Button* mNewBtn;
	OgreBites::Button* mLoadBtn;
	OgreBites::Button* mSaveBtn;

	bool mGameObjectDelete;
};

#endif __GUI_h_