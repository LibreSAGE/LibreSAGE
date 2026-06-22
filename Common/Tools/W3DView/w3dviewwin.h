/*
**	Command & Conquer Renegade(tm)
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

#pragma once
#include "datatreemodel.h"

#include <QMainWindow>
#include <QItemSelection>

class QLabel;
class QTimer;
class QAction;

namespace Ui
{
class W3DViewWindow;
}

class W3DViewDoc;
class AnimationToolBar;
class W3DViewWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit W3DViewWindow(QWidget *parent = nullptr);
    ~W3DViewWindow() override;

private slots:
    void OnFileOpen();
    void OnOpenRecentFile();
    void OnAbout();
    void OnExit();
    void OnTreeSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

public:

	//////////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////////
	void	Update_Frame_Time (DWORD milliseconds);
	void	UpdatePolygonCount (int iPolygons);
	void	Update_Particle_Count (int particles);
	void	UpdateCameraDistance (float cameraDistance);
	void	UpdateFrameCount (int iCurrentFrame, int iTotalFrames, float frame_rate);

	void	Select_Device (bool show_dlg = true);

private:
    void InitializeRenderer();
    void CreateStatusBar();

    //
    //	Recently-used file list (the Qt equivalent of Renegade's MFC MRU:
    //	LoadStdProfileSettings + AddToRecentFileList; mirrors the WDump port).
    //
    void LoadFile(const QString &fileName);
    void AddToRecentFileList(const QString &fileName);
    void UpdateRecentFileListActions();

    static constexpr int MaxRecentFiles = 10;

    Ui::W3DViewWindow *m_ui = NULL;
	W3DViewDoc *m_doc = NULL;
	DataTreeModel m_dataTreeModel;
    bool m_rendererInitialized = false;
	AnimationToolBar *m_animationToolBar = nullptr;
	QAction *m_recentFileActions[MaxRecentFiles] = {};
	QAction *m_recentFileSeparator = nullptr;

	// Status-bar panes (mirror Renegade's m_wndStatusBar)
	QLabel *m_statusPolys = nullptr;
	QLabel *m_statusParticles = nullptr;
	QLabel *m_statusCamera = nullptr;
	QLabel *m_statusFrame = nullptr;
	QLabel *m_statusFps = nullptr;
	QLabel *m_statusResolution = nullptr;
	QTimer *m_statusTimer = nullptr;
	unsigned int m_lastRenderFrameCount = 0;
};
