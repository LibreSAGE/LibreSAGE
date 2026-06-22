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

#include "animationtoolbar.h"

#include "w3dviewdoc.h"
#include "graphicview.h"
#include "utils.h"

#include <QAction>
#include <QTimer>
#include <QCloseEvent>

//////////////////////////////////////////////////////////////
//
//  AnimationToolBar
//
AnimationToolBar::AnimationToolBar (QWidget *parent)
	: QToolBar (parent)
{
	setWindowTitle (tr ("Animation"));
	setObjectName ("AnimationToolBar");

	setMovable (true);
	setFloatable (true);
	setAllowedAreas (Qt::BottomToolBarArea);

	//
	//	Transport controls (same order/semantics as the Renegade toolbar)
	//
	m_playAction     = addAction (tr ("▶"));
	m_pauseAction    = addAction (tr ("❚❚"));
	m_stopAction     = addAction (tr ("■"));
	m_stepBackAction = addAction (tr ("|◀"));
	m_stepFwdAction  = addAction (tr ("▶|"));

	m_playAction->setToolTip (tr ("Play"));
	m_pauseAction->setToolTip (tr ("Pause / resume"));
	m_stopAction->setToolTip (tr ("Stop"));
	m_stepBackAction->setToolTip (tr ("Step back one frame"));
	m_stepFwdAction->setToolTip (tr ("Step forward one frame"));

	// Play and Pause are two-state buttons that reflect the current state.
	m_playAction->setCheckable (true);
	m_pauseAction->setCheckable (true);

	//
	//	Connections
	//
	connect (m_playAction,     &QAction::triggered, this, &AnimationToolBar::OnPlay);
	connect (m_pauseAction,    &QAction::triggered, this, &AnimationToolBar::OnPause);
	connect (m_stopAction,     &QAction::triggered, this, &AnimationToolBar::OnStop);
	connect (m_stepBackAction, &QAction::triggered, this, &AnimationToolBar::OnStepBack);
	connect (m_stepFwdAction,  &QAction::triggered, this, &AnimationToolBar::OnStepForward);

	//
	//	Poll so the Play/Pause buttons track playback driven elsewhere
	//	(e.g. auto-play when an animation is selected in the asset tree).
	//
	m_refreshTimer = new QTimer (this);
	m_refreshTimer->setInterval (100);
	connect (m_refreshTimer, &QTimer::timeout, this, &AnimationToolBar::Refresh);
	m_refreshTimer->start ();

	Refresh ();
	return ;
}


//////////////////////////////////////////////////////////////
//
//  ~AnimationToolBar
//
AnimationToolBar::~AnimationToolBar (void)
{
	return ;
}


//////////////////////////////////////////////////////////////
//
//  closeEvent
//
void
AnimationToolBar::closeEvent (QCloseEvent *event)
{
	emit Closed ();
	QToolBar::closeEvent (event);
	return ;
}


//////////////////////////////////////////////////////////////
//
//  Refresh
//
void
AnimationToolBar::Refresh (void)
{
	W3DViewDoc *doc = ::GetCurrentDocument ();
	GraphicView *view = ::Get_Graphic_View ();

	const bool have_anim = (doc != NULL) && (doc->GetCurrentAnimation () != NULL);

	m_playAction->setEnabled (have_anim);
	m_pauseAction->setEnabled (have_anim);
	m_stopAction->setEnabled (have_anim);
	m_stepBackAction->setEnabled (have_anim);
	m_stepFwdAction->setEnabled (have_anim);

	if (view != NULL) {
		const GraphicView::ANIMATION_STATE state = view->GetAnimationState ();
		m_syncing = true;
		m_playAction->setChecked (state == GraphicView::AnimPlaying);
		m_pauseAction->setChecked (state == GraphicView::AnimPaused);
		m_syncing = false;
	}

	return ;
}


//////////////////////////////////////////////////////////////
//
//  OnPlay
//
void
AnimationToolBar::OnPlay (void)
{
	if (m_syncing) {
		return ;
	}

	GraphicView *view = ::Get_Graphic_View ();
	if (view != NULL) {
		view->SetAnimationState (GraphicView::AnimPlaying);
	}

	Refresh ();
	return ;
}


//////////////////////////////////////////////////////////////
//
//  OnPause
//
//  Toggles between playing and paused, matching CMainFrame::OnAniPause.
//
void
AnimationToolBar::OnPause (void)
{
	if (m_syncing) {
		return ;
	}

	GraphicView *view = ::Get_Graphic_View ();
	if (view != NULL) {
		if (view->GetAnimationState () == GraphicView::AnimPlaying) {
			view->SetAnimationState (GraphicView::AnimPaused);
		} else if (view->GetAnimationState () == GraphicView::AnimPaused) {
			view->SetAnimationState (GraphicView::AnimPlaying);
		}
	}

	Refresh ();
	return ;
}


//////////////////////////////////////////////////////////////
//
//  OnStop
//
void
AnimationToolBar::OnStop (void)
{
	if (m_syncing) {
		return ;
	}

	GraphicView *view = ::Get_Graphic_View ();
	if (view != NULL) {
		view->SetAnimationState (GraphicView::AnimStopped);
	}

	// The original toolbar only stopped playback; reset to the first frame as
	// well so "stop" returns the model to a known pose.
	W3DViewDoc *doc = ::GetCurrentDocument ();
	if (doc != NULL) {
		doc->ResetAnimation ();
		doc->StepAnimation (0);
	}

	Refresh ();
	return ;
}


//////////////////////////////////////////////////////////////
//
//  OnStepForward
//
void
AnimationToolBar::OnStepForward (void)
{
	GraphicView *view = ::Get_Graphic_View ();
	if (view != NULL) {
		view->SetAnimationState (GraphicView::AnimPaused);
	}

	W3DViewDoc *doc = ::GetCurrentDocument ();
	if (doc != NULL) {
		doc->StepAnimation (1);
	}

	Refresh ();
	return ;
}


//////////////////////////////////////////////////////////////
//
//  OnStepBack
//
void
AnimationToolBar::OnStepBack (void)
{
	GraphicView *view = ::Get_Graphic_View ();
	if (view != NULL) {
		view->SetAnimationState (GraphicView::AnimPaused);
	}

	W3DViewDoc *doc = ::GetCurrentDocument ();
	if (doc != NULL) {
		doc->StepAnimation (-1);
	}

	Refresh ();
	return ;
}
