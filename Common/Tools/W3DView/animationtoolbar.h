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

//
// AnimationToolBar: transport controls for the displayed animation, the Qt
// equivalent of CMainFrame::m_animationToolbar in the original Renegade W3DView
// (Play / Pause / Stop / Step-Back / Step-Forward only). It drives
// W3DViewDoc / GraphicView and polls them so the Play/Pause buttons track
// playback (including auto-play from the asset tree). Frame/speed read-outs
// live on the status bar, matching the original.
//
#pragma once

#include <QToolBar>

class QAction;
class QTimer;

class AnimationToolBar : public QToolBar
{
	Q_OBJECT

public:
	explicit AnimationToolBar (QWidget *parent = nullptr);
	~AnimationToolBar () override;

signals:
	// Emitted when the user closes the floating window, so the owning window
	// can keep its "View > Toolbars > Animation" check state in sync.
	void Closed ();

protected:
	void closeEvent (QCloseEvent *event) override;

private slots:
	void OnPlay ();
	void OnPause ();
	void OnStop ();
	void OnStepForward ();
	void OnStepBack ();
	void Refresh ();			// periodic sync of Play/Pause button state

private:
	QAction *m_playAction     = nullptr;
	QAction *m_pauseAction    = nullptr;
	QAction *m_stopAction     = nullptr;
	QAction *m_stepBackAction = nullptr;
	QAction *m_stepFwdAction  = nullptr;
	QTimer  *m_refreshTimer   = nullptr;

	bool     m_syncing = false;	// guard against feedback while updating controls
};
