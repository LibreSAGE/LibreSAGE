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

#include <QDir>
#include <QSettings>

// WorldBuilder.cpp : Defines the class behaviors for the application.
//
// Qt6 port of the original MFC CWorldBuilderApp.  The engine bring-up mirrors
// the original InitInstance/ExitInstance, with the Win32 device
// implementations swapped for their SDL3 counterparts.

#include "WorldBuilder.h"
#include "MainFrm.h"
#include "SplashScreen.h"
#include "BrushTool.h"
#include "FeatherTool.h"
#include "HandScrollTool.h"
#include "MoundTool.h"
#include "ObjectTool.h"
#include "PointerTool.h"
#include "WHeightMapEdit.h"
#include "WorldBuilderDoc.h"

#include "W3DDevice/GameClient/W3DFileSystem.h"
#include "Common/GlobalData.h"
#include "Common/FileSystem.h"
#include "Common/ArchiveFileSystem.h"
#include "Common/LocalFileSystem.h"
#include "Common/CDManager.h"
#include "Common/CriticalSection.h"
#include "Common/Debug.h"
#include "Common/GameMemory.h"
#include "Common/MapObject.h"
#include "Common/NameKeyGenerator.h"
#include "Common/Science.h"
#include "Common/SubsystemInterface.h"
#include "Common/ThingFactory.h"
#include "Common/INI.h"
#include "Common/INIParsers.h"
#include "Common/CommandLine.h"
#include "Common/GameAudio.h"
#include "Common/SpecialPower.h"
#include "Common/TerrainTypes.h"
#include "Common/DamageFX.h"
#include "Common/Upgrade.h"
#include "Common/ModuleFactory.h"
#include "Common/PlayerTemplate.h"
#include "Common/MultiplayerSettings.h"

#include "GameLogic/Armor.h"
#include "GameLogic/CaveSystem.h"
#include "GameLogic/CrateSystem.h"
#include "GameLogic/ObjectCreationList.h"
#include "GameLogic/Weapon.h"
#include "GameLogic/RankInfo.h"
#include "GameLogic/SidesList.h"
#include "GameLogic/ScriptEngine.h"
#include "GameClient/Anim2D.h"
#include "GameClient/GameText.h"
#include "GameClient/ParticleSys.h"
#include "GameClient/Water.h"
#include "GameClient/TerrainRoads.h"
#include "GameClient/FXList.h"
#include "GameClient/VideoPlayer.h"
#include "GameLogic/Locomotor.h"

#include "W3DDevice/Common/W3DModuleFactory.h"
#include "W3DDevice/GameClient/W3DParticleSys.h"
#include "SDL3Device/Common/SDL3LocalFileSystem.h"
#include "SDL3Device/Common/SDL3BIGFileSystem.h"
#if defined(SAGE_USE_MINIAUDIO)
#include "MiniAudioDevice/MiniAudioManager.h"
#elif defined(SAGE_USE_MILES)
#include "MilesAudioDevice/MilesAudioManager.h"
#endif


#include <SDL3/SDL.h>

// ----------------------------------------------------------------------------
// Globals the engine libraries expect the application to provide (the game
// defines the same set in SDL3Main.cpp).
// ----------------------------------------------------------------------------
HINSTANCE ApplicationHInstance = NULL;
HWND ApplicationHWnd = NULL;
Bool ApplicationIsWindowed = true;
SDL_Window *ApplicationWindow = NULL;
class SDL3Mouse;
SDL3Mouse *TheWin32Mouse = NULL;
DWORD TheMessageTime = 0;

const char *gAppPrefix = "wb_"; /// So WB can have a different debug log file name.
const Char *g_strFile = "data\\Generals.str";
const Char *g_csfFile = "data\\%s\\Generals.csf";

static SubsystemInterfaceList TheSubsystemListRecord;

template<class SUBSYSTEM>
void initSubsystem(SUBSYSTEM*& sysref, SUBSYSTEM* sys, const char* path1 = NULL, const char* path2 = NULL, const char* dirpath = NULL)
{
	sysref = sys;
	TheSubsystemListRecord.initSubsystem(sys, path1, path2, dirpath, NULL);
}

/////////////////////////////////////////////////////////////////////////////
// WBGameFileClass - extends the file system a bit so we can get at some
// wb only data.  jba.

class WBGameFileClass : public GameFileClass
{

public:
	WBGameFileClass(char const *filename):GameFileClass(filename){};
	virtual char const * Set_Name(char const *filename);
};

//-------------------------------------------------------------------------------------------------
/** Sets the file name, and finds the GDI asset if present. */
//-------------------------------------------------------------------------------------------------
char const * WBGameFileClass::Set_Name( char const *filename )
{
	char const *pChar = GameFileClass::Set_Name(filename);
	if (this->Is_Available()) {
		return pChar; // it was found by the parent class.
	}

	if (TheFileSystem->doesFileExist(filename)) {
		strcpy( m_filePath, filename );
		m_fileExists = true;
	}
	return m_filename;
}



/////////////////////////////////////////////////////////////////////////////
// WB_W3DFileSystem - extends the file system a bit so we can get at some
// wb only data.  jba.

class	WB_W3DFileSystem : public W3DFileSystem {
	virtual FileClass * Get_File( char const *filename );
};

//-------------------------------------------------------------------------------------------------
/** Gets a file with the specified filename. */
//-------------------------------------------------------------------------------------------------
FileClass * WB_W3DFileSystem::Get_File( char const *filename )
{
	WBGameFileClass *pFile = new WBGameFileClass( filename );
	if (!pFile->Is_Available()) {
		pFile->Set_Name(filename);
	}
	return pFile;
}


/////////////////////////////////////////////////////////////////////////////
// WorldBuilderApp construction/destruction

WorldBuilderApp::WorldBuilderApp(int &argc, char **argv) :
	QApplication(argc, argv),
	m_curTool(NULL),
	m_selTool(NULL),
	m_brushTool(NULL),
	m_pointerTool(NULL),
	m_handScrollTool(NULL),
	m_lockCurTool(0),
	m_document(NULL),
	m_pasteMapObjList(NULL),
	m_engineInited(false)
{
	setOrganizationName("WWVegas");
	setApplicationName("WorldBuilder");

	// Note: no QCommandLineParser here - the engine command line (-dir,
	// -bigdir, ...) is parsed by CommandLine.cpp in initEngine().

	for (Int i=0; i<NUM_VIEW_TOOLS; i++) {
		m_tools[i] = NULL;
	}
	/// @todo re-add the remaining tools as they get ported (see the original
	/// MFC constructor for the full palette and initial mound/feather values).
	m_brushTool = new BrushTool;
	m_pointerTool = new PointerTool;
	m_handScrollTool = new HandScrollTool;
	MoundTool *moundTool = new MoundTool;
	DigTool *digTool = new DigTool;
	FeatherTool *featherTool = new FeatherTool;
	ObjectTool *objectTool = new ObjectTool;
	m_tools[0] = m_brushTool;
	m_tools[2] = featherTool;
	m_tools[6] = moundTool;
	m_tools[7] = digTool;
	m_tools[9] = objectTool;
	m_tools[10] = m_pointerTool;
	m_tools[15] = m_handScrollTool;

	// set up initial values.
	m_brushTool->setHeight(16);
	m_brushTool->setWidth(3);
	m_brushTool->setFeather(3);
	MoundTool::setMoundHeight(3);
	MoundTool::setWidth(3);
	MoundTool::setFeather(3);
	FeatherTool::setFeather(3);
	FeatherTool::setRadius(1);
	FeatherTool::setRate(2);
}

WorldBuilderApp::~WorldBuilderApp()
{
	m_curTool = NULL;
	m_selTool = NULL;

	for (Int i=0; i<NUM_VIEW_TOOLS; i++) {
		if (m_tools[i]) {
			delete m_tools[i];
			m_tools[i] = NULL;
		}
	}

	shutdownEngine();
}

void WorldBuilderApp::deletePasteObjList(void)
{
	if (m_pasteMapObjList)
		m_pasteMapObjList->deleteInstance();
	m_pasteMapObjList = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// WorldBuilderApp engine bring-up (port of InitInstance)

bool WorldBuilderApp::initEngine(int argc, char **argv, SplashScreen *splash)
{
	if (m_engineInited) {
		return true;
	}
	m_engineInited = true;

	if (splash) {
		splash->outputText("Loading...");
	}

	// DXVK renders through SDL3's window system integration.
	setenv("DXVK_WSI_DRIVER", "SDL3", 1);

	// start the log
	DEBUG_INIT(DEBUG_FLAGS_DEFAULT);

#ifdef DEBUG_LOGGING
	// Turn on console output jba [3/20/2003]
	DebugSetFlags(DebugGetFlags() | DEBUG_FLAG_LOG_TO_CONSOLE);
#endif

	DEBUG_LOG(("starting Worldbuilder.\n"));

#ifdef _INTERNAL
	DEBUG_LOG(("_INTERNAL defined.\n"));
#endif
#ifdef _DEBUG
	DEBUG_LOG(("_DEBUG defined.\n"));
#endif
	initMemoryManager();

	// not part of the subsystem list, because it should normally never be reset!
	TheNameKeyGenerator = new NameKeyGenerator;
	TheNameKeyGenerator->init();

	// The engine expects the working directory to contain the game data.  The
	// game directory is passed via -dir, like the game does (SDL3Main.cpp);
	// without it, fall back to the executable's directory like the original
	// tool did.
	QDir::setCurrent(QCoreApplication::applicationDirPath());
	for (int index = 1; index < argc; ++index) {
		if (SDL_strcasecmp(argv[index], "-dir") == 0) {
			if (index + 1 < argc) {
				QDir::setCurrent(QString::fromUtf8(argv[index + 1]));
			}
			index += 1; // skip the next arg since we just used it
		}
	}

	registerINIBlockParsers();

	TheFileSystem = new FileSystem;

	initSubsystem(TheLocalFileSystem, (LocalFileSystem*)new SDL3LocalFileSystem);
	initSubsystem(TheArchiveFileSystem, (ArchiveFileSystem*)new SDL3BIGFileSystem);

	INI ini;

	initSubsystem(TheWritableGlobalData, new GlobalData(), "Data\\INI\\Default\\GameData.ini", "Data\\INI\\GameData.ini");

#if defined(_DEBUG) || defined(_INTERNAL)
	ini.load( AsciiString( "Data\\INI\\GameDataDebug.ini" ), INI_LOAD_MULTIFILE, NULL );
	TheWritableGlobalData->m_debugIgnoreAsserts = false;
#endif

	// special-case: parse command-line parameters after loading global data
	parseCommandLine(argc, argv);
	// Load any extra BIG directories specified on the command line (-bigdir)
	for (const auto& bigDir : TheWritableGlobalData->m_bigDirs)
	{
		TheArchiveFileSystem->loadBigFilesFromDirectory(bigDir, "*.big");
	}

	DEBUG_LOG(("TheWritableGlobalData %x\n", TheWritableGlobalData));

	// ensure the user maps dir exists
	QDir().mkpath(QString("%1Maps").arg(TheGlobalData->getPath_UserData().str()));

	// read the water settings from INI (must do prior to initing GameClient, apparently)
	ini.load( AsciiString( "Data\\INI\\Default\\Water.ini" ), INI_LOAD_OVERWRITE, NULL );
	ini.load( AsciiString( "Data\\INI\\Water.ini" ), INI_LOAD_OVERWRITE, NULL );

	if (splash) {
		splash->outputText("Loading INI data...");
	}

	initSubsystem(TheGameText, CreateGameTextInterface());
	initSubsystem(TheScienceStore, new ScienceStore(), "Data\\INI\\Default\\Science.ini", "Data\\INI\\Science.ini");
	initSubsystem(TheMultiplayerSettings, new MultiplayerSettings(), "Data\\INI\\Default\\Multiplayer.ini", "Data\\INI\\Multiplayer.ini");
	initSubsystem(TheTerrainTypes, new TerrainTypeCollection(), "Data\\INI\\Default\\Terrain.ini", "Data\\INI\\Terrain.ini");
	initSubsystem(TheTerrainRoads, new TerrainRoadCollection(), "Data\\INI\\Default\\Roads.ini", "Data\\INI\\Roads.ini");

	WorldHeightMapEdit::init();

	initSubsystem(TheScriptEngine, (ScriptEngine*)(new ScriptEngine()));

	TheScriptEngine->turnBreezeOff(); // stop the tree sway.

	//  [2/11/2003]
	ini.load( AsciiString( "Data\\Scripts\\Scripts.ini" ), INI_LOAD_OVERWRITE, NULL );

	// need this before TheAudio in case we're running off of CD - TheAudio can try to open Music.big on the CD...
	initSubsystem(TheCDManager, CreateCDManager(), NULL);
#if defined(SAGE_USE_MINIAUDIO)
	initSubsystem(TheAudio, (AudioManager*)new MiniAudioManager());
#elif defined(SAGE_USE_MILES)
	initSubsystem(TheAudio, (AudioManager*)new MilesAudioManager());
#endif

	initSubsystem(TheVideoPlayer, (VideoPlayerInterface*)(new VideoPlayer()));
	initSubsystem(TheModuleFactory, (ModuleFactory*)(new W3DModuleFactory()));
	initSubsystem(TheSidesList, new SidesList());
	initSubsystem(TheCaveSystem, new CaveSystem());
	initSubsystem(TheRankInfoStore, new RankInfoStore(), NULL, "Data\\INI\\Rank.ini");
	initSubsystem(ThePlayerTemplateStore, new PlayerTemplateStore(), "Data\\INI\\Default\\PlayerTemplate.ini", "Data\\INI\\PlayerTemplate.ini");
	initSubsystem(TheSpecialPowerStore, new SpecialPowerStore(), "Data\\INI\\Default\\SpecialPower.ini", "Data\\INI\\SpecialPower.ini" );
	initSubsystem(TheParticleSystemManager, (ParticleSystemManager*)(new W3DParticleSystemManager()));
	initSubsystem(TheFXListStore, new FXListStore(), "Data\\INI\\Default\\FXList.ini", "Data\\INI\\FXList.ini");
	initSubsystem(TheWeaponStore, new WeaponStore(), NULL, "Data\\INI\\Weapon.ini");
	initSubsystem(TheObjectCreationListStore, new ObjectCreationListStore(), "Data\\INI\\Default\\ObjectCreationList.ini", "Data\\INI\\ObjectCreationList.ini");
	initSubsystem(TheLocomotorStore, new LocomotorStore(), NULL, "Data\\INI\\Locomotor.ini");
	initSubsystem(TheDamageFXStore, new DamageFXStore(), NULL, "Data\\INI\\DamageFX.ini");
	initSubsystem(TheArmorStore, new ArmorStore(), NULL, "Data\\INI\\Armor.ini");
	if (splash) {
		splash->outputText("Loading object definitions...");
	}

	initSubsystem(TheThingFactory, new ThingFactory(), "Data\\INI\\Default\\Object.ini", NULL, "Data\\INI\\Object");
	initSubsystem(TheCrateSystem, new CrateSystem(), "Data\\INI\\Default\\Crate.ini", "Data\\INI\\Crate.ini");
	initSubsystem(TheUpgradeCenter, new UpgradeCenter, "Data\\INI\\Default\\Upgrade.ini", "Data\\INI\\Upgrade.ini");
	initSubsystem(TheAnim2DCollection, new Anim2DCollection ); //Init's itself.

	if (splash) {
		splash->outputText("Finalizing...");
	}

	TheSubsystemListRecord.postProcessLoadAll();

	TheW3DFileSystem = new WB_W3DFileSystem;

	// Just to be sure - wb doesn't do well with half res terrain.
	DEBUG_ASSERTCRASH(!TheGlobalData->m_useHalfHeightMap, ("TheGlobalData->m_useHalfHeightMap : Don't use this setting in WB."));
	TheWritableGlobalData->m_useHalfHeightMap = false;

#if defined(_DEBUG) || defined(_INTERNAL)
	// WB never uses the shroud.
	TheWritableGlobalData->m_shroudOn = FALSE;
#endif

	TheWritableGlobalData->m_isWorldBuilder = TRUE;

	m_document = new CWorldBuilderDoc(this);

	selectPointerTool();

	QSettings settings;
	m_currentDirectory = AsciiString(settings.value("App/OpenDirectory").toString().toUtf8().constData());

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// WorldBuilderApp engine shutdown (port of ExitInstance)

void WorldBuilderApp::shutdownEngine()
{
	if (!m_engineInited) {
		return;
	}
	m_engineInited = false;

	QSettings settings;
	settings.setValue("App/OpenDirectory", QString::fromUtf8(m_currentDirectory.str()));
	m_currentDirectory.clear();

	deletePasteObjList();

	ScriptList::reset();

	TheSubsystemListRecord.shutdownAll();

	WorldHeightMapEdit::shutdown();

	delete TheFileSystem;
	TheFileSystem = NULL;

	delete TheW3DFileSystem;
	TheW3DFileSystem = NULL;

	delete TheNameKeyGenerator;
	TheNameKeyGenerator = NULL;

	shutdownMemoryManager();
	DEBUG_SHUTDOWN();
}

//=============================================================================
// WorldBuilderApp::selectPointerTool
//=============================================================================
/** Sets the active tool to the pointer, and clears the selection. */
//=============================================================================
void WorldBuilderApp::selectPointerTool(void)
{
	setActiveTool(m_pointerTool);
	// Clear selection.
	PointerTool::clearSelection();
}

//=============================================================================
// WorldBuilderApp::findTool
//=============================================================================
Tool *WorldBuilderApp::findTool(Int toolID)
{
	for (Int i=0; i<NUM_VIEW_TOOLS; i++) {
		if (m_tools[i] && m_tools[i]->getToolID() == toolID) {
			return m_tools[i];
		}
	}
	return NULL;
}

//=============================================================================
// WorldBuilderApp::setActiveTool
//=============================================================================
/** Sets the active tool, and activates it after deactivating the current tool. */
//=============================================================================
void WorldBuilderApp::setActiveTool(Tool *pNewTool)
{
	if (m_curTool == pNewTool) {
		// same tool
		return;
	}
	if (m_selTool && m_selTool != pNewTool) {
		m_selTool->deactivate();
	}
	if (pNewTool) {
		pNewTool->activate();
	}
	m_curTool = pNewTool;
	m_selTool = pNewTool;
}

//=============================================================================
// WorldBuilderApp::updateCurTool
//=============================================================================
/** Checks to see if any key modifiers (ctrl or alt) are pressed.  If so,
selectes the appropriate tool, else uses the normal tool. */
//=============================================================================
void WorldBuilderApp::updateCurTool(Bool forceHand)
{
	DEBUG_ASSERTCRASH((m_lockCurTool>=0),("oops"));
	if (!m_lockCurTool) {	 // don't change tools that are doing something.
		Qt::KeyboardModifiers mods = QGuiApplication::queryKeyboardModifiers();
		if (forceHand) {
			// Space bar gives scroll hand.
			m_curTool = m_handScrollTool;
		} else if (mods & Qt::AltModifier) {
			/// @todo alt gives the eyedropper once that tool is ported.
			m_curTool = m_selTool;
		} else if (mods & Qt::ControlModifier) {
			// Control key gives pointer.
			m_curTool = m_pointerTool;
		} else {
			// Else the tool selected in the tool palette.
			m_curTool = m_selTool;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// main

int main(int argc, char **argv)
{
	// The engine's pooled operator new/delete is used from every thread Qt
	// creates, so the allocator locks must be in place before the
	// QApplication (and with it the Qt worker threads) comes up - same as
	// SDL3Main.cpp does for the game.
	static CriticalSection critSec1, critSec2, critSec3, critSec4, critSec5;
	TheAsciiStringCriticalSection = &critSec1;
	TheUnicodeStringCriticalSection = &critSec2;
	TheDmaCriticalSection = &critSec3;
	TheMemoryPoolCriticalSection = &critSec4;
	TheDebugLogCriticalSection = &critSec5;

	WorldBuilderApp app(argc, argv);

	SplashScreen splash;
	splash.show();

	if (!app.initEngine(argc, argv, &splash)) {
		return 1;
	}

	CMainFrame window;
	window.show();
	splash.finish(&window);

	// Test hook: WB_TEST_OPEN=<path> loads a map right after startup.
	if (qEnvironmentVariableIsSet("WB_TEST_OPEN")) {
		app.getDocument()->openDocument(qgetenv("WB_TEST_OPEN").constData());
	}

	int ret = app.exec();

	app.shutdownEngine();
	return ret;
}
