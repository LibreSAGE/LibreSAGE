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

// MainFrm.h : interface of the CMainFrame class (Qt6 port of the MFC frame)
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QMainWindow>

#include "Lib/BaseType.h"

/// @todo port the options panels (BrushOptions, TerrainMaterial, ...) as Qt
/// widgets and re-add them to the options dock.

#define TWO_D_WINDOW_SECTION "TwoDWindow"
#define MAIN_FRAME_SECTION "MainFrame"

class QDockWidget;
class QStackedWidget;
class QTimer;
class WbView3d;
class BrushOptions;
class MoundOptions;
class FeatherOptions;
class MapObjectProps;

class CMainFrame : public QMainWindow
{
	Q_OBJECT

public:
	CMainFrame(QWidget *parent = NULL);
	~CMainFrame() override;

	static CMainFrame *GetMainFrame() { return TheMainFrame; }

	/// Switch the options dock to the panel a tool requested (by ToolID).
	void showOptionsDialog(Int panelId);

	WbView3d *get3DView(void) {return m_3dView;}

	void OnEditGloballightoptions();
	void ResetWindowPositions(void);
	Bool isAutoSaving(void) {return m_autoSaving;};
	void handleCameraChange(void);
	void onEditScripts();

	void SetMessageText(const char *text);

protected:
	void closeEvent(QCloseEvent *event) override;

private slots:
	void OnViewBrushfeedback();
	void OnAutoSaveTimer();
	void OnFileNew();
	void OnFileOpen();
	void OnFileSave();
	void OnFileSaveAs();
	void OnEditUndo();
	void OnEditRedo();

private:
	void createMenus(void);
	void createToolBar(void);

protected:
	QDockWidget				*m_optionsDock;
	QStackedWidget			*m_optionsStack;
	QWidget					*m_curOptions;
	QWidget					*m_noOptions;
	BrushOptions			*m_brushOptions;
	MoundOptions			*m_moundOptions;
	FeatherOptions			*m_featherOptions;
	MapObjectProps			*m_mapObjectProps;
	WbView3d				*m_3dView;

	QTimer					*m_autoSaveTimer;  ///< Timer that triggers for autosave.
	Bool					m_autoSaving;  ///< True if we are autosaving.
	Bool					m_autoSave;    ///< If true, then do autosaves.
	Int						m_autoSaveInterval;  ///< Time between autosaves in seconds.

	static CMainFrame *TheMainFrame;
};
