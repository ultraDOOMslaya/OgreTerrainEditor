#include "SimRunnable.h"


namespace Offsets {
	/** Map builders **/
	extern const float worldSize = 2000;
	extern const float baseHeight = 200;
	
	extern const int cellSize = 80;
	extern const float heightOffset = 200;

	extern const float brushSize = 0.025;
}

namespace TerrainType {
	extern const Ogre::String Grass = "grass";
	extern const Ogre::String Rock = "rock";
	extern const Ogre::String Sand = "sand";
}

SimRunnable::SimRunnable()
	: mTerrainGroup(0),
	mTerrainGlobals(0),
	mInfoLabel(0)
{
}
//----------------------------------------------------------------

SimRunnable::~SimRunnable()
{
}
//----------------------------------------------------------------

void SimRunnable::serialize()
{
	std::ofstream oFile("terrain_bogaloo.txt");

	for (auto cell : worldSpaceCells)
	{
		oFile << cell.x << ' ' << cell.y << ' ' << cell.unpassable << '\n';
	}
	oFile.close();
}
//----------------------------------------------------------------

void getCellCenter(Ogre::Vector3 centerPosition, Ogre::Vector3* centerCellPosition)
{
	int x = centerPosition.x;
	int z = centerPosition.z;

	/*int cell_x = x % Offsets::cellSize > 0 ? (x / Offsets::cellSize) + 1 : x / Offsets::cellSize;
	int cell_z = z % Offsets::cellSize > 0 ? (z / Offsets::cellSize) + 1 : z / Offsets::cellSize;*/

	int cell_x = x / Offsets::cellSize;
	int cell_z = z / Offsets::cellSize;

	int cell_width = (cell_x * Offsets::cellSize) + (Offsets::cellSize / 2);
	int cell_height = (cell_z * Offsets::cellSize) + (Offsets::cellSize / 2);

	centerCellPosition->x = cell_width;
	centerCellPosition->z = cell_height;
}
//----------------------------------------------------------------

void SimRunnable::buttonHit(OgreBites::Button* btn)
{
	if (btn->getName() == "Save") {
		//mTerrainGroup->saveGroupDefinition("terrain_bogaloo.dat");
		mTerrainGroup->saveAllTerrains(true);
		serialize();
	}
	else if (btn->getName() == "Load") {
		/*Ogre::String filePath = "terrain_bogaloo.dat";
		mTerrainGroup->removeAllTerrains();
		mTerrainGroup->loadGroupDefinition(filePath);
		mTerrainGroup->loadAllTerrains(true);*/
		//mTerrainGroup->loadAllTerrains(true);

	}
}
//----------------------------------------------------------------

Ogre::Vector2 SimRunnable::updateCoords(Ogre::Vector3 centerPosition)
{
	int x = centerPosition.x;
	int z = centerPosition.z;

	/*int cell_x = x % Offsets::cellSize > 0 ? (x / Offsets::cellSize) + 1 : x / Offsets::cellSize;
	int cell_z = z % Offsets::cellSize > 0 ? (z / Offsets::cellSize) + 1 : z / Offsets::cellSize;*/

	int cell_x = x / Offsets::cellSize;
	int cell_z = z / Offsets::cellSize;

	Ogre::StringStream ss = Ogre::StringStream();
	ss << "Coords x: " << x << " z: " << z << "\n";
	ss << "Cell x: " << cell_x << " z: " << cell_z << "\n";
	mCoordsBox->setText(ss.str());

	return Ogre::Vector2(cell_x, cell_z);
}
//----------------------------------------------------------------

void SimRunnable::wipeTerrain()
{
	mTerrainGroup->removeAllTerrains();
}
//----------------------------------------------------------------

void SimRunnable::alterTerrain(Ogre::Terrain* terrain, Ogre::Vector3 centerPosition, Ogre::Vector2 gridCoordinates)
{
	Ogre::Vector3 tsPos;
	Ogre::Vector3 tsCenterPos;
	Ogre::Vector3 centerCellPos;
	terrain->getTerrainPosition(centerPosition, &tsPos);
	getCellCenter(centerPosition, &centerCellPos);
	terrain->getTerrainPosition(centerCellPos, &tsCenterPos);

	// blend pointers
	Ogre::TerrainLayerBlendMap* blendMap0 = terrain->getLayerBlendMap(1);
	Ogre::TerrainLayerBlendMap* blendMap1 = terrain->getLayerBlendMap(2);
	float* pBlend0 = blendMap0->getBlendPointer();
	float* pBlend1 = blendMap1->getBlendPointer();

	Ogre::Real terrainSize = (terrain->getSize() - 1);
	long terrain_x = tsCenterPos.x * terrainSize;
	long terrain_y = tsCenterPos.y * terrainSize;
	long startx = terrain_x - ((Offsets::cellSize / 8) * mBrushSizeSlider->getValue());
	long starty = terrain_y - ((Offsets::cellSize / 8) * mBrushSizeSlider->getValue());
	long endx = terrain_x + ((Offsets::cellSize / 8) * mBrushSizeSlider->getValue());
	long endy = terrain_y + ((Offsets::cellSize / 8) * mBrushSizeSlider->getValue());
	float adjustedHeight = mElevationSlider->getValue() * Offsets::heightOffset + Offsets::baseHeight;

	
	float minHeight0 = 20;
	float fadeDist0 = 15;
	float minHeight1 = 70;
	float fadeDist1 = 15;
	

	for (long y = starty; y <= endy; ++y)
	{
		for (long x = startx; x <= endx; ++x)
		{
			float currentHeight = terrain->getHeightAtPoint(x, y);
			if (currentHeight != adjustedHeight)
			{
				terrain->setHeightAtPoint(x, y, adjustedHeight);
				mTerrainGroup->unloadTerrain(x, y);
				mTerrainGroup->loadTerrain(x, y, true);

				if (mElevationSlider->getValue() != 0)
				{
					//int cellIndex = (gridCoordinates.x - 1) + ((Offsets::worldSize / Offsets::cellSize) * (gridCoordinates.y - 1));

					int cellIndex = gridCoordinates.x + ((Offsets::worldSize / Offsets::cellSize) * gridCoordinates.y);
					Cell* cell = &worldSpaceCells.at(cellIndex);
					cell->unpassable = 1;
					//worldSpaceCells.at(cellIndex).unpassable = 1;
				}
				else {
					//int cellIndex = gridCoordinates.x + ((Offsets::worldSize / Offsets::cellSize) * gridCoordinates.y);
					int cellIndex = gridCoordinates.x + ((Offsets::worldSize / Offsets::cellSize) * gridCoordinates.y);
					worldSpaceCells.at(cellIndex).unpassable = 0;
				}
			}

		}
	}	
	
	float brushSize = mBrushSizeSlider->getValue() * Offsets::brushSize;

	int wsBlendStart_X = centerCellPos.x - (Offsets::cellSize / 2) * mBrushSizeSlider->getValue();
	int wsBlendEnd_X = centerCellPos.x + (Offsets::cellSize / 2) * mBrushSizeSlider->getValue();

	int wsBlendStart_Z = centerCellPos.z - (Offsets::cellSize / 2) * mBrushSizeSlider->getValue();
	int wsBlendEnd_Z = centerCellPos.z + (Offsets::cellSize / 2) * mBrushSizeSlider->getValue();

	Ogre::Vector3 wsBlendStart = Ogre::Vector3(wsBlendStart_X, 0, wsBlendStart_Z);
	Ogre::Vector3 wsBlendEnd = Ogre::Vector3(wsBlendEnd_X, 0, wsBlendEnd_Z);

	Ogre::uint16 blendMapSize = terrain->getLayerBlendMapSize();
	Ogre::Vector3 tsBlendStart;
	Ogre::Vector3 tsBlendEnd;

	terrain->getTerrainPosition(wsBlendStart, &tsBlendStart);
	terrain->getTerrainPosition(wsBlendEnd, &tsBlendEnd);

	Ogre::uint16 startTerrain_X = tsBlendStart.x * terrainSize;
	Ogre::uint16 endTerrain_X = tsBlendEnd.x * terrainSize;

	Ogre::uint16 startTerrain_Y = tsBlendStart.y * terrainSize;
	Ogre::uint16 endTerrain_Y = tsBlendEnd.y * terrainSize;

	Ogre::uint16 start_X = tsBlendStart.x * blendMapSize;
	Ogre::uint16 end_X = tsBlendEnd.x * blendMapSize;

	Ogre::uint16 start_Y = (1 - tsBlendStart.y) * blendMapSize;
	Ogre::uint16 end_Y = (1 - tsBlendEnd.y) * blendMapSize;

	int index = 0;

	for (Ogre::uint16 y = 0; y < blendMapSize; ++y)
	{
		for (Ogre::uint16 x = 0; x < blendMapSize; ++x)
		{

			if ((x >= start_X && x <= end_X) && (y >= start_Y && y <= end_Y))
			{
				if (mEditTextureSM->getSelectedItem() == TerrainType::Grass)
				{
					pBlend0[index] = 0.1f;
					pBlend1[index] = 0.1f;
				}
				else if (mEditTextureSM->getSelectedItem() == TerrainType::Rock)
				{
					pBlend0[index] = 0.85f;
					pBlend1[index] = 0.1f;
				}
				else if (mEditTextureSM->getSelectedItem() == TerrainType::Sand)
				{
					pBlend0[index] = 0.1f;
					pBlend1[index] = 0.85f;
				}
				
			}
			index++;
		}
	}
	
	blendMap0->dirty();
	blendMap0->update();
	blendMap1->dirty();
	blendMap1->update();
}
//----------------------------------------------------------------

bool SimRunnable::mouseReleased(const OgreBites::MouseButtonEvent& evt)
{
	int relx = evt.x;
	int rely = evt.y;
	Ogre::String objectName = "";
	Ogre::Ray mouseRay = mTrayMgr->getCursorRay(mCam);

	Ogre::TerrainGroup::RayResult rayResult = mTerrainGroup->rayIntersects(mouseRay);
	if (rayResult.hit) 
	{
		Ogre::TerrainGroup::TerrainList terrainList;
		Ogre::Sphere sphere(rayResult.position, 50);
		mTerrainGroup->sphereIntersects(sphere, &terrainList);
		
		for (Ogre::TerrainGroup::TerrainList::iterator ti = terrainList.begin(); ti != terrainList.end(); ++ti)
		{
			Ogre::Vector2 gridCoordinates = updateCoords(rayResult.position);
			alterTerrain(*ti, rayResult.position, gridCoordinates);
			mTerrainGroup->update();
		}
		/*Ogre::AxisAlignedBox box()
		mTerrainGroup->boxIntersects*/
	}

	return 0;
}


void SimRunnable::setup(void) {
	OgreBites::ApplicationContext::setup();
	addInputListener(this);
	SDL_ShowCursor(SDL_DISABLE);

	Ogre::Root* root = getRoot();
	Ogre::ConfigDialog* dialog = OgreBites::getNativeConfigDialog();

	mScnMgr = root->createSceneManager();
	mScnMgr->addRenderQueueListener(getOverlaySystem());
	mTrayMgr = new OgreBites::TrayManager("InterfaceName", getRenderWindow(), this);
	mTrayMgr->showFrameStats(OgreBites::TrayLocation::TL_BOTTOMLEFT);

	mLoadBtn = mTrayMgr->createButton(OgreBites::TL_TOPLEFT, "New", "New", 150.0f);
	mLoadBtn = mTrayMgr->createButton(OgreBites::TL_TOPLEFT, "Load", "Load", 150.0f);
	mSaveBtn = mTrayMgr->createButton(OgreBites::TL_TOPLEFT, "Save", "Save", 150.0f);

	mMoveableCamera = mTrayMgr->createCheckBox(OgreBites::TL_TOPRIGHT, "MoveableCamera", "Moveable Camera", 180.0f);
	mElevationSlider = mTrayMgr->createThickSlider(OgreBites::TL_TOPRIGHT, "Elevation", "Elevation:", 180.0f, 80.0f, -1, 1, 3);
	mElevationSlider->setValue(0);
	mBrushSizeSlider = mTrayMgr->createThickSlider(OgreBites::TL_TOPRIGHT, "BrushSize", "Brush Size:", 180.0f, 80.0f, 1, 4, 4);
	mEditTextureSM = mTrayMgr->createThickSelectMenu(OgreBites::TL_TOPRIGHT, "EditColor", "Colors:", 180.0f, 3, { TerrainType::Grass, TerrainType::Rock, TerrainType::Sand });

	mCoordsBox = mTrayMgr->createTextBox(OgreBites::TL_BOTTOMRIGHT, "CoordsBox", "Coordinates", 200.0f, 125.0f);

	//mTrayMgr->createTextBox(OgreBites::TL_CENTER, "File)

	addInputListener(mTrayMgr);

	Ogre::RTShader::ShaderGenerator* shadergen = Ogre::RTShader::ShaderGenerator::getSingletonPtr();
	shadergen->addSceneManager(mScnMgr);

	camAnchor = mScnMgr->getRootSceneNode()->createChildSceneNode("camAnchor");
	camNode = camAnchor->createChildSceneNode("camNode");
	mCam = mScnMgr->createCamera("myCam");
	camAnchor->setPosition(Ogre::Vector3::ZERO + Ogre::Vector3(1683, 1900, 2116));
	camNode->lookAt(Ogre::Vector3(0, 0, 0), Ogre::Node::TransformSpace::TS_WORLD);

	mCam->setNearClipDistance(5);
	camNode->attachObject(mCam);
	Ogre::Viewport* vp = getRenderWindow()->addViewport(mCam);
	vp->setBackgroundColour(Ogre::ColourValue(0, 0, 0));
	mCam->setAspectRatio(Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight()));

	root->addFrameListener(this);

	mScnMgr->setAmbientLight(Ogre::ColourValue(1.0f, 1.0f, 1.0f));
	//mScnMgr->setShadowTechnique(Ogre::ShadowTechnique::SHADOWTYPE_STENCIL_MODULATIVE);
	Ogre::Light * spotLight = mScnMgr->createLight("SpotLight");
	spotLight->setDiffuseColour(0, 0, 1.0);
	spotLight->setSpecularColour(0, 0, 1.0);
	spotLight->setType(Ogre::Light::LT_SPOTLIGHT);
	Ogre::SceneNode * spotLightNode = mScnMgr->getRootSceneNode()->createChildSceneNode();
	spotLightNode->attachObject(spotLight);
	spotLightNode->setDirection(-1, -1, 0);
	spotLightNode->setPosition(Ogre::Vector3(200, 200, 0));
	spotLight->setSpotlightRange(Ogre::Degree(35), Ogre::Degree(50));
	
	/** Terrain loading **/
	mTerrainGlobals = new Ogre::TerrainGlobalOptions();


	mTerrainGroup = OGRE_NEW Ogre::TerrainGroup(
		mScnMgr,
		Ogre::Terrain::ALIGN_X_Z,
		513, Offsets::worldSize);
	mTerrainGroup->setFilenameConvention(Ogre::String("terrain"), Ogre::String("dat"));
	mTerrainGroup->setOrigin(Ogre::Vector3::ZERO);
	mTerrainGroup->setOrigin(Ogre::Vector3(1000, 0, 1000));

	configureTerrainDefaults(spotLight);

	/*for (long x = 0; x <= 0; ++x)
		for (long y = 0; y <= 0; ++y)*/
	defineTerrain(0, 0);

	// sync load since we want everything in place when we start
	mTerrainGroup->loadAllTerrains(true);

	if (mTerrainsImported)
	{
		Ogre::TerrainGroup::TerrainIterator ti = mTerrainGroup->getTerrainIterator();
		while (ti.hasMoreElements())
		{
			Ogre::Terrain* t = ti.getNext()->instance;
			initBlendMaps(t);
		}
	}

	mInfoLabel = mTrayMgr->createLabel(OgreBites::TL_TOP, "TerrainInfo", "", 350);

	mTerrainGroup->freeTemporaryResources();
	
	/** initialize world space (ws) terrain properties **/
	int width = Offsets::worldSize / Offsets::cellSize;
	int height = Offsets::worldSize / Offsets::cellSize;

	for (int y = 0; y < width; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			Cell cell;
			cell.x = x;
			cell.y = y;
			cell.unpassable = 0;
			worldSpaceCells.push_back(cell);
		}
	}
}
//----------------------------------------------------------------


void SimRunnable::frameRendered(const Ogre::FrameEvent& evt)
{
	/* Camera movement */
	if (mMoveableCamera->isChecked()) {
		Ogre::Vector3 transVector = Ogre::Vector3::ZERO;
		int mMoveScale = 250;
		int mHalfScale = 125;
		int x, y;
		SDL_GetMouseState(&x, &y);
		if (x > (getRenderWindow()->getWidth() - 20)) {
			transVector.x += mMoveScale;
			transVector.z -= mMoveScale;
		}
		else if (x < 20) {
			transVector.x -= mMoveScale;
			transVector.z += mMoveScale;
		}
		else if (y > (getRenderWindow()->getHeight() - 20)) {
			transVector.z += mMoveScale;
			transVector.x += mMoveScale;
		}
		else if (y < 20) {
			transVector.z -= mMoveScale;
			transVector.x -= mMoveScale;
		}
		mScnMgr->getSceneNode("camAnchor")->translate(transVector * evt.timeSinceLastFrame, Ogre::Node::TS_LOCAL);
	}
	
	/* Terrain loading */
	if (mTerrainGroup->isDerivedDataUpdateInProgress())
	{
		mTrayMgr->moveWidgetToTray(mInfoLabel, OgreBites::TL_TOP, 0);
		mInfoLabel->show();

		if (mTerrainsImported)
			mInfoLabel->setCaption("Building terrain...");
		else
			mInfoLabel->setCaption("Updating textures...");
	}
	else
	{
		mTrayMgr->removeWidgetFromTray(mInfoLabel);
		mInfoLabel->hide();

		if (mTerrainsImported)
		{
			mTerrainGroup->saveAllTerrains(true);
			mTerrainsImported = false;
		}
	}
}
//----------------------------------------------------------------


void getTerrainImage(bool flipX, bool flipY, Ogre::Image& img)
{
	img.load("white.bmp", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	//img.load("terrain.png", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

	if (flipX)
		img.flipAroundY();
	if (flipY)
		img.flipAroundX();

}
//----------------------------------------------------------------


void SimRunnable::defineTerrain(long x, long y)
{
	//mTerrainGroup->defineTerrain(x, y);
	Ogre::String filename = mTerrainGroup->generateFilename(x, y);
	//Ogre::String filename = "terrain_bogaloo.dat";
	if (Ogre::ResourceGroupManager::getSingleton().resourceExists(mTerrainGroup->getResourceGroup(), filename))
	{
		mTerrainGroup->defineTerrain(x, y);
	}
	else
	{
		Ogre::Image img;
		getTerrainImage(x % 2 != 0, y % 2 != 0, img);
		mTerrainGroup->defineTerrain(x, y, &img);
		mTerrainsImported = true;
	}
}
//----------------------------------------------------------------


void SimRunnable::initBlendMaps(Ogre::Terrain* terrain)
{
	float minHeight0 = 20;
	float fadeDist0 = 15;
	float minHeight1 = 70;
	float fadeDist1 = 15;

	Ogre::TerrainLayerBlendMap* blendMap0 = terrain->getLayerBlendMap(1);
	Ogre::TerrainLayerBlendMap* blendMap1 = terrain->getLayerBlendMap(2);
	
	float* pBlend0 = blendMap0->getBlendPointer();
	float* pBlend1 = blendMap1->getBlendPointer();

	for (Ogre::uint16 y = 0; y < terrain->getLayerBlendMapSize(); ++y)
	{
		for (Ogre::uint16 x = 0; x < terrain->getLayerBlendMapSize(); ++x)
		{
			Ogre::Real tx, ty;

			blendMap0->convertImageToTerrainSpace(x, y, &tx, &ty);
			*pBlend0++ = 0.1f;
			*pBlend1++ = 0.1f;
		}
	}
	
	blendMap0->dirty();
	blendMap1->dirty();
	blendMap0->update();
	blendMap1->update();
}
//----------------------------------------------------------------


void SimRunnable::configureTerrainDefaults(Ogre::Light* light)
{
	mTerrainGlobals->setMaxPixelError(8);
	mTerrainGlobals->setCompositeMapDistance(6000);

	mTerrainGlobals->setLightMapDirection(light->getDerivedDirection());
	mTerrainGlobals->setCompositeMapAmbient(mScnMgr->getAmbientLight());
	mTerrainGlobals->setCompositeMapDiffuse(light->getDiffuseColour());

	Ogre::Terrain::ImportData& importData = mTerrainGroup->getDefaultImportSettings();
	importData.terrainSize = 513;
	importData.worldSize = Offsets::worldSize;
	importData.inputScale = 200;
	importData.minBatchSize = 33;
	importData.maxBatchSize = 65;

	importData.layerList.resize(3);
	importData.layerList[0].worldSize = 500;
	importData.layerList[0].textureNames.push_back(
		"grass_green-01_diffusespecular.dds");
	importData.layerList[0].textureNames.push_back(
		"grass_green-01_normalheight.dds");
	importData.layerList[1].worldSize = 1000;
	importData.layerList[1].textureNames.push_back(
		"dirt_grayrocky_diffusespecular.dds");
	importData.layerList[1].textureNames.push_back(
		"dirt_grayrocky_normalheight.dds");
	importData.layerList[2].worldSize = 1000;
	importData.layerList[2].textureNames.push_back(
		"sand_1_diffusespecular.dds");
	importData.layerList[2].textureNames.push_back(
		"sand_1_normalheight.dds");
}
//----------------------------------------------------------------


#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif
#ifdef __cplusplus
extern "C" {
#endif
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT)
#else
	int main(int argc, char* argv[])
#endif
	{
		// Create application object
		SimRunnable app;
		try {
			app.initApp();
			app.getRoot()->startRendering();
			app.closeApp();
			return 0;
		}
		catch (Ogre::Exception & e) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
			MessageBox(NULL, e.getFullDescription().c_str(), "An exception has occurred!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
			std::cerr << "An exception has occurred: " <<
				e.getFullDescription().c_str() << std::endl;
#endif
		}
		return 0;
	}
#ifdef __cplusplus
}
#endif