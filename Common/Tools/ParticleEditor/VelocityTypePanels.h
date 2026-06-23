/*
**	Command & Conquer Generals(tm)
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

// FILE: VelocityTypePanels.h ///////////////////////////////////////////////////
// Desc:       Velocity panels are pretty similar, they all go here.
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ISwapablePanel.h"

namespace Ui
{
	class VelocityPanelOrtho;
	class VelocityPanelSphere;
	class VelocityPanelHemisphere;
	class VelocityPanelCylinder;
	class VelocityPanelOutward;
}

// VelocityPanelOrtho ///////////////////////////////////////////////////////////
class VelocityPanelOrtho : public ISwapablePanel
{
	Q_OBJECT
public:
	explicit VelocityPanelOrtho( DebugWindowDialog *parentDialog );
	~VelocityPanelOrtho() override;
	void InitPanel( void ) override;
	void performUpdate( bool toUI ) override;
private:
	Ui::VelocityPanelOrtho *m_ui;
};

// VelocityPanelSphere //////////////////////////////////////////////////////////
class VelocityPanelSphere : public ISwapablePanel
{
	Q_OBJECT
public:
	explicit VelocityPanelSphere( DebugWindowDialog *parentDialog );
	~VelocityPanelSphere() override;
	void InitPanel( void ) override;
	void performUpdate( bool toUI ) override;
private:
	Ui::VelocityPanelSphere *m_ui;
};

// VelocityPanelHemisphere //////////////////////////////////////////////////////
class VelocityPanelHemisphere : public ISwapablePanel
{
	Q_OBJECT
public:
	explicit VelocityPanelHemisphere( DebugWindowDialog *parentDialog );
	~VelocityPanelHemisphere() override;
	void InitPanel( void ) override;
	void performUpdate( bool toUI ) override;
private:
	Ui::VelocityPanelHemisphere *m_ui;
};

// VelocityPanelCylinder ////////////////////////////////////////////////////////
class VelocityPanelCylinder : public ISwapablePanel
{
	Q_OBJECT
public:
	explicit VelocityPanelCylinder( DebugWindowDialog *parentDialog );
	~VelocityPanelCylinder() override;
	void InitPanel( void ) override;
	void performUpdate( bool toUI ) override;
private:
	Ui::VelocityPanelCylinder *m_ui;
};

// VelocityPanelOutward /////////////////////////////////////////////////////////
class VelocityPanelOutward : public ISwapablePanel
{
	Q_OBJECT
public:
	explicit VelocityPanelOutward( DebugWindowDialog *parentDialog );
	~VelocityPanelOutward() override;
	void InitPanel( void ) override;
	void performUpdate( bool toUI ) override;
private:
	Ui::VelocityPanelOutward *m_ui;
};
