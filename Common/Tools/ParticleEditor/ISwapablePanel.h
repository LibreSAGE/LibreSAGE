/*
**	Command & Conquer Generals(tm)
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

// FILE: ISwapablePanel.h ///////////////////////////////////////////////////////
//
// Desc:       Swapable panels derive from this so that we can easily call the
//             update function.  The MFC original derived from CDialog; the Qt6
//             port makes these plain QWidgets that live inside a QStackedWidget.
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QWidget>

class DebugWindowDialog;

class ISwapablePanel : public QWidget
{
	Q_OBJECT

public:
	explicit ISwapablePanel( DebugWindowDialog *parentDialog );
	~ISwapablePanel() override = default;

	// One-time set up (populate combos, hook up signals, ...).
	virtual void InitPanel( void ) = 0;

	// if true, updates the UI from the Particle System.
	// if false, updates the Particle System from the UI
	virtual void performUpdate( bool toUI ) = 0;

public slots:
	// Notify the owning dialog that the user edited a value on this panel.
	void onParticleSystemEdit();

protected:
	DebugWindowDialog *m_dialog;
};
