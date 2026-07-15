/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**  Copyright 2026 Stephan Vedder
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QDateTime>
#include <QSettings>
#include <QTimer>
#include <QVBoxLayout>
#include <QWheelEvent>

// wbview3d.cpp : the 3d view (Qt6 port of the MFC WbView3d).  W3D renders
// into an embedded SDL window (QSdlWindow) via DXVK, same pattern as W3DView.
// Labels, letterbox and the DrawObject feedback overlay are not ported yet.

#include "wbview3d.h"

#include "MainFrm.h"
#include "WHeightMapEdit.h"
#include "WorldBuilder.h"
#include "WorldBuilderDoc.h"
#include "WBHeightMap.h"

#include "Common/Debug.h"
#include "Common/FileSystem.h"		// for TEST_STRING
#include "Common/GlobalData.h"
#include "Common/MapObject.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "Common/ModuleFactory.h"
#include "Common/PlayerTemplate.h"
#include "Common/WellKnownKeys.h"
#include "GameClient/Water.h"
#include "GameLogic/SidesList.h"
#include "GameLogic/TerrainLogic.h"
#include "W3DDevice/GameClient/HeightMap.h"
#include "W3DDevice/GameClient/W3DAssetManager.h"
#include "W3DDevice/GameClient/W3DDynamicLight.h"
#include "W3DDevice/GameClient/W3DScene.h"
#include "W3DDevice/GameClient/W3DShaderManager.h"
#include "W3DDevice/GameClient/W3DShadow.h"
#include "W3DDevice/GameClient/Module/W3DModelDraw.h"
#include "W3DDevice/GameClient/W3DTreeBuffer.h"

#include "ww3d.h"
#include "wwmath.h"
#include "camera.h"
#include "light.h"
#include "layer.h"
#include "intersec.h"
#include "part_ldr.h"
#include "agg_def.h"
#include "predlod.h"
#include "coltest.h"
#include "matpass.h"
#include "lineseg.h"

#include "qsdlwindow.h"


#include <SDL3/SDL.h>

// ----------------------------------------------------------------------------
// Constants:
// ----------------------------------------------------------------------------
#define UPDATE_TIME					100  /* 10 frames a second */

static UnsignedInt wbTickCount()
{
	return (UnsignedInt)QDateTime::currentMSecsSinceEpoch();
}

// The W3DShadowManager accesses TheTacticalView; the placeholder stub lives
// in placeholderview.cpp because GameClient/View.h cannot be included in a TU
// that also sees Qt's X11 "Display" typedef (qguiapplication.h).
extern void WbInstallPlaceholderTacticalView();
extern void WbPlaceholderViewSetSize(Int width, Int height);

// ----------------------------------------------------------------------------
// Customized scene for worldbuilder preview window.
// ----------------------------------------------------------------------------

class SkeletonSceneClass : public RTS3DScene
{
public:
	SkeletonSceneClass(void) : m_testPass(NULL) { }
	~SkeletonSceneClass(void) { REF_PTR_RELEASE(m_testPass); }

	void					Set_Material_Pass(MaterialPassClass * pass)	{ REF_PTR_SET(m_testPass, pass); }
	virtual void Remove_Render_Object(RenderObjClass * obj);

protected:
	MaterialPassClass *m_testPass;
};

void SkeletonSceneClass::Remove_Render_Object(RenderObjClass * obj)
{
	if (RenderList.Contains(obj)) {
		RenderObjClass *refPtr = NULL;
		REF_PTR_SET(refPtr, obj); // ref it, as when it gets removed from the scene, may get deleted otherwise.
		RTS3DScene::Remove_Render_Object(obj);
		REF_PTR_RELEASE(refPtr);
	}
}

// ----------------------------------------------------------------------------
// WbView3d
// ----------------------------------------------------------------------------

WbView3d::WbView3d(QWidget *parent) :
	WbView(parent),
	m_assetManager(NULL),
	m_scene(NULL),
	m_overlayScene(NULL),
	m_transparentObjectsScene(NULL),
	m_baseBuildScene(NULL),
	m_camera(NULL),
	m_heightMapRenderObj(NULL),
	m_layer(NULL),
	m_buildLayer(NULL),
	m_intersector(NULL),
	m_sdlWindow(NULL),
	m_pSDLWindow(NULL),
	m_renderTimer(NULL),
	m_mouseWheelOffset(0),
	m_cameraAngle(0.0),
	m_FXPitch(1.0f),
	m_actualHeightAboveGround(0.0f),
	m_groundLevel(10),
	m_curTrackingZ(10),
	m_updateCount(0),
	m_partialMapSize(129),
	m_needToLoadRoads(false),
	m_showEntireMap(true),
	m_showWireframe(false),
	m_projection(false),
	m_showShadows(true),
	m_ww3dInited(false),
	m_time(0),
	m_buildRedMultiplier(0)
{
	WbInstallPlaceholderTacticalView();
	m_cameraOffset.x = m_cameraOffset.y = m_cameraOffset.z = 1;

	QSettings settings;
	m_actualWinSize.x = THREE_D_VIEW_WIDTH;
	m_actualWinSize.y = THREE_D_VIEW_HEIGHT;

	for (Int i=0; i<MAX_GLOBAL_LIGHTS; i++)
	{
		m_globalLight[i] = NEW_REF( LightClass, (LightClass::DIRECTIONAL) );
	}

	m_showWireframe = settings.value(MAIN_FRAME_SECTION "/ShowWireframe", false).toBool();
	m_showEntireMap = settings.value(MAIN_FRAME_SECTION "/ShowEntireMap", true).toBool();
	m_showShadows = settings.value(MAIN_FRAME_SECTION "/ShowShadows", true).toBool();
	TheWritableGlobalData->m_useShadowDecals = m_showShadows;
	TheWritableGlobalData->m_useShadowVolumes = m_showShadows;
	m_partialMapSize = settings.value("GameOptions/partialMapSize", 97).toInt();

	// Embed the SDL/DXVK render window.
	m_sdlWindow = new QSdlWindow();
	QWidget *sdlWidget = QWidget::createWindowContainer(m_sdlWindow, this);
	// Mouse events must reach this widget (the tools), not the native window.
	sdlWidget->setAttribute(Qt::WA_TransparentForMouseEvents);
	m_sdlWindow->setFlag(Qt::WindowTransparentForInput);
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(sdlWidget);
	setLayout(layout);

	m_sdlWindow->Initialize();
	m_pSDLWindow = m_sdlWindow->GetSDLWindow();

	// The "game loop" timer continuously re-renders the scene.
	m_renderTimer = new QTimer(this);
	connect(m_renderTimer, &QTimer::timeout, this, [this]() { redraw(); });
}

// ----------------------------------------------------------------------------
WbView3d::~WbView3d()
{
	shutdownWW3D();
}

// ----------------------------------------------------------------------------
void WbView3d::shutdownWW3D(void)
{
	if (m_renderTimer) {
		m_renderTimer->stop();
	}
	if (m_intersector) {
		delete m_intersector;
		m_intersector = NULL;
	}
	if (m_layer) {
		delete m_layer;
		m_layer = NULL;
	}
	if (m_buildLayer) {
		delete m_buildLayer;
		m_buildLayer = NULL;
	}
	if (m_ww3dInited) {
		m_lightList.Reset_List();

		if (m_assetManager) {
			PredictiveLODOptimizerClass::Free();
			m_assetManager->Free_Assets();
			delete m_assetManager;
			m_assetManager = NULL;
		}

		if (TheW3DShadowManager)
		{
			TheW3DShadowManager->removeAllShadows();
			delete TheW3DShadowManager;
			TheW3DShadowManager=NULL;
		}
		REF_PTR_RELEASE(m_transparentObjectsScene);
		REF_PTR_RELEASE(m_overlayScene);
		REF_PTR_RELEASE(m_baseBuildScene);
		REF_PTR_RELEASE(m_scene);
		REF_PTR_RELEASE(m_camera);
		REF_PTR_RELEASE(m_heightMapRenderObj);
		for (Int i=0; i<MAX_GLOBAL_LIGHTS; i++)
			REF_PTR_RELEASE(m_globalLight[i]);
		WW3D::Shutdown();
		WWMath::Shutdown();
	}
	m_ww3dInited = false;
}

// ----------------------------------------------------------------------------
void WbView3d::reset3dEngineDisplaySize(Int width, Int height)
{
	if (m_actualWinSize.x == width &&
		m_actualWinSize.y == height) {
		return;
	}
	WbPlaceholderViewSetSize(width, height);
	m_actualWinSize.x = width;
	m_actualWinSize.y = height;
	if (m_ww3dInited) {
		WW3D::Set_Device_Resolution(m_actualWinSize.x, m_actualWinSize.y, -1, -1, true);
	}
}

// ----------------------------------------------------------------------------
void WbView3d::initAssets()
{
	m_assetManager = new W3DAssetManager;
	m_assetManager->Register_Prototype_Loader(&_ParticleEmitterLoader );
	m_assetManager->Register_Prototype_Loader(&_AggregateLoader);
	m_assetManager->Set_WW3D_Load_On_Demand(true);
}

// ----------------------------------------------------------------------------
void WbView3d::initWW3D()
{
	// only want to do once per instance, but do lazily.
	if (!m_ww3dInited) {
		m_ww3dInited = true;

		WWMath::Init();

		WW3D::Set_Prelit_Mode(WW3D::PRELIT_MODE_VERTEX);

		initAssets();
		WW3D::Init(m_pSDLWindow);
		WW3D::Set_Prelit_Mode( WW3D::PRELIT_MODE_LIGHTMAP_MULTI_PASS );
		WW3D::Set_Collision_Box_Display_Mask(0x00);	///<set to 0xff to make collision boxes visible

		m_actualWinSize.x = width() > 0 ? width() : THREE_D_VIEW_WIDTH;
		m_actualWinSize.y = height() > 0 ? height() : THREE_D_VIEW_HEIGHT;
		WbPlaceholderViewSetSize(m_actualWinSize.x, m_actualWinSize.y);
		if (WW3D::Set_Render_Device(0, m_actualWinSize.x, m_actualWinSize.y, 32, true, true) != WW3D_ERROR_OK)
		{
			// Getting the device at the default bit depth (32) didn't work, so try
			// getting a 16 bit display.  (Voodoo 1-3 only supported 16 bit.) jba.
			if (WW3D::Set_Render_Device(0, m_actualWinSize.x, m_actualWinSize.y, 16, true, true) != WW3D_ERROR_OK)
			{
				DEBUG_CRASH(("Couldn't set render device."));
			}
		}

		WW3D::Enable_Static_Sort_Lists(true);
		WW3D::Set_Thumbnail_Enabled(false);
		WW3D::Set_Screen_UV_Bias( TRUE );  ///< this makes text look good :)

		W3DShaderManager::init();
		init3dScene();
		m_layer = new LayerClass( m_scene, m_camera );
		m_buildLayer = new LayerClass( m_baseBuildScene, m_camera );
		m_intersector = new IntersectionClass();
		/// @todo create the DrawObject feedback overlay once DrawObject is ported.

		TheWritableGlobalData->m_useShadowVolumes = true;
		TheWritableGlobalData->m_useShadowDecals = true;
		TheWritableGlobalData->m_enableBehindBuildingMarkers = false;	//this is only for the game.
		if (TheW3DShadowManager==NULL)
		{	TheW3DShadowManager = new W3DShadowManager;
 			TheW3DShadowManager->init();
		}
		updateLights();
		resetRenderObjects();

		m_renderTimer->start(UPDATE_TIME);
	}
}

// ----------------------------------------------------------------------------
#define TERRAIN_SAMPLE_SIZE 40.0f
static Real getHeightAroundPos(WBHeightMap *heightMap, Real x, Real y)
{
	Real terrainHeight = heightMap->getHeightMapHeight(x, y, NULL);

	// find best approximation of max terrain height we can see
	Real terrainHeightMax = terrainHeight;
	terrainHeightMax = max(terrainHeightMax, heightMap->getHeightMapHeight(x+TERRAIN_SAMPLE_SIZE, y-TERRAIN_SAMPLE_SIZE, NULL));
	terrainHeightMax = max(terrainHeightMax, heightMap->getHeightMapHeight(x-TERRAIN_SAMPLE_SIZE, y-TERRAIN_SAMPLE_SIZE, NULL));
	terrainHeightMax = max(terrainHeightMax, heightMap->getHeightMapHeight(x+TERRAIN_SAMPLE_SIZE, y+TERRAIN_SAMPLE_SIZE, NULL));
	terrainHeightMax = max(terrainHeightMax, heightMap->getHeightMapHeight(x-TERRAIN_SAMPLE_SIZE, y+TERRAIN_SAMPLE_SIZE, NULL));

	return terrainHeightMax;
}

// ----------------------------------------------------------------------------
void WbView3d::setupCamera()
{
	Matrix3D camtransform(1);
	float zOffset = - m_mouseWheelOffset / 1200.0f;
	Real zoom = 1.0f;
	if (zOffset != 0) {
		Real zPos = (m_cameraOffset.length()-m_groundLevel)/m_cameraOffset.length();
		Real zAbs = zOffset + zPos;
		if (zAbs<0) zAbs = -zAbs;
		if (zAbs<0.01) zAbs = 0.01f;
		if (zOffset > 0) {
			zOffset *= zAbs;
		}	else if (zOffset < -0.3f) {
			zOffset = -0.15f + zOffset/2.0f;
		}
		if (zOffset < -0.6f) {
			zOffset = -0.3f + zOffset/2.0f;
		}
		zoom = zAbs;
	}

	Vector3 sourcePos, targetPos;

	Real angle = m_cameraAngle;
	Real pitch = 0;
	Coord3D pos;
	pos.x = m_centerPt.X* MAP_XY_FACTOR;
	pos.y = m_centerPt.Y* MAP_XY_FACTOR;
	pos.z = m_centerPt.Z* MAP_XY_FACTOR;

	Real groundLevel = m_heightMapRenderObj?getHeightAroundPos(m_heightMapRenderObj, pos.x, pos.y) : 0;

	// set position of camera itself
	sourcePos.X = m_cameraOffset.x * zoom;
	sourcePos.Y = m_cameraOffset.y * zoom;
	sourcePos.Z = m_cameraOffset.z * zoom;

	// camera looking at origin
	targetPos.X = 0;
	targetPos.Y = 0;
	targetPos.Z = 0;

	Real factor = 1.0 - (groundLevel/sourcePos.Z );

	// construct a matrix to rotate around the up vector by the given angle
	Matrix3D angleTransform( Vector3( 0.0f, 0.0f, 1.0f ), angle );

	// construct a matrix to rotate around the horizontal vector by the given angle
	Matrix3D pitchTransform( Vector3( 1.0f, 0.0f, 0.0f ), pitch );

	// rotate camera position (pitch, then angle)
#ifdef ALLOW_TEMPORARIES
	sourcePos = pitchTransform * sourcePos;
	sourcePos = angleTransform * sourcePos;
#else
	pitchTransform.mulVector3(sourcePos);
	angleTransform.mulVector3(sourcePos);
#endif
	sourcePos *= factor;

	// translate to current XY position
	sourcePos.X += pos.x;
	sourcePos.Y += pos.y;
	sourcePos.Z += pos.z+groundLevel;

	targetPos.X += pos.x;
	targetPos.Y += pos.y;
	targetPos.Z += pos.z+groundLevel;

	// do fxPitch adjustment
	Real height = sourcePos.Z - targetPos.Z;
	height *= m_FXPitch;
	targetPos.Z = sourcePos.Z - height;

	// Just for kicks, lets see how high we are above the ground
	m_actualHeightAboveGround = m_cameraOffset.z * zoom - groundLevel;
	m_cameraSource = sourcePos;
	m_cameraTarget = targetPos;

	// build new camera transform
	camtransform.Make_Identity();
	if (factor < 0) { //WST 11/11/02. Fix camera flipping over when near the ground too early
		targetPos = sourcePos + (sourcePos-targetPos);
	}
	camtransform.Look_At( sourcePos, targetPos, 0 );

	targetPos.Z = 0;
	Real lookDistance = (targetPos-sourcePos).Length();
	Real nearZ, farZ;
	if (lookDistance < 300) lookDistance = 300;
	m_camera->Get_Clip_Planes(nearZ, farZ);
	m_camera->Set_Clip_Planes(lookDistance/200, lookDistance*3);

	if (m_heightMapRenderObj) {
		if (m_projection) {
			camtransform.Make_Identity();
			camtransform.Set_Translation(Vector3(targetPos.X, targetPos.Y, lookDistance));
			m_heightMapRenderObj->setFlattenHeights(true);
		} else {
			m_heightMapRenderObj->setFlattenHeights(false);
		}
	}
	m_camera->Set_Transform( camtransform );
	if (m_heightMapRenderObj) {
		m_heightMapRenderObj->setDrawEntireMap(m_showEntireMap);
	}
}

// ----------------------------------------------------------------------------
void WbView3d::init3dScene()
{
	// build scene
	REF_PTR_RELEASE(m_overlayScene);
	REF_PTR_RELEASE(m_transparentObjectsScene);
	REF_PTR_RELEASE(m_baseBuildScene);
	REF_PTR_RELEASE(m_scene);
	REF_PTR_RELEASE(m_camera);
	REF_PTR_RELEASE(m_heightMapRenderObj);

	m_scene = NEW_REF(SkeletonSceneClass,());
	m_overlayScene = NEW_REF(SkeletonSceneClass,());
	m_baseBuildScene = NEW_REF(SkeletonSceneClass,());
	m_transparentObjectsScene = NEW_REF(SkeletonSceneClass,());
	m_scene->Set_Ambient_Light(Vector3(0.5f,0.5f,0.5f));
	m_overlayScene->Set_Ambient_Light(Vector3(0.5f,0.5f,0.5f));
	m_baseBuildScene->Set_Ambient_Light(Vector3(0.5f,0.5f,0.5f));

	// Scene needs camera to be rendered with ----------------------------------
	m_camera = NEW_REF(CameraClass,());
}

// ----------------------------------------------------------------------------
void WbView3d::resetRenderObjects()
{
	if (!m_scene) return;
	if (TheW3DShadowManager) {
		TheW3DShadowManager->removeAllShadows();
	}

	SceneIterator *sceneIter = m_scene->Create_Iterator();
	sceneIter->First();
	while(!sceneIter->Is_Done()) {
		RenderObjClass * robj = sceneIter->Current_Item();
		robj->Add_Ref();
		m_scene->Remove_Render_Object(robj);
		robj->Release_Ref();
		sceneIter->Next();
	}
	m_scene->Destroy_Iterator(sceneIter);
	sceneIter = m_baseBuildScene->Create_Iterator();
	sceneIter->First();
	while(!sceneIter->Is_Done()) {
		RenderObjClass * robj = sceneIter->Current_Item();
		robj->Add_Ref();
		m_baseBuildScene->Remove_Render_Object(robj);
		robj->Release_Ref();
		sceneIter->Next();
	}
	m_baseBuildScene->Destroy_Iterator(sceneIter);
	MapObject *pMapObj = MapObject::getFirstMapObject();
	// Erase references to render objs that have been removed.
	while (pMapObj)
	{
		pMapObj->setRenderObj(NULL);
		pMapObj = pMapObj->getNext();
	}

	Int i;
	for (i=0; i<TheSidesList->getNumSides(); i++) {
		SidesInfo *pSide = TheSidesList->getSideInfo(i);
		BuildListInfo *pBuild = pSide->getBuildList();
		while (pBuild) {
			pBuild->setRenderObj(NULL);
			pBuild = pBuild->getNext();
		}
	}

	m_needToLoadRoads = true; // load roads next time we redraw.

	if (TheW3DShadowManager)
		TheW3DShadowManager->Reset();

	updateLights();
	if (m_heightMapRenderObj) {
		m_scene->Add_Render_Object(m_heightMapRenderObj);
		m_heightMapRenderObj->removeAllTrees();
		m_heightMapRenderObj->removeAllProps();
	}
}

// ----------------------------------------------------------------------------
void WbView3d::stepTimeOfDay()
{
	TheWritableGlobalData->m_timeOfDay = (TimeOfDay)(TheGlobalData->m_timeOfDay+1);
	if (TheGlobalData->m_timeOfDay >= TIME_OF_DAY_COUNT) {
		TheWritableGlobalData->m_timeOfDay = TIME_OF_DAY_FIRST;
	}
	resetRenderObjects();
	invalObjectInView(NULL);
}

// ----------------------------------------------------------------------------
void WbView3d::updateLights()
{
	++m_updateCount;

	// Update lights list.
	m_lightList.Reset_List();

	{
		TheWritableGlobalData->setTimeOfDay(TheGlobalData->m_timeOfDay);
		const GlobalData::TerrainLighting *ol = &TheGlobalData->m_terrainObjectsLighting[TheGlobalData->m_timeOfDay][0];

		if( m_scene )
		{
			m_scene->Set_Ambient_Light( Vector3(ol->ambient.red, ol->ambient.green, ol->ambient.blue) );
			m_baseBuildScene->Set_Ambient_Light( Vector3(ol->ambient.red, ol->ambient.green, ol->ambient.blue) );
		}

		if (TheW3DShadowManager) {
			TheW3DShadowManager->setTimeOfDay(TheGlobalData->m_timeOfDay);
		}

		for (Int i=0; i<MAX_GLOBAL_LIGHTS; i++)
		{

			if( m_globalLight[i] )
			{
				ol = &TheGlobalData->m_terrainObjectsLighting[TheGlobalData->m_timeOfDay][i];
				m_globalLight[i]->Set_Ambient( Vector3( 0.0f, 0.0f, 0.0f ) );
				m_globalLight[i]->Set_Diffuse( Vector3(ol->diffuse.red, ol->diffuse.green, ol->diffuse.blue ) );
				m_globalLight[i]->Set_Specular( Vector3(0,0,0) );
				Matrix3D mtx;
				mtx.Set(Vector3(1,0,0), Vector3(0,1,0), Vector3(ol->lightPos.x, ol->lightPos.y, ol->lightPos.z), Vector3(0,0,0));
				m_globalLight[i]->Set_Transform(mtx);
 				m_scene->setGlobalLight(m_globalLight[i],i);
 				m_baseBuildScene->setGlobalLight(m_globalLight[i],i);
			}
		}
		if(TheTerrainRenderObject) {
			TheTerrainRenderObject->setTimeOfDay(TheGlobalData->m_timeOfDay);
		}
	}

	MapObject *pMapObj = MapObject::getFirstMapObject();
	while (pMapObj && m_heightMapRenderObj) {
		if (pMapObj->isLight()) {
			Coord3D loc = *pMapObj->getLocation();
			loc.z += m_heightMapRenderObj->getHeightMapHeight(loc.x, loc.y, NULL);
			RenderObjClass *renderObj= pMapObj->getRenderObj();
			if (renderObj) {
				m_scene->Remove_Render_Object(renderObj);
				pMapObj->setRenderObj(NULL);
			}
			// It is a light, and handled at the device level.  jba.
			LightClass* lightP = NEW_REF(LightClass, (LightClass::POINT));

			Dict *props = pMapObj->getProperties();

			Real lightHeightAboveTerrain, lightInnerRadius, lightOuterRadius;
			RGBColor lightAmbientColor, lightDiffuseColor;
			lightHeightAboveTerrain = props->getReal(TheKey_lightHeightAboveTerrain);
			lightInnerRadius = props->getReal(TheKey_lightInnerRadius);
			lightOuterRadius = props->getReal(TheKey_lightOuterRadius);
			lightAmbientColor.setFromInt(props->getInt(TheKey_lightAmbientColor));
			lightDiffuseColor.setFromInt(props->getInt(TheKey_lightDiffuseColor));

			lightP->Set_Ambient( Vector3( lightAmbientColor.red, lightAmbientColor.green, lightAmbientColor.blue ) );
			lightP->Set_Diffuse( Vector3(  lightDiffuseColor.red, lightDiffuseColor.green, lightDiffuseColor.blue) );

			lightP->Set_Position(Vector3(loc.x, loc.y, loc.z+lightHeightAboveTerrain));

			lightP->Set_Far_Attenuation_Range(lightInnerRadius, lightOuterRadius);

			m_lightList.Add(lightP);
 			m_scene->Add_Render_Object(lightP);
			pMapObj->setRenderObj(lightP);
			REF_PTR_RELEASE( lightP );
		}
		pMapObj = pMapObj->getNext();
	}

	--m_updateCount;
}

// ----------------------------------------------------------------------------
void WbView3d::updateScorches(void)
{
	TheTerrainRenderObject->clearAllScorches();
	MapObject *pMapObj;
	for (pMapObj = MapObject::getFirstMapObject(); pMapObj; pMapObj = pMapObj->getNext())
	{
		if (pMapObj->isScorch())
		{
			const Coord3D *pos = pMapObj->getLocation();
			Real radius = pMapObj->getProperties()->getReal(TheKey_objectRadius);
			Scorches type = (Scorches) pMapObj->getProperties()->getInt(TheKey_scorchType);

			Vector3 loc(pos->x, pos->y, pos->z);
			TheTerrainRenderObject->addScorch(loc, radius, type);
		}
	}
}

// ----------------------------------------------------------------------------
void WbView3d::updateTrees(void)
{
	TheTerrainRenderObject->removeAllTrees();
	TheTerrainRenderObject->removeAllProps();
	MapObject *pMapObj;
	for (pMapObj = MapObject::getFirstMapObject(); pMapObj; pMapObj = pMapObj->getNext())
	{
		const ThingTemplate *tTemplate;

		tTemplate = pMapObj->getThingTemplate();
		if (tTemplate && tTemplate->isKindOf(KINDOF_OPTIMIZED_TREE) )
		{
			Real scale = tTemplate->getAssetScale();
			const ModuleInfo& mi = tTemplate->getDrawModuleInfo();
			if (mi.getCount() > 0)
			{
				const ModuleData* mdd = mi.getNthData(0);
				const W3DTreeDrawModuleData* md = mdd ? mdd->getAsW3DTreeDrawModuleData(): NULL;
				if (md)
				{
					Coord3D pos = *pMapObj->getLocation();
					if (m_heightMapRenderObj) {
						pos.z += m_heightMapRenderObj->getHeightMapHeight(pos.x, pos.y, NULL);
						TheTerrainRenderObject->addTree((DrawableID)(uintptr_t)pMapObj, pos, scale, pMapObj->getAngle(),
							0.0f /*no random scaling*/, md);
					}
				}
			}
		}
	}
}

// ----------------------------------------------------------------------------
void WbView3d::invalidateCellInView(int xIndex, int yIndex)
{
	update();	/// @todo be smarter about invaling the area
}

// ----------------------------------------------------------------------------
/// @todo srj -- this is a terrible hack, since things can have multiple models, and it's private info. fix later.
AsciiString WbView3d::getBestModelName(const ThingTemplate* tt, const ModelConditionFlags& c)
{
	if (tt)
	{
		const ModuleInfo& mi = tt->getDrawModuleInfo();
		if (mi.getCount() > 0)
		{
			const ModuleData* mdd = mi.getNthData(0);
			const W3DModelDrawModuleData* md = mdd ? mdd->getAsW3DModelDrawModuleData() : NULL;
			if (md)
			{
				return md->getBestModelNameForWB(c);
			}
		}
	}
	// removing this crash as sounds can (and should) have no model - jkmcd
	return AsciiString::TheEmptyString;
}

// ----------------------------------------------------------------------------
AsciiString WbView3d::getModelNameAndScale(MapObject *pMapObj, Real *scale, BodyDamageType curDamageState)
{
	ModelConditionFlags state;
	switch (curDamageState)
	{
			case BODY_PRISTINE:
			default:
				state.clear();
				break;

			case BODY_DAMAGED:
				state.set(MODELCONDITION_DAMAGED);
				break;

			case BODY_REALLYDAMAGED:
				state.set(MODELCONDITION_REALLY_DAMAGED);
				break;

			case BODY_RUBBLE:
				state.set(MODELCONDITION_RUBBLE);
				break;
	}

	if (getShowGarrisoned())
	{
		state.set(MODELCONDITION_GARRISONED);
	}
	Int objWeather = 0;
	Bool exists;
	if (pMapObj && pMapObj->getProperties())
	{
		objWeather = pMapObj->getProperties()->getInt(TheKey_objectWeather, &exists);
	}
	switch (objWeather)
	{
		default:
		case 0:
			if (TheGlobalData->m_weather == WEATHER_SNOWY)
			{
				state.set(MODELCONDITION_SNOW);
			}
			break;
		case 2:
			state.set(MODELCONDITION_SNOW);
			break;
	}

	Int objTime = 0;
	if (pMapObj && pMapObj->getProperties())
	{
		objTime = pMapObj->getProperties()->getInt(TheKey_objectTime, &exists);
	}
	switch (objTime)
	{
		default:
		case 0:
			if (TheGlobalData->m_timeOfDay == TIME_OF_DAY_NIGHT)
			{
				state.set(MODELCONDITION_NIGHT);
			}
			break;
		case 2:
			state.set(MODELCONDITION_NIGHT);
			break;
	}

	AsciiString modelName("No Model Name");
	*scale = 1.0f;
	Int i;
	char buffer[ _MAX_PATH ];
	if (strncmp(TEST_STRING, pMapObj->getName().str(), strlen(TEST_STRING)) == 0)
	{
		/* Handle test art models here */
		strcpy(buffer, pMapObj->getName().str());

		for (i=0; buffer[i]; i++) {
			if (buffer[i] == '/') {
				i++;
				break;
			}
		}
		modelName = buffer+i;
	}
	else
	{
		modelName = "No Model Name"; // must be this while GDF exists (it's the default)
		const ThingTemplate *tTemplate;

		tTemplate = pMapObj->getThingTemplate();
		if( tTemplate && !(pMapObj->getFlags() & FLAG_DONT_RENDER))
		{
			// get visual data from the thing template
			modelName = getBestModelName(tTemplate, state);
			*scale = tTemplate->getAssetScale();
		}  // end if
	}  // end else
	return modelName;
}

// ----------------------------------------------------------------------------
void WbView3d::invalObjectInView(MapObject *pMapObjIn)
{
	++m_updateCount;
	Bool updateAllTrees = false;
	if (m_heightMapRenderObj == NULL) {
		m_heightMapRenderObj = NEW_REF(WBHeightMap,());

		m_scene->Add_Render_Object(m_heightMapRenderObj);
	}
	Bool found = false;
	Bool isRoad = false;
	Bool isLight = false;
	Bool isScorch = false;
	if (pMapObjIn == NULL)
		isScorch = true;
	MapObject *pMapObj;
	for (pMapObj = MapObject::getFirstMapObject(); pMapObj; pMapObj = pMapObj->getNext())
	{
		if (found) break;
		if (pMapObjIn == pMapObj)
			found = true;
		if (pMapObjIn != NULL && !found) {
			continue;
		}
		if (pMapObj->getFlags() & (FLAG_ROAD_FLAGS|FLAG_BRIDGE_FLAGS)) {
			isRoad = true;
			continue; // Roads don't create drawable objects.
		}
		if (pMapObj->isLight() ) {
			isLight = true;
			continue; // Lights don't create drawable objects.
		}
		if (pMapObj->isScorch()) {
			if (pMapObj == pMapObjIn) {
				isScorch = true;
			}
			continue;
		}

		Coord3D loc = *pMapObj->getLocation();
		loc.z += m_heightMapRenderObj->getHeightMapHeight(loc.x, loc.y, NULL);

		const ThingTemplate *tTemplate = pMapObj->getThingTemplate();
		if (tTemplate && tTemplate->isKindOf(KINDOF_OPTIMIZED_TREE)) {
			if (!m_heightMapRenderObj->updateTreePosition((DrawableID)(uintptr_t)pMapObj, loc, pMapObj->getAngle())) {
				// Couldn't find it, so update them all. [5/27/2003]
				updateAllTrees = true;
			}
			if (found) break;
		}

		RenderObjClass *renderObj=NULL;
		Shadow		   *shadowObj=NULL;

		REF_PTR_SET( renderObj, pMapObj->getRenderObj() );
		Int playerColor = 0xFFFFFF;
		BodyDamageType curDamageState = BODY_PRISTINE;
		Bool isVehicle = false;

		Bool exists;
		if (tTemplate && !(pMapObj->getFlags() & FLAG_DONT_RENDER))
		{
			isVehicle = tTemplate->isKindOf(KINDOF_VEHICLE);
			AsciiString objectTeamName = pMapObj->getProperties()->getAsciiString(TheKey_originalOwner, &exists);
			if (exists) {
				TeamsInfo *teamInfo = TheSidesList->findTeamInfo(objectTeamName);
				if (teamInfo) {
					AsciiString teamOwner = teamInfo->getDict()->getAsciiString(TheKey_teamOwner);
					SidesInfo* pSide = TheSidesList->findSideInfo(teamOwner);
					if (pSide) {
						Bool hasColor = false;
						Int color = pSide->getDict()->getInt(TheKey_playerColor, &hasColor);
						if (hasColor) {
							playerColor = color;
						} else {
							AsciiString tmplname = pSide->getDict()->getAsciiString(TheKey_playerFaction);
							const PlayerTemplate* pt = ThePlayerTemplateStore->findPlayerTemplate(NAMEKEY(tmplname));
							if (pt) {
								playerColor = pt->getPreferredColor()->getAsInt();
							}
						}
					}
				}
			}
		}
		Int health = 100;
		health = pMapObj->getProperties()->getInt(TheKey_objectInitialHealth, &exists);
		Real ratio = health/100.0;
		if (ratio > TheGlobalData->m_unitDamagedThresh)
		{
			curDamageState = BODY_PRISTINE;
		}
		else if (ratio > TheGlobalData->m_unitReallyDamagedThresh)
		{
			curDamageState = BODY_DAMAGED;
		}
		else if (ratio > 0.0f)
		{
			curDamageState = BODY_REALLYDAMAGED;
		}
		else
		{
			curDamageState = BODY_RUBBLE;
		}

		if (!renderObj) {
			Real scale = 1.0;
			AsciiString modelName = getModelNameAndScale(pMapObj, &scale, curDamageState);
			// set render object, or create if we need to
			if( renderObj == NULL && modelName.isEmpty() == FALSE &&
					strncmp( modelName.str(), "No ", 3 ) )
			{

				if (!getShowModels()) {
					continue;
				}
				renderObj = m_assetManager->Create_Render_Obj( modelName.str(), scale, playerColor);
				if( m_showShadows && renderObj )
				{
					Shadow::ShadowTypeInfo shadowInfo;
					shadowInfo.allowUpdates=FALSE;	//shadow image will never update
					shadowInfo.allowWorldAlign=TRUE;	//shadow image will wrap around world objects
					if (tTemplate && tTemplate->getShadowType() != SHADOW_NONE && !(pMapObj->getFlags() & FLAG_DONT_RENDER))
					{	//add correct type of shadow
						strcpy(shadowInfo.m_ShadowName,tTemplate->getShadowTextureName().str());
						DEBUG_ASSERTCRASH(shadowInfo.m_ShadowName[0] != '\0', ("this should be validated in ThingTemplate now"));
						shadowInfo.m_type=(ShadowType)tTemplate->getShadowType();
						shadowInfo.m_sizeX=tTemplate->getShadowSizeX();
						shadowInfo.m_sizeY=tTemplate->getShadowSizeY();
						shadowInfo.m_offsetX=tTemplate->getShadowOffsetX();
						shadowInfo.m_offsetY=tTemplate->getShadowOffsetY();
						shadowObj=TheW3DShadowManager->addShadow(renderObj, &shadowInfo);
					} else if (!tTemplate) {
						shadowInfo.m_type=(ShadowType)SHADOW_VOLUME;
						shadowObj=TheW3DShadowManager->addShadow(renderObj, &shadowInfo);
					}
				}
			}  // end if
		}

		if (renderObj && !(pMapObj->getFlags() & FLAG_DONT_RENDER)) {
			pMapObj->setRenderObj(renderObj);
			pMapObj->setShadowObj(shadowObj);

			// set item's position to loc, and get scale from item and apply it.

			Matrix3D renderObjPos(true);	// init to identity
			renderObjPos.Translate(loc.x, loc.y, loc.z);
			renderObjPos.Rotate_Z(pMapObj->getAngle());
			renderObj->Set_Transform( renderObjPos );

			if (isVehicle) {
				// note that this affects our orientation, but NOT our position... specifically,
				// it does NOT force us to "stick" to the ground!
				Matrix3D mtx;
				Coord3D terrainNormal;
				m_heightMapRenderObj->getHeightMapHeight(loc.x, loc.y, &terrainNormal );
				makeAlignToNormalMatrix(pMapObj->getAngle(), loc, terrainNormal, mtx);
				renderObj->Set_Transform( mtx );
			}

			m_scene->Add_Render_Object(renderObj);

			REF_PTR_RELEASE(renderObj); // belongs to m_scene now.
		} else if (renderObj) {
			m_scene->Remove_Render_Object(renderObj);
		}
		if (found) break;
	}
	if (!found && pMapObjIn) {
		if (pMapObjIn->getFlags() & (FLAG_ROAD_FLAGS|FLAG_BRIDGE_FLAGS)) {
			isRoad = true;
		}
		const ThingTemplate *tTemplate = pMapObjIn->getThingTemplate();
		if (tTemplate && tTemplate->isKindOf(KINDOF_OPTIMIZED_TREE)) {
			updateAllTrees = true;
		}
	}
	if (!found && pMapObjIn && pMapObjIn->getRenderObj()) {
		if( m_showShadows ) {
			resetRenderObjects();
			invalObjectInView(NULL);
			--m_updateCount;
			return;
		}
		m_scene->Remove_Render_Object(pMapObjIn->getRenderObj());
		pMapObjIn->setRenderObj(NULL);
	}

	if (isRoad) {
		m_needToLoadRoads = true; // load roads next time we redraw.
	}
	if (updateAllTrees) {
		updateTrees();
	}
	if (isLight) {
		updateLights();
	}
	if (isScorch) {
		updateScorches();
	}

	--m_updateCount;
}

// ----------------------------------------------------------------------------
void WbView3d::updateHeightMapInView(WorldHeightMap *htMap, Bool partial, const IRegion2D &partialRange)
{
	if (htMap == NULL)
		return;
	initWW3D();
	++m_updateCount;

	if (m_heightMapRenderObj == NULL) {
		m_heightMapRenderObj = NEW_REF(WBHeightMap,());
		m_scene->Add_Render_Object(m_heightMapRenderObj);
		partial = false;
	}


	if (m_heightMapRenderObj) {

		RefRenderObjListIterator lightListIt(&m_lightList);
		if (partial) {
			m_heightMapRenderObj->doPartialUpdate(partialRange, htMap, &lightListIt);
		} else {
			if (m_showEntireMap) {
				htMap->setDrawOrg(0, 0);
				htMap->setDrawWidth(htMap->getXExtent());
				htMap->setDrawHeight(htMap->getYExtent());
				m_heightMapRenderObj->initHeightData(htMap->getXExtent(), htMap->getYExtent(), htMap, &lightListIt);
			} else {
				htMap->setDrawWidth(m_partialMapSize);
				htMap->setDrawHeight(m_partialMapSize);
				m_heightMapRenderObj->initHeightData(htMap->getDrawWidth(), htMap->getDrawHeight(), htMap, &lightListIt);
			}
			m_heightMapRenderObj->updateViewImpassableAreas();
		}
	}

	invalObjectInView(NULL);	// update all the map objects, to account for ground changes

	--m_updateCount;
}

// ----------------------------------------------------------------------------
void WbView3d::setCenterInView(Real x, Real y)
{
	if (x != m_centerPt.X || y != m_centerPt.Y) {
		m_centerPt.X = x;
		m_centerPt.Y = y;
		constrainCenterPt();
		redraw();
		CMainFrame::GetMainFrame()->handleCameraChange();
	}
}

// ----------------------------------------------------------------------------
void WbView3d::setShowTopDownView(Bool show)
{
	m_projection = show;
	redraw();
}

// ----------------------------------------------------------------------------
void WbView3d::setShowEntireMap(Bool show)
{
	m_showEntireMap = show;
	CWorldBuilderDoc *pDoc = WbDoc();
	if (pDoc && pDoc->GetHeightMap()) {
		IRegion2D range = {0,0,0,0};
		updateHeightMapInView(pDoc->GetHeightMap(), false, range);
	}
	redraw();
}

// ----------------------------------------------------------------------------
Bool WbView3d::viewToDocCoords(QPoint curPt, Coord3D *newPt, Bool constrain)
{
	Bool result = false;
	if (!m_ww3dInited || m_camera == NULL) {
		newPt->zero();
		return false;
	}

	// get our "logical" or relative screen coords
	float logX = (Real)curPt.x() / (Real)width();
	float logY = (Real)curPt.y() / (Real)height();
	Vector3 intersection(0,0,0);
	// determine the ray corresponding to the camera and distance to projection plane
	Matrix3D camera_matrix = m_camera->Get_Transform();

	Vector3 camera_location  = m_camera->Get_Position();

	Vector3 rayLocation;
	Vector3 rayDirection;
	Vector3 rayDirectionPt;
	// the projected ray has the same origin as the camera
	rayLocation = camera_location;
	// determine the location of the screen coordinate in camera-model space
	const ViewportClass &viewport = m_camera->Get_Viewport();

	Vector2 vmin,vmax;
	m_camera->Get_View_Plane(vmin,vmax);
	float xscale = (vmax.X - vmin.X);
	float yscale = (vmax.Y - vmin.Y);

	float zmod = -1.0; // Note: view plane distance is now always 1.0 from the camera
	float xmod = (-logX + 0.5 + viewport.Min.X) * zmod * xscale;
	float ymod = (logY - 0.5 - viewport.Min.Y) * zmod * yscale;

	// transform the screen coordinates by the camera's matrix into world coordinates.
	float x = zmod * camera_matrix[0][2] + xmod * camera_matrix[0][0] + ymod * camera_matrix[0][1];
	float y = zmod * camera_matrix[1][2] + xmod * camera_matrix[1][0] + ymod * camera_matrix[1][1];
	float z = zmod * camera_matrix[2][2] + xmod * camera_matrix[2][0] + ymod * camera_matrix[2][1];

	rayDirection.Set(x,y,z);
	rayDirection.Normalize();
	float MaxDistance =  m_camera->Get_Depth()*MAP_XY_FACTOR;
	rayDirectionPt = rayLocation + rayDirection*MaxDistance;

	LineSegClass ray(rayLocation, rayDirectionPt);

	// Note - there are 2 ways to track.  One is for tools (like paint texture)
	// that follow the terrain.  They want to track the terrain, so the texturing
	// follows the cursor.  Most tools, however, don't want to jump up & down clifs
	// and such.  So they use a fixed z plane when tracking, so things don't move
	// depending what you move over.
	Bool followTerrain = true;
	if (WbApp()->isCurToolLocked() && WbApp()->getCurTool()) {
		followTerrain = WbApp()->getCurTool()->followsTerrain();
	}
	if (followTerrain && TheTerrainRenderObject) {
		CastResultStruct castResult;
		RayCollisionTestClass rayCollide(ray, &castResult) ;
		if( TheTerrainRenderObject->Cast_Ray(rayCollide) )
		{
			// get the point of intersection according to W3D
			intersection = castResult.ContactPoint;
			m_curTrackingZ = intersection.Z;
			result = true;
		}  // end if
	}
	if (!result) {
		intersection.X = Vector3::Find_X_At_Z(m_curTrackingZ, rayLocation, rayDirectionPt);
		intersection.Y = Vector3::Find_Y_At_Z(m_curTrackingZ, rayLocation, rayDirectionPt);
		result = true;
	}
	newPt->x = intersection.X;
	newPt->y = intersection.Y;
	newPt->z = MAGIC_GROUND_Z;
	if (constrain) {
		if (m_doLockAngle) {
			Real dy = fabs(m_mouseDownDocPoint.y - newPt->y);
			Real dx = fabs(m_mouseDownDocPoint.x - newPt->x);
			if (dx>2*dy) {
				// lock to dx.
				newPt->y = m_mouseDownDocPoint.y;
			} else if (dy>2*dx) {
				//lock to dy.
				newPt->x = m_mouseDownDocPoint.x;
			} else {
				// Lock to 45 degree.
				dx = (dx+dy)/2;
				dy = dx;
				if (newPt->x < m_mouseDownDocPoint.x) dx = -dx;
				if (newPt->y < m_mouseDownDocPoint.y) dy = -dy;
				newPt->x = m_mouseDownDocPoint.x+dx;
				newPt->y = m_mouseDownDocPoint.y+dy;
			}
		}
	}
	return result;
}

//=============================================================================
/** Returns the map object whose render object is under the pixel location. */
//=============================================================================
MapObject *WbView3d::picked3dObjectInView(QPoint viewPt)
{
	// This code picks on all 3d objects.
	if (m_intersector && m_layer) {
		float logX = (Real)viewPt.x() / (Real)width();
		float logY = (Real)viewPt.y() / (Real)height();
		// do the intersection using W3D intersector class
		Bool hit = m_intersector->Intersect_Screen_Point_Layer( logX, logY, *m_layer );
		if( hit )
		{
			MapObject *pObj;
			for (pObj = MapObject::getFirstMapObject(); pObj; pObj = pObj->getNext()) {
				if (pObj->getRenderObj() == m_intersector->Result.IntersectedRenderObject) {
					return pObj;
				}
			}
		}
	}

	return NULL;
}

// ----------------------------------------------------------------------------
Bool WbView3d::docToViewCoords(Coord3D curPt, QPoint* newPt)
{
	if (!m_ww3dInited || m_camera == NULL) {
		return false;
	}
	Vector3 world(curPt.x, curPt.y, curPt.z);
	Vector3 screen;
	if (m_camera->Project(screen, world) != CameraClass::INSIDE_FRUSTUM) {
		return false;
	}
	// Project returns screen coords in -1..1 range; map to widget pixels.
	newPt->setX((Int)((screen.X + 1.0f) * 0.5f * width()));
	newPt->setY((Int)((1.0f - screen.Y) * 0.5f * height()));
	return true;
}

// ----------------------------------------------------------------------------
void WbView3d::scrollInView(Real xScroll, Real yScroll, Bool end)
{
	m_centerPt.X += xScroll;
	m_centerPt.Y += yScroll;
	constrainCenterPt();
	redraw();
	if (end)
		WbDoc()->syncViewCenters(m_centerPt.X, m_centerPt.Y);
	CMainFrame::GetMainFrame()->handleCameraChange();
}

// ----------------------------------------------------------------------------
void WbView3d::setDefaultCamera()
{
	m_mouseWheelOffset = 0;
	m_cameraAngle = 0;
	m_FXPitch = 1.0f;
	if (m_centerPt.X < 0) m_centerPt.X = 0;
	if (m_centerPt.Y < 0) m_centerPt.Y = 0;
	CWorldBuilderDoc *pDoc = CWorldBuilderDoc::GetActiveDoc();
	if (pDoc) {
		WorldHeightMapEdit *pMap = pDoc->GetHeightMap();
		if (pMap) {
			if (m_centerPt.X > pMap->getXExtent()) {
				m_centerPt.X = pMap->getXExtent();
			}
			if (m_centerPt.Y > pMap->getYExtent()) {
				m_centerPt.Y = pMap->getYExtent();
			}
		}
	}
	m_groundLevel = 10.0;
	Coord3D pos;
	pos.x = m_centerPt.X;
	pos.y = m_centerPt.Y;
	pos.z = m_centerPt.Z;
	AsciiString startingCamName = TheNameKeyGenerator->keyToName(TheKey_InitialCameraPosition);
	MapObject *pMapObj = MapObject::getFirstMapObject();
	while (pMapObj) {
		if (pMapObj->isWaypoint()) {
			if (startingCamName == pMapObj->getWaypointName()) {
				pos = *pMapObj->getLocation();
			}
		}
		pMapObj = pMapObj->getNext();
	}

	if (m_heightMapRenderObj) {
		m_groundLevel = m_heightMapRenderObj->getHeightMapHeight(pos.x, pos.y, NULL);
	}

	m_cameraOffset.z = m_groundLevel+TheGlobalData->m_maxCameraHeight;
	m_cameraOffset.y = -(m_cameraOffset.z / tan(TheGlobalData->m_cameraPitch * (PI / 180.0)));
	m_cameraOffset.x = -(m_cameraOffset.y * tan(TheGlobalData->m_cameraYaw * (PI / 180.0)));

	redraw();
	CMainFrame::GetMainFrame()->handleCameraChange();
}

// ----------------------------------------------------------------------------
void WbView3d::rotateCamera(Real delta)
{
	if (m_projection) return; // camera doesn't rotate in top down view.
	m_cameraAngle += delta;
	redraw();
	CMainFrame::GetMainFrame()->handleCameraChange();
}

// ----------------------------------------------------------------------------
void WbView3d::pitchCamera(Real delta)
{
	if (m_projection) return; // camera doesn't pitch in top down view.
	m_FXPitch += delta;
	redraw();
	CMainFrame::GetMainFrame()->handleCameraChange();
}

// ----------------------------------------------------------------------------
void WbView3d::setCameraPitch(Real absolutePitch)
{
	if (m_projection) return; // camera doesn't pitch in top down view.
	m_FXPitch = absolutePitch;
	redraw();
	CMainFrame::GetMainFrame()->handleCameraChange();
}

// ----------------------------------------------------------------------------
Real WbView3d::getCameraPitch(void)
{
	return m_FXPitch;
}

//WST 10.17.2002 ----------------------------------------------------------------------------
Real WbView3d::getCurrentZoom(void)
{
	float zOffset = - m_mouseWheelOffset / 1200.0f; //WST 11/21/02 new triple speed camera zoom.
	Real zoom = 1.0f;
	if (zOffset != 0) {
		Real zPos = (m_cameraOffset.length()-m_groundLevel)/m_cameraOffset.length();
		Real zAbs = zOffset + zPos;
		if (zAbs<0) zAbs = -zAbs;
		if (zAbs<0.01) zAbs = 0.01f;
		if (zOffset > 0) {
			zOffset *= zAbs;
		}	else if (zOffset < -0.3f) {
			zOffset = -0.15f + zOffset/2.0f;
		}
		if (zOffset < -0.6f) {
			zOffset = -0.3f + zOffset/2.0f;
		}
		zoom = zAbs;
	}
	return zoom;
}

// ----------------------------------------------------------------------------
// Menu/keyboard zoom: same wheel-offset curve as the mouse wheel (one "step"
// equals a standard wheel notch of 120).
static const Int ZOOM_STEP = 300;

void WbView3d::zoomIn(void)
{
	m_mouseWheelOffset += ZOOM_STEP;
	redraw();
	CMainFrame::GetMainFrame()->handleCameraChange();
}

void WbView3d::zoomOut(void)
{
	m_mouseWheelOffset -= ZOOM_STEP;
	redraw();
	CMainFrame::GetMainFrame()->handleCameraChange();
}

void WbView3d::zoomReset(void)
{
	m_mouseWheelOffset = 0;
	redraw();
	CMainFrame::GetMainFrame()->handleCameraChange();
}

// ----------------------------------------------------------------------------
void WbView3d::wheelEvent(QWheelEvent *event)
{
	if (m_trackingMode == TRACK_NONE) {
		Int zDelta = event->angleDelta().y();

		//WST 11/21/02 New Triple speed camera zoom request by designers
		if (getCurrentZoom() > 2.0f)
		{
			m_mouseWheelOffset += zDelta;
		}
		else if (getCurrentZoom() > 1.0f)
		{
			m_mouseWheelOffset += zDelta/2;
		}
		else
		{
			m_mouseWheelOffset += zDelta/8;
		}

		redraw();
		CMainFrame::GetMainFrame()->handleCameraChange();
	}
	event->accept();
}

// ----------------------------------------------------------------------------
void WbView3d::resizeEvent(QResizeEvent *event)
{
	WbView::resizeEvent(event);
	if (m_ww3dInited) {
		reset3dEngineDisplaySize(width(), height());
		m_camera->Set_Aspect_Ratio((float)width()/(float)height());
		redraw();
	}
}

// ----------------------------------------------------------------------------
void WbView3d::redraw(void)
{
	if (m_updateCount > 0) {
		return;
	}
	if (!isVisible()) {
		return;
	}
	if (!m_ww3dInited) {
		return;
	}

	setupCamera();

	if (m_heightMapRenderObj) {
		if (m_needToLoadRoads) {
			m_heightMapRenderObj->loadRoadsAndBridges(NULL,FALSE);
			m_heightMapRenderObj->worldBuilderUpdateBridgeTowers( m_assetManager, m_scene );
			m_needToLoadRoads = false;
		}
		++m_updateCount;
		RefRenderObjListIterator lightListIt(&m_lightList);
		m_heightMapRenderObj->updateCenter(m_camera, &lightListIt);
		m_heightMapRenderObj->On_Frame_Update();
		--m_updateCount;
	}

	WW3D::Sync( wbTickCount() );
	m_buildRedMultiplier += (wbTickCount()-m_time)/500.0f;
	if (m_buildRedMultiplier>4.0f || m_buildRedMultiplier<0) {
		m_buildRedMultiplier = 0;
	}
	render();
	m_time = wbTickCount();
}

// ----------------------------------------------------------------------------
void WbView3d::render()
{
	++m_updateCount;

	if (WW3D::Begin_Render(true,true,Vector3(0.5f,0.5f,0.5f), TheWaterTransparency->m_minWaterOpacity) == WW3D_ERROR_OK)
	{
		if (m_heightMapRenderObj) {
			m_heightMapRenderObj->Set_Hidden((m_showTerrain ? 0 : 1));
			m_heightMapRenderObj->doTextures(true);
		}
		m_scene->Set_Polygon_Mode(SceneClass::FILL);
		// Render 3D scene
		WW3D::Render(m_scene,m_camera);
		Vector3 amb = m_baseBuildScene->Get_Ambient_Light();
		Vector3 newAmb(amb);
		Real mul = m_buildRedMultiplier;
		if (mul>2.0f) mul = 4.0f-mul;
		Real gMul = 2.0-mul;
		newAmb.X *= mul;
		newAmb.Y *= gMul;
		if (newAmb.X>1) newAmb.X = 1;
		m_baseBuildScene->Set_Ambient_Light(newAmb);
		WW3D::Render(m_baseBuildScene,m_camera);
		m_baseBuildScene->Set_Ambient_Light(amb);

		if (m_showWireframe) {
			if (m_heightMapRenderObj) {
				m_heightMapRenderObj->doTextures(false);
				m_scene->Set_Polygon_Mode(SceneClass::LINE);
				// Render 3D scene
				WW3D::Render(m_scene,m_camera);
				WW3D::Render(m_baseBuildScene,m_camera);
				m_heightMapRenderObj->doTextures(true);
			}
		}

		// Draw the 3d obj icons on top of the rest of the data.
		WW3D::Render(m_overlayScene,m_camera);

		WW3D::End_Render();
	}
	--m_updateCount;
}
