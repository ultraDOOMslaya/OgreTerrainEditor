#pragma once
#ifndef __SimRunnable_h_
#define __SimRunnable_h_

#include <Ogre.h>
#include <OgreApplicationContext.h>
#include <OgreBitesConfigDialog.h>

#include <OgreOverlayManager.h>
#include <OgreOverlay.h>
#include <OgreOverlayContainer.h>
#include <OgreOverlayElementFactory.h>

#include <OgreTrays.h>
#include <SDL.h>
#include <iostream>
//#include "SDL2-2.0.9/include/SDL.h"

#include <OgreTerrain.h>
#include <OgreTerrainGroup.h>

struct Cell {
	int x;
	int y;
	int unpassable;
};

class SimRunnable : public OgreBites::ApplicationContext, public OgreBites::InputListener, public OgreBites::TrayListener
{
public:
	SimRunnable();
	~SimRunnable();

	virtual void frameRendered(const Ogre::FrameEvent& evt);
	void alterTerrain(Ogre::Terrain* terrain, Ogre::Vector3 centerPosition, Ogre::Vector2 gridCoordinates);
	bool mouseReleased(const OgreBites::MouseButtonEvent& evt);
	void buttonHit(OgreBites::Button* btn);

	Ogre::Vector2 updateCoords(Ogre::Vector3 centerPosition);
	void wipeTerrain();

	void serialize();
	void setup(void);

	void configureTerrainDefaults(Ogre::Light* light);
	void initBlendMaps(Ogre::Terrain* terrain);
	void defineTerrain(long x, long y);

	bool mTerrainsImported;
	Ogre::TerrainGroup* mTerrainGroup;
	Ogre::TerrainGlobalOptions* mTerrainGlobals;

	OgreBites::CheckBox* mMoveableCamera;
	OgreBites::Label* mInfoLabel = nullptr;
	OgreBites::Slider* mElevationSlider;
	OgreBites::Slider* mBrushSizeSlider;
	OgreBites::SelectMenu* mEditTextureSM;
	OgreBites::TextBox* mCoordsBox;
	OgreBites::Button* mNewBtn;
	OgreBites::Button* mLoadBtn;
	OgreBites::Button* mSaveBtn;

	OgreBites::TrayManager* mTrayMgr;
	Ogre::SceneManager* mScnMgr;
	Ogre::RaySceneQuery* mRayScnQuery;
	
	Ogre::Camera* mCam;
	Ogre::RenderWindow* mWindow;
	Ogre::SceneNode* camNode;
	Ogre::SceneNode* camAnchor;

	/** Map Dimensions **/
	float width, height;
	std::vector<Cell> worldSpaceCells;

};

#endif __SimRunnable_h_