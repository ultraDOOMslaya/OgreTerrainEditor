#include "SimRunnable.h"


namespace Offsets {
	/** Map builders **/
	extern const float worldSize = 2000;
	extern const float baseHeight = 200;
	
	extern const int cellSize = 80;
	extern const float heightOffset = 200;

	extern const float brushSize = 0.025;
}


namespace EditType {
	extern const Ogre::String Ground = "ground";
	extern const Ogre::String Tree = "tree";
	extern const Ogre::String Dodad = "dodad";
}

namespace TerrainType {
	extern const Ogre::String Grass = "grass";
	extern const Ogre::String Plains = "plains";
	extern const Ogre::String Tundra = "tundra";
}

namespace GrassTypeMesh {
	extern const Ogre::String GrassTuft03Mesh = "GrassTuft03Mesh";
	extern const Ogre::String GrassTuft05Mesh = "GrassTuft05Mesh";
}

namespace EditorQueryFlags {
	extern int Dodad = 1;
};

SimRunnable::SimRunnable()
	: mTerrainGroup(0),
	mTerrainGlobals(0),
	mInfoLabel(0),
	mLastDodadId(0),
	mLastTreeId(0)
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

bool SimRunnable::keyPressed(const OgreBites::KeyboardEvent& evt)
{	
	Ogre::Vector3 camPosition = mScnMgr->getSceneNode("camAnchor")->getPosition();
	if (evt.keysym.sym == SDLK_F1) {
		Ogre::Vector3 newCamPos = Ogre::Vector3(0, 50, 0);
		mScnMgr->getSceneNode("camAnchor")->translate(newCamPos, Ogre::Node::TS_LOCAL);
	}

	if (evt.keysym.sym == SDLK_F2) {
		Ogre::Vector3 newCamPos = Ogre::Vector3(0, -50, 0);
		mScnMgr->getSceneNode("camAnchor")->translate(newCamPos, Ogre::Node::TS_LOCAL);
	}

	if (evt.keysym.sym == SDLK_x) {
		mGUI->toggleGameObjectDelete();
	}

	return true;
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

		Ogre::Image combined;

		combined.loadTwoImagesAsRGBA("grass_flowmap_02.jpg", "grass_flowmap_02_SPEC.bmp",
			Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::PF_BYTE_RGBA);
		combined.save("grass_flowmap_02_diffusespecular.png");

		combined.loadTwoImagesAsRGBA("grass_flowmap_02_NORM.tga", "grass_flowmap_02_DISP.bmp",
			Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::PF_BYTE_RGBA);
		combined.save("grass_flowmap_02_normalheight.png");

		/*Ogre::String filePath = "terrain_bogaloo.dat";
		mTerrainGroup->removeAllTerrains();
		mTerrainGroup->loadGroupDefinition(filePath);
		mTerrainGroup->loadAllTerrains(true);*/
		//mTerrainGroup->loadAllTerrains(true);
	}
	
}
//----------------------------------------------------------------

void SimRunnable::itemSelected(OgreBites::SelectMenu* menu)
{
	mGUI->itemSelectedEvent(menu);
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
	long startx = terrain_x - ((Offsets::cellSize / 8) * mGUI->mBrushSizeSlider->getValue());
	long starty = terrain_y - ((Offsets::cellSize / 8) * mGUI->mBrushSizeSlider->getValue());
	long endx = terrain_x + ((Offsets::cellSize / 8) * mGUI->mBrushSizeSlider->getValue());
	long endy = terrain_y + ((Offsets::cellSize / 8) * mGUI->mBrushSizeSlider->getValue());
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
	
	float brushSize = mGUI->mBrushSizeSlider->getValue() * Offsets::brushSize;

	int wsBlendStart_X = centerCellPos.x - (Offsets::cellSize / 2) * mGUI->mBrushSizeSlider->getValue();
	int wsBlendEnd_X = centerCellPos.x + (Offsets::cellSize / 2) * mGUI->mBrushSizeSlider->getValue();

	int wsBlendStart_Z = centerCellPos.z - (Offsets::cellSize / 2) * mGUI->mBrushSizeSlider->getValue();
	int wsBlendEnd_Z = centerCellPos.z + (Offsets::cellSize / 2) * mGUI->mBrushSizeSlider->getValue();

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
				pBlend0[index] = mGUI->mTexture01Depth->getValue();
				pBlend1[index] = mGUI->mTexture02Depth->getValue();
				
			}
			index++;
		}
	}
	
	blendMap0->dirty();
	blendMap0->update();
	blendMap1->dirty();
	blendMap1->update();
	mTerrainGroup->update();
}
//----------------------------------------------------------------

//TODO: rename to gameobject
void SimRunnable::createTree(Ogre::Vector3 centerPosition) {
	Ogre::Entity* treeEntity = mScnMgr->createEntity(mGUI->mSelectGameObject->getSelectedItem());

	Ogre::String treeName = "Tree_" + Ogre::StringConverter::toString(mLastTreeId++);

	Ogre::SceneNode* treeNode = mScnMgr->getRootSceneNode()->createChildSceneNode(treeName, Ogre::Vector3(centerPosition.x, 190, centerPosition.z));
	treeNode->setScale(40, 40, 40);
	treeNode->attachObject(treeEntity);
	treeEntity->setCastShadows(true);


}


void SimRunnable::alterDodad(Ogre::Terrain* terrain, Ogre::Vector3 centerPosition, Ogre::Vector2 gridCoordinates) {
	Ogre::Vector3 centerCellPos;
	getCellCenter(centerPosition, &centerCellPos);

	createDodadSG(Ogre::Vector3(centerPosition.x, 0, centerPosition.z) , mLastDodadId++);
}
//----------------------------------------------------------------


bool SimRunnable::mouseReleased(const OgreBites::MouseButtonEvent& evt)
{
	if (mTrayMgr->mouseReleased(evt)) return true;

	int relx = evt.x;
	int rely = evt.y;
	Ogre::String objectName = "";
	Ogre::Ray mouseRay = mTrayMgr->getCursorRay(mCam);

	/** Terrain Queries **/
	Ogre::TerrainGroup::RayResult rayResult = mTerrainGroup->rayIntersects(mouseRay);
	if (rayResult.hit) 
	{
		Ogre::TerrainGroup::TerrainList terrainList;
		Ogre::Sphere sphere(rayResult.position, 50);
		mTerrainGroup->sphereIntersects(sphere, &terrainList);
		
		for (Ogre::TerrainGroup::TerrainList::iterator ti = terrainList.begin(); ti != terrainList.end(); ++ti)
		{
			Ogre::Vector2 gridCoordinates = updateCoords(rayResult.position);

			
			if (mGUI->currentAlteration() == EditType::Ground) {
				alterTerrain(*ti, rayResult.position, gridCoordinates);
			}
			else if (mGUI->currentAlteration() == EditType::Tree) {
				if (!mGUI->deleteGameObject()) {
					createTree(rayResult.position);
				}
			}
			else if (mGUI->currentAlteration() == EditType::Dodad) {
				if (!mGUI->deleteGameObject()) {
					alterDodad(*ti, rayResult.position, gridCoordinates);
				}
			}
		}
	}

	/** Dodad Queries **/
	mDodadRayScnQuery->setRay(mouseRay);
	Ogre::RaySceneQueryResult& result = mDodadRayScnQuery->execute();
	Ogre::RaySceneQueryResult::iterator it = result.begin();
	int dodadRayQuerySize = result.size();

	if (dodadRayQuerySize > 0 && mGUI->deleteGameObject()) {
		mScnMgr->destroySceneNode(it->movable->getParentNode()->getName());
	}

	return 0;
}
//----------------------------------------------------------------


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

	/** Camera **/
	mMoveableCamera = mTrayMgr->createCheckBox(OgreBites::TL_TOPRIGHT, "MoveableCamera", "Moveable Camera", 180.0f);

	/** Top Right UI **/
	mElevationSlider = mTrayMgr->createThickSlider(OgreBites::TL_TOPRIGHT, "Elevation", "Elevation:", 180.0f, 80.0f, -1, 1, 3);
	mElevationSlider->setValue(0);
	
	//TODO move all the widgets to this
	mGUI = new GUI(mTrayMgr);

	mCoordsBox = mTrayMgr->createTextBox(OgreBites::TL_BOTTOMRIGHT, "CoordsBox", "Coordinates", 200.0f, 100.0f);

	//mTrayMgr->createTextBox(OgreBites::TL_CENTER, "File)

	addInputListener(mTrayMgr);

	Ogre::RTShader::ShaderGenerator* shadergen = Ogre::RTShader::ShaderGenerator::getSingletonPtr();
	shadergen->addSceneManager(mScnMgr);

	camAnchor = mScnMgr->getRootSceneNode()->createChildSceneNode("camAnchor");
	camNode = camAnchor->createChildSceneNode("camNode");
	mCam = mScnMgr->createCamera("myCam");
	camAnchor->setPosition(Ogre::Vector3::ZERO + Ogre::Vector3(940, 1050, 1158));
	//camAnchor->setPosition(Ogre::Vector3::ZERO + Ogre::Vector3(1683, 1900, 2116));
	camNode->lookAt(Ogre::Vector3(0, 0, 0), Ogre::Node::TransformSpace::TS_WORLD);

	mCam->setNearClipDistance(5);
	camNode->attachObject(mCam);
	Ogre::Viewport* vp = getRenderWindow()->addViewport(mCam);
	vp->setBackgroundColour(Ogre::ColourValue(0, 0, 0));
	mCam->setAspectRatio(Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight()));

	root->addFrameListener(this);

	mScnMgr->setAmbientLight(Ogre::ColourValue(0.85f, 0.85f, 0.85f));
	//mScnMgr->setAmbientLight(Ogre::ColourValue(0.5f, 0.5f, 0.5f));
	//mScnMgr->setShadowTechnique(Ogre::ShadowTechnique::SHADOWTYPE_STENCIL_MODULATIVE);
	Ogre::Light* spotLight = mScnMgr->createLight("SpotLight");
	spotLight->setType(Ogre::Light::LT_DIRECTIONAL);
	spotLight->setDirection(Ogre::Vector3(1.0f, -1.0f, -1.0f).normalisedCopy());
	spotLight->setDiffuseColour(1.0f, 1.0f, 1.0f);
	spotLight->setSpecularColour(0.25f, 0.25f, 0.25f);

	/*Ogre::Light * spotLight = mScnMgr->createLight("SpotLight");
	spotLight->setDiffuseColour(0, 0, 1.0);
	spotLight->setSpecularColour(0, 0, 1.0);
	spotLight->setType(Ogre::Light::LT_SPOTLIGHT);
	Ogre::SceneNode * spotLightNode = mScnMgr->getRootSceneNode()->createChildSceneNode();
	spotLightNode->attachObject(spotLight);
	spotLightNode->setDirection(-1, -1, 0);
	spotLightNode->setPosition(Ogre::Vector3(200, 200, 0));
	spotLight->setSpotlightRange(Ogre::Degree(35), Ogre::Degree(50));*/
	
	/** Terrain loading **/
	mTerrainGlobals = new Ogre::TerrainGlobalOptions();


	mTerrainGroup = OGRE_NEW Ogre::TerrainGroup(
		mScnMgr,
		Ogre::Terrain::ALIGN_X_Z,
		513, Offsets::worldSize);
	mTerrainGroup->setFilenameConvention(Ogre::String("terrainNewEdit"), Ogre::String("dat"));
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

	//Ogre::Entity* treeEnt = mScnMgr->createEntity("BirchTree.001.mesh");
	Ogre::Entity* treeEnt = mScnMgr->createEntity("leaves.484.mesh");
	Ogre::SceneNode* treeNode = mScnMgr->getRootSceneNode()->createChildSceneNode("BirchTree", Ogre::Vector3(500, 190, 500));
	treeNode->setScale(40, 40, 40);
	treeNode->attachObject(treeEnt);
	treeEnt->setCastShadows(true);

	initializeDodads();

	mDodadRayScnQuery = mScnMgr->createRayQuery(Ogre::Ray());
	mDodadRayScnQuery->setQueryMask(EditorQueryFlags::Dodad);
	mDodadRayScnQuery->setSortByDistance(true);
	/*Ogre::Entity* grass = mScnMgr->createEntity("GrassBladesMesh");
	Ogre::StaticGeometry* sg = mScnMgr->createStaticGeometry("GrassArea");

	const int size = Offsets::cellSize;
	const int amount = 1;

	sg->setRegionDimensions(Ogre::Vector3(size, size, size));
	sg->setOrigin(Ogre::Vector3(500, 200, 500));

	Ogre::Vector3 dodadPosition = Ogre::Vector3::ZERO;
	Ogre::Vector3 scale(1, Ogre::Math::RangeRandom(0.9, 1.1), 1);
	Ogre::Quaternion orientation;
	orientation.FromAngleAxis(
		Ogre::Degree(Ogre::Math::RangeRandom(0, 359)),
		Ogre::Vector3::UNIT_Y);

	sg->addEntity(grass, dodadPosition, orientation, scale);*/

	/*const int size = 375;
	const int amount = 1;

	sg->setRegionDimensions(Ogre::Vector3(size, size, size));
	sg->setOrigin(Ogre::Vector3(-size / 2, 0, -size / 2));

	for (int x = -size / 2; x < size / 2; x += (size / amount))
	{
		for (int z = -size / 2; z < size / 2; z += (size / amount))
		{
			Ogre::Real r = size / (float)amount / 2;
			Ogre::Vector3 pos(
				x + Ogre::Math::RangeRandom(-r, r),
				0,
				z + Ogre::Math::RangeRandom(-r, r));

			Ogre::Vector3 scale(1, Ogre::Math::RangeRandom(0.9, 1.1), 1);

			Ogre::Quaternion orientation;
			orientation.FromAngleAxis(
				Ogre::Degree(Ogre::Math::RangeRandom(0, 359)),
				Ogre::Vector3::UNIT_Y);

			sg->addEntity(grass, pos, orientation, scale);
		}
	}*/

	//sg->build();
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

void SimRunnable::initializeDodads()
{
	const float width = 60;
	const float height = 280;
	Ogre::Vector3 vec(width / 2, 0, 0);
	Ogre::ManualObject obj("GrassTuft03Object");
	Ogre::ManualObject obj1("GrassTuft05Object");

	Ogre::Quaternion quat;
	quat.FromAngleAxis(Ogre::Degree(60), Ogre::Vector3::UNIT_Y);

	//GrassWave01
	//GrassBladesClumps1
	//GrassTuft03

	obj.begin("Examples/GrassTuft03", Ogre::RenderOperation::OT_TRIANGLE_LIST);

	obj.position(-vec.x, height, -vec.z);
	obj.textureCoord(0, 0);
	obj.position(vec.x, height, vec.z);
	obj.textureCoord(1, 0);
	obj.position(-vec.x, 200, -vec.z);
	obj.textureCoord(0, 1);
	obj.position(vec.x, 200, vec.z);
	obj.textureCoord(1, 1);

	obj.triangle(0, 3, 1);
	obj.triangle(0, 2, 3);

	vec = quat * vec;

	obj.end();
	obj.convertToMesh(GrassTypeMesh::GrassTuft03Mesh);


	obj1.begin("Examples/GrassTuft05", Ogre::RenderOperation::OT_TRIANGLE_LIST);

	obj1.position(-vec.x, height, -vec.z);
	obj1.textureCoord(0, 0);
	obj1.position(vec.x, height, vec.z);
	obj1.textureCoord(1, 0);
	obj1.position(-vec.x, 200, -vec.z);
	obj1.textureCoord(0, 1);
	obj1.position(vec.x, 200, vec.z);
	obj1.textureCoord(1, 1);

	obj1.triangle(0, 3, 1);
	obj1.triangle(0, 2, 3);

	vec = quat * vec;

	obj1.end();
	obj1.convertToMesh(GrassTypeMesh::GrassTuft05Mesh);

}
//----------------------------------------------------------------

void SimRunnable::createDodadSG(Ogre::Vector3 position, int id) {
	// Make a selection manager that controls whech dodad type we're on
	//TODO pass an object instead of 4 calls
	Ogre::String dodadMeshName = mGUI->mSelectGrassDodad->getSelectedItem();
	Ogre::Real width = mGUI->mDodadWidth->getValue();
	Ogre::Real height = mGUI->mDodadHeight->getValue();
	Ogre::Degree rotation =  Ogre::Degree(mGUI->mDodadRotation->getValue());


	Ogre::Entity* grass = mScnMgr->createEntity(dodadMeshName);
	grass->setQueryFlags(EditorQueryFlags::Dodad);
	Ogre::String sgName = "GrassArea_" + Ogre::StringConverter::toString(id);
	
	Ogre::Vector3 dodadPosition = position;
	Ogre::Vector3 scale(width, height, width);
	Ogre::Quaternion orientation;
	
	Ogre::SceneNode* dodadNode = mScnMgr->getRootSceneNode()->createChildSceneNode(sgName, position);
	dodadNode->setScale(scale);
	dodadNode->attachObject(grass);

	orientation.FromAngleAxis(
		rotation,
		Ogre::Vector3::UNIT_Y
	);

	dodadNode->setOrientation(orientation);
	
	//Ogre::StaticGeometry* sg = mScnMgr->createStaticGeometry(sgName);

	//const int size = Offsets::cellSize;
	//const int amount = 1;

	//sg->setRegionDimensions(Ogre::Vector3(10, 10, 10));
	//sg->setOrigin(position);

	//
	//orientation.FromAngleAxis(
	//	Ogre::Degree(mDodadRotation->getValue()),
	//	Ogre::Vector3::UNIT_Y);
	///*orientation.FromAngleAxis(
	//	Ogre::Degree(Ogre::Math::RangeRandom(0, 359)),
	//	Ogre::Vector3::UNIT_Y);
	//orientation.FromAngleAxis(
	//	Ogre::Degree(45),
	//	Ogre::Vector3::UNIT_Y);*/

	//sg->addEntity(grass, dodadPosition, orientation, scale);
	//sg->build();
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
	importData.layerList[0].worldSize = 300;
	/*importData.layerList[0].textureNames.push_back(
		"grass_green-01_diffusespecular.dds");
	importData.layerList[0].textureNames.push_back(
		"grass_green-01_normalheight.dds");*/

	/*importData.layerList[0].textureNames.push_back("grass_0_diffusespecular.dds");
	importData.layerList[0].textureNames.push_back("grass_0_normalheight.dds");*/

	importData.layerList[0].textureNames.push_back("grass_seamless_diffusespecular.png");
	importData.layerList[0].textureNames.push_back("grass_seamless_diffusespecular.png");
	//importData.layerList[0].textureNames.push_back("grass_seamless_normalheight.png");

	importData.layerList[1].worldSize = 500;

	importData.layerList[1].textureNames.push_back(
		"grass_flowmap_02_diffusespecular.png");
	importData.layerList[1].textureNames.push_back(
		"grass_flowmap_02_normalheight.png");

	importData.layerList[2].worldSize = 200;
	/*importData.layerList[1].textureNames.push_back(
		"dirt_grayrocky_diffusespecular.dds");
	importData.layerList[1].textureNames.push_back(
		"dirt_grayrocky_normalheight.dds");*/

	importData.layerList[2].textureNames.push_back(
		"tiaga_seamless_0_diffusespecular.png");
	importData.layerList[2].textureNames.push_back(
		"tiaga_seamless_0_normalheight.png");

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