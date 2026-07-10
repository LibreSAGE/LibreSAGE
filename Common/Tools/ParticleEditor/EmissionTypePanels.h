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

// FILE: EmissionTypePanels.h ///////////////////////////////////////////////////
// Desc:       Emission panels are pretty similar, they all go here.
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ISwapablePanel.h"

namespace Ui
{
	class EmissionPanelPoint;
	class EmissionPanelLine;
	class EmissionPanelBox;
	class EmissionPanelSphere;
	class EmissionPanelCylinder;
}

// EmissionPanelPoint ///////////////////////////////////////////////////////////
class EmissionPanelPoint : public ISwapablePanel
{
	Q_OBJECT
public:
	explicit EmissionPanelPoint( DebugWindowDialog *parentDialog );
	~EmissionPanelPoint() override;
	void InitPanel( void ) override;
	void performUpdate( bool toUI ) override;
private:
	Ui::EmissionPanelPoint *m_ui;
};

// EmissionPanelLine ////////////////////////////////////////////////////////////
class EmissionPanelLine : public ISwapablePanel
{
	Q_OBJECT
public:
	explicit EmissionPanelLine( DebugWindowDialog *parentDialog );
	~EmissionPanelLine() override;
	void InitPanel( void ) override;
	void performUpdate( bool toUI ) override;
private:
	Ui::EmissionPanelLine *m_ui;
};

// EmissionPanelBox /////////////////////////////////////////////////////////////
class EmissionPanelBox : public ISwapablePanel
{
	Q_OBJECT
public:
	explicit EmissionPanelBox( DebugWindowDialog *parentDialog );
	~EmissionPanelBox() override;
	void InitPanel( void ) override;
	void performUpdate( bool toUI ) override;
private:
	Ui::EmissionPanelBox *m_ui;
};

// EmissionPanelSphere //////////////////////////////////////////////////////////
class EmissionPanelSphere : public ISwapablePanel
{
	Q_OBJECT
public:
	explicit EmissionPanelSphere( DebugWindowDialog *parentDialog );
	~EmissionPanelSphere() override;
	void InitPanel( void ) override;
	void performUpdate( bool toUI ) override;
private:
	Ui::EmissionPanelSphere *m_ui;
};

// EmissionPanelCylinder ////////////////////////////////////////////////////////
class EmissionPanelCylinder : public ISwapablePanel
{
	Q_OBJECT
public:
	explicit EmissionPanelCylinder( DebugWindowDialog *parentDialog );
	~EmissionPanelCylinder() override;
	void InitPanel( void ) override;
	void performUpdate( bool toUI ) override;
private:
	Ui::EmissionPanelCylinder *m_ui;
};
