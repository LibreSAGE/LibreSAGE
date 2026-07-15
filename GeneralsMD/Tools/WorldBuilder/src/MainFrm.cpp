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

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QCloseEvent>
#include <QDockWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTimer>
#include <QToolBar>

// MainFrm.cpp : implementation of the CMainFrame class (Qt6 port)
//

#include "MainFrm.h"

#include "Common/GlobalData.h"

#include "WorldBuilder.h"
#include "WorldBuilderDoc.h"
#include "brushoptions.h"
#include "MoundOptions.h"
#include "FeatherOptions.h"
#include "mapobjectprops.h"
#include "MapSettings.h"
#include "ObjectOptions.h"
#include "DrawObject.h"
#include "wbview3d.h"



CMainFrame *CMainFrame::TheMainFrame = NULL;

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame(QWidget *parent) :
	QMainWindow(parent),
	m_optionsDock(NULL),
	m_optionsStack(NULL),
	m_curOptions(NULL),
	m_noOptions(NULL),
	m_brushOptions(NULL),
	m_moundOptions(NULL),
	m_featherOptions(NULL),
	m_mapObjectProps(NULL),
	m_objectOptions(NULL),
	m_3dView(NULL),
	m_autoSaveTimer(NULL),
	m_autoSaving(false)
{
	TheMainFrame = this;

	setWindowTitle("WorldBuilder");

	createMenus();
	createToolBar();

	// Options panels dock on the right; the individual tool panels register
	// into m_optionsStack as they get ported.
	m_optionsDock = new QDockWidget("Options", this);
	m_optionsDock->setObjectName("OptionsDock");
	m_optionsStack = new QStackedWidget(m_optionsDock);
	m_noOptions = new QLabel("No options for this tool.", m_optionsStack);
	m_optionsStack->addWidget(m_noOptions);
	m_brushOptions = new BrushOptions(m_optionsStack);
	m_optionsStack->addWidget(m_brushOptions);
	m_moundOptions = new MoundOptions(m_optionsStack);
	m_optionsStack->addWidget(m_moundOptions);
	m_featherOptions = new FeatherOptions(m_optionsStack);
	m_optionsStack->addWidget(m_featherOptions);
	m_mapObjectProps = new MapObjectProps(m_optionsStack);
	m_optionsStack->addWidget(m_mapObjectProps);
	m_objectOptions = new ObjectOptions(m_optionsStack);
	m_optionsStack->addWidget(m_objectOptions);
	m_optionsDock->setWidget(m_optionsStack);
	// The original MFC options panel is a free-floating tool window: it may float
	// anywhere but must not dock into the main window.  Disallowing every dock
	// area (plus keeping only the Floatable feature) prevents re-docking, and
	// setFloating() detaches it into its own window.
	m_optionsDock->setAllowedAreas(Qt::NoDockWidgetArea);
	m_optionsDock->setFeatures(QDockWidget::DockWidgetFloatable);
	addDockWidget(Qt::RightDockWidgetArea, m_optionsDock);
	m_optionsDock->setFloating(true);
	// Keep the dock narrow like the original floating options panels; the 3d
	// view gets all remaining space.
	m_optionsStack->setMinimumWidth(220);
	resizeDocks({m_optionsDock}, {260}, Qt::Horizontal);

	// The 3d view is the central widget, attached to the (single) document.
	m_3dView = new WbView3d(this);
	m_3dView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_3dView->setMinimumSize(320, 240);
	setCentralWidget(m_3dView);
	if (WbApp()->getDocument()) {
		WbApp()->getDocument()->attach3DView(m_3dView);
	}

	statusBar()->showMessage("Ready");

	// Default: room for the 800x600 3d view plus the options dock; the saved
	// geometry from the previous session wins when there is one.
	resize(THREE_D_VIEW_WIDTH + 400, THREE_D_VIEW_HEIGHT + 100);
	QSettings settings;
	if (settings.contains(MAIN_FRAME_SECTION "/State2")) {
		restoreGeometry(settings.value(MAIN_FRAME_SECTION "/Geometry").toByteArray());
		restoreState(settings.value(MAIN_FRAME_SECTION "/State2").toByteArray());
	}
	// A saved state from a run where the dock could be closed/floated must not
	// leave it hidden now that it is a fixed tool window.
	m_optionsDock->setVisible(true);

	m_autoSave = settings.value(MAIN_FRAME_SECTION "/AutoSave", true).toBool();
	m_autoSaveInterval = settings.value(MAIN_FRAME_SECTION "/AutoSaveIntervalSeconds", 120).toInt();

	m_autoSaveTimer = new QTimer(this);
	connect(m_autoSaveTimer, &QTimer::timeout, this, &CMainFrame::OnAutoSaveTimer);
	m_autoSaveTimer->start(m_autoSaveInterval * 1000);

	// Restore the brush-feedback enable state from the saved setting.
	if (settings.value(MAIN_FRAME_SECTION "/ShowBrushFeedback", true).toBool())
		DrawObject::enableFeedback();
	else
		DrawObject::disableFeedback();

	// Start with a fresh default map (the original did this via the MFC
	// document template on startup).
	if (WbApp()->getDocument()) {
		WbApp()->getDocument()->newDocument(100, 100, 16, 30);
	}
}

CMainFrame::~CMainFrame()
{
	QSettings settings;
	settings.setValue(MAIN_FRAME_SECTION "/AutoSave", (bool)m_autoSave);
	settings.setValue(MAIN_FRAME_SECTION "/AutoSaveIntervalSeconds", (int)m_autoSaveInterval);

	TheMainFrame = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// Menu / toolbar construction (port of the IDR_MAPDOC menu resource)

// Menu entries whose backing subsystem has not been ported yet are created
// through this helper: visible, but disabled until their feature returns.
static QAction *addStubAction(QMenu *menu, const QString &text,
							  const QKeySequence &shortcut = QKeySequence())
{
	QAction *action = menu->addAction(text);
	action->setShortcut(shortcut);
	action->setEnabled(false);
	return action;
}

void CMainFrame::createMenus(void)
{
	QMenuBar *bar = menuBar();

	// --- File ---------------------------------------------------------------
	QMenu *fileMenu = bar->addMenu("&File");
	fileMenu->addAction("&New", QKeySequence::New, this, &CMainFrame::OnFileNew);
	fileMenu->addAction("&Open...", QKeySequence::Open, this, &CMainFrame::OnFileOpen);
	fileMenu->addAction("&Save", QKeySequence::Save, this, &CMainFrame::OnFileSave);
	fileMenu->addAction("Save &As...", QKeySequence::SaveAs, this, &CMainFrame::OnFileSaveAs);
	addStubAction(fileMenu, "&Jump to Game", QKeySequence("Ctrl+J"));
	fileMenu->addSeparator();
	addStubAction(fileMenu, "Resize...");
	addStubAction(fileMenu, "Dump Map To File");
	fileMenu->addSeparator();
	QAction *exitAction = fileMenu->addAction("E&xit", QKeySequence::Quit,
											  qApp, &QApplication::closeAllWindows);
	Q_UNUSED(exitAction);

	// --- Edit ---------------------------------------------------------------
	QMenu *editMenu = bar->addMenu("&Edit");
	editMenu->addAction("&Undo", QKeySequence::Undo, this, &CMainFrame::OnEditUndo);
	editMenu->addAction("&Redo", QKeySequence("Ctrl+Shift+Z"), this, &CMainFrame::OnEditRedo);
	editMenu->addSeparator();
	addStubAction(editMenu, "Cu&t", QKeySequence::Cut);
	addStubAction(editMenu, "&Copy", QKeySequence::Copy);
	addStubAction(editMenu, "&Paste", QKeySequence::Paste);
	editMenu->addSeparator();
	addStubAction(editMenu, "Delete", QKeySequence::Delete);
	editMenu->addSeparator();
	addStubAction(editMenu, "Select Duplicate Objects");
	addStubAction(editMenu, "Select Objects w/bad teams");
	addStubAction(editMenu, "Select Si&milar", QKeySequence("Ctrl+M"));
	addStubAction(editMenu, "Select Macrotexture...");
	addStubAction(editMenu, "Replace Selected...");
	QMenu *pickMenu = editMenu->addMenu("Pick Constraint");
	static const char *pickNames[] = {
		"Anything", "Buildings", "Infantry", "Vehicles", "Shrubbery",
		"Props", "Natural", "Debris", "Waypoints && Areas", "Roads", "Sounds"};
	for (size_t i = 0; i < sizeof(pickNames)/sizeof(pickNames[0]); ++i) {
		QKeySequence shortcut = (i <= 9) ? QKeySequence(QString("Ctrl+%1").arg(i)) : QKeySequence();
		addStubAction(pickMenu, pickNames[i], shortcut);
	}
	editMenu->addSeparator();
	QAction *scriptsAction = editMenu->addAction("Scripts...");
	connect(scriptsAction, &QAction::triggered, this, &CMainFrame::onEditScripts);
	editMenu->addSeparator();
	addStubAction(editMenu, "Global Light Options...");
	addStubAction(editMenu, "Camera Options...");
	addStubAction(editMenu, "Edit Shadows...");
	editMenu->addAction("Edit Map Settings...", this, &CMainFrame::OnEditMapSettings);
	editMenu->addSeparator();
	addStubAction(editMenu, "Edit Teams...");
	addStubAction(editMenu, "Edit Player List...");

	// --- View ---------------------------------------------------------------
	QMenu *viewMenu = bar->addMenu("&View");

	// Adds a checkable View item bound to a WbView3d overlay flag.  The 3d view
	// is created after the menus, so the lambda reads/writes the flag lazily and
	// persists it under the same QSettings key the view restores from.
	auto addViewToggle = [this, viewMenu](const QString &text, const QString &settingsKey,
										  void (WbView3d::*setter)(Bool), bool def,
										  const QKeySequence &shortcut = QKeySequence()) {
		QAction *action = viewMenu->addAction(text);
		if (!shortcut.isEmpty()) action->setShortcut(shortcut);
		action->setCheckable(true);
		QSettings settings;
		action->setChecked(settings.value(QString(MAIN_FRAME_SECTION "/") + settingsKey, def).toBool());
		connect(action, &QAction::triggered, this, [this, setter, settingsKey](bool checked) {
			if (m_3dView) (m_3dView->*setter)(checked);
			QSettings s;
			s.setValue(QString(MAIN_FRAME_SECTION "/") + settingsKey, checked);
		});
	};

	addStubAction(viewMenu, "Show Grid", QKeySequence("Ctrl+G"));
	addStubAction(viewMenu, "Show Texture", QKeySequence("Ctrl+T"));
	viewMenu->addSeparator();
	addStubAction(viewMenu, "Show Terrain");
	addViewToggle("Show Object Icons", "ShowObjectIcons", &WbView3d::setShowObjects, true, QKeySequence("Ctrl+B"));
	addViewToggle("Show Waypoints", "ShowWaypoints", &WbView3d::setShowWaypoints, true);
	addStubAction(viewMenu, "Show Trigger Areas");
	addStubAction(viewMenu, "Show Shadows");
	addStubAction(viewMenu, "Show Labels");
	addStubAction(viewMenu, "Show Objects");
	addViewToggle("Show Bounding Boxes", "ShowBoundingBoxes", &WbView3d::setShowBoundingBoxes, false);
	addViewToggle("Show Sight Ranges", "ShowSightRanges", &WbView3d::setShowSightRanges, false);
	addViewToggle("Show Weapon Ranges", "ShowWeaponRanges", &WbView3d::setShowWeaponRanges, false);
	addStubAction(viewMenu, "Show Garrisoned");
	addStubAction(viewMenu, "Show Map Boundaries");
	addViewToggle("Show Letterbox", "ShowLetterbox", &WbView3d::setShowLetterbox, false);
	addStubAction(viewMenu, "Show Sound Flags");
	addViewToggle("Show Sound Circles", "ShowSoundCircles", &WbView3d::setShowSoundCircles, false);
	viewMenu->addSeparator();
	addViewToggle("Highlight Test Art", "HighlightTestArt", &WbView3d::setHighlightTestArt, false);
	viewMenu->addSeparator();
	addStubAction(viewMenu, "Show Impassable Areas", QKeySequence("Ctrl+I"));
	addStubAction(viewMenu, "Impassable Area Options...");
	viewMenu->addSeparator();
	QAction *entireMapAction = viewMenu->addAction("Show All of 3d Map");
	entireMapAction->setShortcut(QKeySequence("Ctrl+A"));
	entireMapAction->setCheckable(true);
	connect(entireMapAction, &QAction::triggered, this, [this](bool checked) {
		m_3dView->setShowEntireMap(checked);
	});
	QMenu *partialMenu = viewMenu->addMenu("Partial Map Size");
	addStubAction(partialMenu, "96x96 (Standard game size)");
	addStubAction(partialMenu, "128x128");
	addStubAction(partialMenu, "160x160");
	addStubAction(partialMenu, "192x192");
	viewMenu->addSeparator();
	QAction *wireframeAction = viewMenu->addAction("Show Wireframe 3D View");
	wireframeAction->setShortcut(QKeySequence("Ctrl+W"));
	wireframeAction->setCheckable(true);
	connect(wireframeAction, &QAction::triggered, this, [this](bool checked) {
		m_3dView->setShowWireframe(checked);
	});
	QAction *topDownAction = viewMenu->addAction("Show From Top Down View");
	topDownAction->setShortcut(QKeySequence("Ctrl+F"));
	topDownAction->setCheckable(true);
	connect(topDownAction, &QAction::triggered, this, [this](bool checked) {
		m_3dView->setShowTopDownView(checked);
	});
	addStubAction(viewMenu, "Show 3-Way Blends in White");
	addStubAction(viewMenu, "Show Clouds", QKeySequence("Ctrl+U"));
	addStubAction(viewMenu, "Show Soft Water");
	addStubAction(viewMenu, "Show Macrotexture");
	viewMenu->addSeparator();
	viewMenu->addAction("Change Time Of Day", QKeySequence("Ctrl+D"), this, [this]() {
		m_3dView->stepTimeOfDay();
	});
	viewMenu->addSeparator();
	viewMenu->addAction("Zoom In", QKeySequence::ZoomIn, this, [this]() {
		m_3dView->zoomIn();
	});
	viewMenu->addAction("Zoom Out", QKeySequence::ZoomOut, this, [this]() {
		m_3dView->zoomOut();
	});
	viewMenu->addAction("Reset Zoom", QKeySequence("Ctrl+0"), this, [this]() {
		m_3dView->zoomReset();
	});
	viewMenu->addSeparator();
	addStubAction(viewMenu, "Snap To Grid", QKeySequence("Ctrl+Shift+G"));
	viewMenu->addSeparator();
	QAction *brushFeedback = viewMenu->addAction("Show Brush Feedback");
	brushFeedback->setCheckable(true);
	brushFeedback->setChecked(QSettings().value(MAIN_FRAME_SECTION "/ShowBrushFeedback", true).toBool());
	connect(brushFeedback, &QAction::triggered, this, &CMainFrame::OnViewBrushfeedback);
	viewMenu->addSeparator();
	addStubAction(viewMenu, "Reload Textures");
	viewMenu->addSeparator();
	QAction *layersAction = viewMenu->addAction("Layers List");
	layersAction->setCheckable(true);
	layersAction->setEnabled(false); /// @todo enable once the layers panel is ported.

	// --- Window -------------------------------------------------------------
	QMenu *windowMenu = bar->addMenu("&Window");
	addStubAction(windowMenu, "640x480");
	addStubAction(windowMenu, "800x600");
	addStubAction(windowMenu, "1024x768");
	windowMenu->addSeparator();
	windowMenu->addAction("Reset Window Positions", this, &CMainFrame::ResetWindowPositions);

	// --- Texture Sizing -----------------------------------------------------
	QMenu *textureMenu = bar->addMenu("Texture Sizing");
	addStubAction(textureMenu, "Map Cliff Textures");
	addStubAction(textureMenu, "Remove Cliff Tex Mapping");
	textureMenu->addSeparator();
	addStubAction(textureMenu, "Optimize tiles and blend tiles.");
	textureMenu->addSeparator();
	addStubAction(textureMenu, "Remap Textures...");
	textureMenu->addSeparator();
	addStubAction(textureMenu, "Texture Sizing Info...");
	textureMenu->addSeparator();
	addStubAction(textureMenu, "Tile 4x4");
	addStubAction(textureMenu, "Tile 6x6");
	addStubAction(textureMenu, "Tile 8x8");

	// --- Validation ---------------------------------------------------------
	QMenu *validationMenu = bar->addMenu("Validation");
	addStubAction(validationMenu, "Generate Report");
	addStubAction(validationMenu, "Fix Teams");

	// --- Help ---------------------------------------------------------------
	QMenu *helpMenu = bar->addMenu("&Help");
	helpMenu->addAction("&About World Builder...", this, [this]() {
		QMessageBox::about(this, "About World Builder",
						   "World Builder\nMap editor for Command & Conquer Generals Zero Hour");
	});
}

void CMainFrame::createToolBar(void)
{
	QToolBar *toolBar = addToolBar("Main");
	toolBar->setObjectName("MainToolBar");

	// Tool palette: one checkable action per ported tool.
	/// @todo add the remaining tools as they get ported (original
	/// IDR_MAINFRAME toolbar).
	QActionGroup *toolGroup = new QActionGroup(this);
	struct { const char *name; Int id; } toolActions[] = {
		{ "Pointer", ID_POINTER_TOOL },
		{ "Hand Scroll", ID_HAND_SCROLL_TOOL },
		{ "Place Object", ID_OBJECT_TOOL },
		{ "Height Brush", ID_BRUSH_TOOL },
		{ "Mound", ID_MOUND_TOOL },
		{ "Dig", ID_DIG_TOOL },
		{ "Smooth", ID_FEATHER_TOOL },
	};
	for (size_t i = 0; i < sizeof(toolActions)/sizeof(toolActions[0]); ++i) {
		QAction *action = toolBar->addAction(toolActions[i].name);
		action->setCheckable(true);
		Int toolId = toolActions[i].id;
		connect(action, &QAction::triggered, this, [toolId]() {
			Tool *tool = WbApp()->findTool(toolId);
			if (tool) {
				WbApp()->setActiveTool(tool);
			}
		});
		toolGroup->addAction(action);
		if (toolId == ID_POINTER_TOOL) {
			action->setChecked(true);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame operations

void CMainFrame::SetMessageText(const char *text)
{
	statusBar()->showMessage(QString::fromUtf8(text));
}

void CMainFrame::ResetWindowPositions(void)
{
	move(20, 20);
	resize(THREE_D_VIEW_WIDTH, THREE_D_VIEW_HEIGHT);
	show();
	/// @todo reset the options panel / floating toolbar positions once they
	/// are ported.
}

void CMainFrame::showOptionsDialog(Int panelId)
{
	QWidget *newOptions = NULL;
	switch (panelId) {
		case ID_BRUSH_TOOL: newOptions = m_brushOptions; break;
		case ID_MOUND_TOOL:
		case ID_DIG_TOOL: newOptions = m_moundOptions; break;
		case ID_FEATHER_TOOL: newOptions = m_featherOptions; break;
		case ID_POINTER_TOOL: newOptions = m_mapObjectProps; break;
		case ID_OBJECT_TOOL: newOptions = m_objectOptions; break;
		/// @todo route the remaining panels here as they get ported.
		default: newOptions = m_noOptions; break;
	}
	if (m_optionsStack && newOptions) {
		m_optionsStack->setCurrentWidget(newOptions);
	}
}

void CMainFrame::OnEditGloballightoptions()
{
	/// @todo show the GlobalLightOptions panel once it is ported.
}

void CMainFrame::OnEditMapSettings()
{
	MapSettings dlg(this);
	dlg.exec();
}

void CMainFrame::onEditScripts()
{
	/// @todo show the ScriptDialog once it is ported.
	SetMessageText("The script editor has not been ported to Qt yet.");
}

void CMainFrame::handleCameraChange(void)
{
	/// @todo update the CameraOptions panel once it is ported.
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

void CMainFrame::OnViewBrushfeedback()
{
	// The checkable action has already toggled; flip DrawObject to match and
	// persist the new state.
	bool enabled = !DrawObject::isFeedbackEnabled();
	if (enabled)
		DrawObject::enableFeedback();
	else
		DrawObject::disableFeedback();
	QSettings().setValue(MAIN_FRAME_SECTION "/ShowBrushFeedback", enabled);
}

void CMainFrame::OnAutoSaveTimer()
{
	CWorldBuilderDoc *pDoc = CWorldBuilderDoc::GetActiveDoc();
	if (pDoc && pDoc->needAutoSave()) {
		m_autoSaving = true;
		SetMessageText("Auto Saving map...");
		pDoc->autoSave();
		SetMessageText("Auto Save Complete.");
		m_autoSaving = false;
	}
}

/////////////////////////////////////////////////////////////////////////////
// File & edit menu handlers

void CMainFrame::OnFileNew()
{
	CWorldBuilderDoc *pDoc = CWorldBuilderDoc::GetActiveDoc();
	if (pDoc) {
		/// @todo port the CNewHeightMap size dialog; using the saved defaults.
		QSettings settings;
		Int height = settings.value("GameOptions/DefaultMapHeight", 16).toInt();
		Int xSize = settings.value("GameOptions/DefaultMapXSize", 100).toInt();
		Int ySize = settings.value("GameOptions/DefaultMapYSize", 100).toInt();
		Int border = settings.value("GameOptions/DefaultMapBorder", 30).toInt();
		pDoc->newDocument(xSize, ySize, height, border);
		setWindowTitle("WorldBuilder - Untitled");
	}
}

void CMainFrame::OnFileOpen()
{
	CWorldBuilderDoc *pDoc = CWorldBuilderDoc::GetActiveDoc();
	if (pDoc == NULL) {
		return;
	}
	QString dir = QString::fromUtf8(WbApp()->getCurrentDirectory().str());
	QString path = QFileDialog::getOpenFileName(this, "Open Map", dir, "Maps (*.map)");
	if (path.isEmpty()) {
		return;
	}
	WbApp()->setCurrentDirectory(AsciiString(QFileInfo(path).absolutePath().toUtf8().constData()));
	if (pDoc->openDocument(path.toUtf8().constData())) {
		setWindowTitle(QString("WorldBuilder - %1").arg(QFileInfo(path).fileName()));
	}
}

void CMainFrame::OnFileSave()
{
	CWorldBuilderDoc *pDoc = CWorldBuilderDoc::GetActiveDoc();
	if (pDoc == NULL) {
		return;
	}
	if (pDoc->getFilePath().isEmpty()) {
		OnFileSaveAs();
		return;
	}
	pDoc->saveDocument(pDoc->getFilePath().str());
}

void CMainFrame::OnFileSaveAs()
{
	CWorldBuilderDoc *pDoc = CWorldBuilderDoc::GetActiveDoc();
	if (pDoc == NULL) {
		return;
	}
	QString dir = QString::fromUtf8(WbApp()->getCurrentDirectory().str());
	QString path = QFileDialog::getSaveFileName(this, "Save Map", dir, "Maps (*.map)");
	if (path.isEmpty()) {
		return;
	}
	if (!path.endsWith(".map", Qt::CaseInsensitive)) {
		path += ".map";
	}
	if (pDoc->saveDocument(path.toUtf8().constData())) {
		setWindowTitle(QString("WorldBuilder - %1").arg(QFileInfo(path).fileName()));
	}
}

void CMainFrame::OnEditUndo()
{
	CWorldBuilderDoc *pDoc = CWorldBuilderDoc::GetActiveDoc();
	if (pDoc) {
		pDoc->OnEditUndo();
	}
}

void CMainFrame::OnEditRedo()
{
	CWorldBuilderDoc *pDoc = CWorldBuilderDoc::GetActiveDoc();
	if (pDoc) {
		pDoc->OnEditRedo();
	}
}

void CMainFrame::closeEvent(QCloseEvent *event)
{
	QSettings settings;
	settings.setValue(MAIN_FRAME_SECTION "/Geometry", saveGeometry());
	settings.setValue(MAIN_FRAME_SECTION "/State2", saveState());
	QMainWindow::closeEvent(event);
}
