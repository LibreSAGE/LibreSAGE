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

// WorldBuilder.h : main header file for the WORLDBUILDER application
//

#pragma once

#include <QApplication>

#include "Lib/BaseType.h"
#include "Common/AsciiString.h"
#include "Common/STLTypedefs.h"
#include "Common/Debug.h"

class BrushTool;
class CWorldBuilderDoc;
class EyedropperTool;
class HandScrollTool;
class MapObject;
class PointerTool;
class SplashScreen;
class Tool;

/////////////////////////////////////////////////////////////////////////////
// WorldBuilderApp:
// See WorldBuilder.cpp for the implementation of this class
//

// Force maps into a directory structure.
#define DO_MAPS_IN_DIRECTORIES 1

#define NONE_STRING "<none>"


enum {THREE_D_VIEW_WIDTH=800, THREE_D_VIEW_HEIGHT=600};
enum {MAX_OBJECTS_IN_MAP = 3000};

class WorldBuilderApp : public QApplication
{
	Q_OBJECT

public:
	WorldBuilderApp(int &argc, char **argv);
	~WorldBuilderApp() override;

	/// Brings up the game-engine subsystems the editor depends on.
	/// Must run before any document or view is created.  argc/argv are the
	/// raw program arguments; the engine command line (-dir, -bigdir, ...) is
	/// parsed by the same code the game uses (CommandLine.cpp).  When a
	/// splash screen is given, progress messages are shown on it.
	bool initEngine(int argc, char **argv, SplashScreen *splash = NULL);
	void shutdownEngine();

protected:

	enum {NUM_VIEW_TOOLS=25};

	Tool							*m_tools[NUM_VIEW_TOOLS]; ///< array of tool pointers.
	Tool							*m_curTool;   ///< Currently active tool.
	Tool							*m_selTool;   ///< Normal tool.  If we hit alt, curTool turns to eyedropper.
	BrushTool					*m_brushTool;				///< Height brush tool.
	PointerTool				*m_pointerTool;			///< Select and move/rotate tool.
	HandScrollTool		*m_handScrollTool;	///< Scroll tool.
	EyedropperTool		*m_eyedropperTool;	///< Eyedropper tool (activated by holding alt).

	Int								m_lockCurTool;

	AsciiString				m_currentDirectory; ///< Current directory for open file.

	CWorldBuilderDoc	*m_document;				///< The (single) open document.

	MapObject					*m_pasteMapObjList;	///< List of copied/cut map objects.

	bool							m_engineInited;

protected:
	void deletePasteObjList(void);

public:

	CWorldBuilderDoc *getDocument(void) { return m_document; }

	/// Set the pointer tool as the active tool.
	void selectPointerTool(void);

	/// Look up a tool by its id (returns NULL when not (yet) ported).
	Tool *findTool(Int toolID);

	/// Set the tool that will be active.
	void setActiveTool(Tool *newTool);

	/// Sets the current directry for file opens.
	void setCurrentDirectory(AsciiString dir) {m_currentDirectory = dir;};
	AsciiString getCurrentDirectory(void) {return m_currentDirectory;};

	Tool *getCurTool() { return m_curTool; }

	/// Check to see if any keyboard overrides are changing the current tool.
	void updateCurTool(Bool forceHand);

	void lockCurTool()		{ DEBUG_ASSERTCRASH(!m_lockCurTool,("already locked")); m_lockCurTool = 1; }
	void unlockCurTool()	{ m_lockCurTool = 0; }
	Bool isCurToolLocked()	{ return m_lockCurTool != 0; }

	/// Note - read only data - make yourself a copy.
	MapObject *getMapObjPasteList(void) { return(m_pasteMapObjList);};

	/// Note - the app owns this, and will delete it on close.
	void setMapObjPasteList(MapObject *list) { deletePasteObjList(); m_pasteMapObjList = list; };
};

inline WorldBuilderApp *WbApp() { return static_cast<WorldBuilderApp*>(QApplication::instance()); }
