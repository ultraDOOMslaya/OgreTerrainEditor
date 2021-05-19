#include "GUI.h"

namespace Constants {
	/** Select Menu & other Widget Names **/
	extern const Ogre::String ChangeAlteration = "ChangeAlteration";

	extern const Ogre::String BrushSize = "BrushSize";
	extern const Ogre::String Texture01Depth = "Texture01Depth";
	extern const Ogre::String Texture02Depth = "Texture02Depth";

	extern const Ogre::String TreeModels = "TreeModels";

	extern const Ogre::String GrassDodad = "GrassDodad";
	extern const Ogre::String DodadHeight = "DodadHeight";
	extern const Ogre::String DodadWidth = "DodadWidth";
	extern const Ogre::String DodadRotation = "DodadRotation";

	/** Alterations **/
	extern const Ogre::String EditTypeGround = "ground";
	extern const Ogre::String EditTypeTree = "tree";
	extern const Ogre::String EditTypeDodad = "dodad";

	/** 2D gress meshes **/
	extern const Ogre::String GrassTuft03Mesh = "GrassTuft03Mesh";
	extern const Ogre::String GrassTuft05Mesh = "GrassTuft05Mesh";

	/** 3D tree meshes **/
	extern const Ogre::String Birch01 = "BirchTree.001.mesh";
	extern const Ogre::String Birch02 = "leaves.484.mesh";
}

GUI::GUI(OgreBites::TrayManager* trayManager)
	: mGameObjectDelete(false)
{
	mTrayMgr = trayManager;

	/** Alteration type selection **/
	mSelectAlterationSM = mTrayMgr->createThickSelectMenu(OgreBites::TL_TOPRIGHT, Constants::ChangeAlteration, "Change Alteration:", 180.0f, 3, { Constants::EditTypeGround, Constants::EditTypeTree, Constants::EditTypeDodad });

	mBrushSizeSlider = mTrayMgr->createThickSlider(OgreBites::TL_TOPRIGHT, Constants::BrushSize, "Brush Size:", 180.0f, 80.0f, 1, 4, 4);
	mTexture01Depth = mTrayMgr->createThickSlider(OgreBites::TL_TOPRIGHT, Constants::Texture01Depth, "Greenery:", 180.0f, 80.0f, 0.0, 1.0, 20);
	mTexture01Depth->setValue(0.1);
	mTexture02Depth = mTrayMgr->createThickSlider(OgreBites::TL_TOPRIGHT, Constants::Texture02Depth, "Tundra:", 180.0f, 80.0f, 0.0, 1.0, 20);
	mTexture02Depth->setValue(0.1);

	/** Game Objects **/
	mSelectGameObject = mTrayMgr->createThickSelectMenu(OgreBites::TL_NONE, Constants::TreeModels, "Tree Mesh:", 180.0f, 2, { Constants::Birch01, Constants::Birch02 });
	mSelectGameObject->hide();


	/** Dodad Alterations **/
	mSelectGrassDodad = mTrayMgr->createThickSelectMenu(OgreBites::TL_NONE, Constants::GrassDodad, "Grass Brush:", 180.0f, 2, { "GrassTuft03Mesh", Constants::GrassTuft05Mesh });
	mDodadHeight = mTrayMgr->createThickSlider(OgreBites::TL_NONE, Constants::DodadHeight, "Dodad Height:", 180.0f, 80.0f, 0.5, 1.5, 10);
	mDodadHeight->setValue(1);
	mDodadWidth = mTrayMgr->createThickSlider(OgreBites::TL_NONE, Constants::DodadWidth, "Dodad Width:", 180.0f, 80.0f, 0.5, 1, 5);
	mDodadWidth->setValue(1);
	mDodadRotation = mTrayMgr->createThickSlider(OgreBites::TL_NONE, Constants::DodadRotation, "Dodad Rotation:", 180.0f, 80.0f, 0, 360, 360);
	mDodadDelete = mTrayMgr->createCheckBox(OgreBites::TL_NONE, "DeleteDodad", "Remove Dodad:", 180.0f);
	mDodadHeight->hide();
	mDodadWidth->hide();
	mDodadRotation->hide();
	mSelectGrassDodad->hide();
	mDodadDelete->hide();
}


GUI::~GUI()
{
}


void GUI::toggleGameObjectDelete() {
	mGameObjectDelete = !mGameObjectDelete;
	mDodadDelete->setChecked(mGameObjectDelete);
}
//----------------------------------------------------------------

bool GUI::deleteGameObject() {
	return mGameObjectDelete;
}
//----------------------------------------------------------------

Ogre::String GUI::currentAlteration() {
	return mSelectAlterationSM->getSelectedItem();
}
//----------------------------------------------------------------

void GUI::itemSelectedEvent(OgreBites::SelectMenu* menu) {

	if (menu->getName() == Constants::ChangeAlteration) {
		if (menu->getSelectedItem() == Constants::EditTypeGround) {
			showGameObjects(false);
			showDodads(false);
			showTextures(true);
		}
		else if (menu->getSelectedItem() == Constants::EditTypeTree) {
			showTextures(false);
			showDodads(false);
			showGameObjects(true);
			
		}
		else if (menu->getSelectedItem() == Constants::EditTypeDodad) {
			showTextures(false);
			showGameObjects(false);
			showDodads(true);
		}
	}
}
//----------------------------------------------------------------

void GUI::showTextures(bool show) {
	if (show) {
		mBrushSizeSlider->show();
		mTrayMgr->moveWidgetToTray(OgreBites::TL_NONE, mBrushSizeSlider->getName(), OgreBites::TL_TOPRIGHT);
		mTexture01Depth->show();
		mTrayMgr->moveWidgetToTray(OgreBites::TL_NONE, mTexture01Depth->getName(), OgreBites::TL_TOPRIGHT);
		mTexture02Depth->show();
		mTrayMgr->moveWidgetToTray(OgreBites::TL_NONE, mTexture02Depth->getName(), OgreBites::TL_TOPRIGHT);
	}
	else {
		mBrushSizeSlider->hide();
		mTrayMgr->removeWidgetFromTray(mBrushSizeSlider->getName());
		mTexture01Depth->hide();
		mTrayMgr->removeWidgetFromTray(mTexture01Depth->getName());
		mTexture02Depth->hide();
		mTrayMgr->removeWidgetFromTray(mTexture02Depth->getName());
	}
}
//----------------------------------------------------------------

void GUI::showGameObjects(bool show) {
	if (show) {
		mSelectGameObject->show();
		mTrayMgr->moveWidgetToTray(OgreBites::TL_NONE, mSelectGameObject->getName(), OgreBites::TL_TOPRIGHT);

		mDodadDelete->show();
		mTrayMgr->moveWidgetToTray(mDodadDelete->getName(), OgreBites::TL_TOPRIGHT);
	}
	else {
		mSelectGameObject->hide();
		mTrayMgr->removeWidgetFromTray(mSelectGameObject->getName());

		mDodadDelete->hide();
		mTrayMgr->removeWidgetFromTray(mDodadDelete->getName());
	}
}
//----------------------------------------------------------------

void GUI::showDodads(bool show) {
	if (show) {
		mSelectGrassDodad->show();
		mTrayMgr->moveWidgetToTray(OgreBites::TL_NONE, mSelectGrassDodad->getName(), OgreBites::TL_TOPRIGHT);
		mDodadHeight->show();
		mTrayMgr->moveWidgetToTray(OgreBites::TL_NONE, mDodadHeight->getName(), OgreBites::TL_TOPRIGHT);
		mDodadWidth->show();
		mTrayMgr->moveWidgetToTray(OgreBites::TL_NONE, mDodadWidth->getName(), OgreBites::TL_TOPRIGHT);
		mDodadRotation->show();
		mTrayMgr->moveWidgetToTray(OgreBites::TL_NONE, mDodadRotation->getName(), OgreBites::TL_TOPRIGHT);

		mDodadDelete->show();
		mTrayMgr->moveWidgetToTray(mDodadDelete->getName(), OgreBites::TL_TOPRIGHT);
	}
	else {
		mSelectGrassDodad->hide();
		mTrayMgr->removeWidgetFromTray(mSelectGrassDodad->getName());
		mDodadHeight->hide();
		mTrayMgr->removeWidgetFromTray(mDodadHeight->getName());
		mDodadWidth->hide();
		mTrayMgr->removeWidgetFromTray(mDodadWidth->getName());
		mDodadRotation->hide();
		mTrayMgr->removeWidgetFromTray(mDodadRotation->getName());

		mDodadDelete->hide();
		mTrayMgr->removeWidgetFromTray(mDodadDelete->getName());
	}
}
//----------------------------------------------------------------