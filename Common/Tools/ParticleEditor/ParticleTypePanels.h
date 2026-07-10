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

// FILE: ParticleTypePanels.h ///////////////////////////////////////////////////
// Desc:       Particle type panels (2-D particle, 3-D drawable, streak).
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ISwapablePanel.h"

namespace Ui
{
	class ParticlePanelParticle;
	class ParticlePanelDrawable;
	class ParticlePanelStreak;
}

// ParticlePanelParticle ////////////////////////////////////////////////////////
class ParticlePanelParticle : public ISwapablePanel
{
	Q_OBJECT
public:
	explicit ParticlePanelParticle( DebugWindowDialog *parentDialog );
	~ParticlePanelParticle() override;
	void InitPanel( void ) override;
	void performUpdate( bool toUI ) override;
private:
	Ui::ParticlePanelParticle *m_ui;
};

// ParticlePanelDrawable ////////////////////////////////////////////////////////
class ParticlePanelDrawable : public ISwapablePanel
{
	Q_OBJECT
public:
	explicit ParticlePanelDrawable( DebugWindowDialog *parentDialog );
	~ParticlePanelDrawable() override;
	void InitPanel( void ) override;
	void performUpdate( bool toUI ) override;
	void clearAllThingTemplates( void );
private:
	Ui::ParticlePanelDrawable *m_ui;
};

// ParticlePanelStreak //////////////////////////////////////////////////////////
class ParticlePanelStreak : public ISwapablePanel
{
	Q_OBJECT
public:
	explicit ParticlePanelStreak( DebugWindowDialog *parentDialog );
	~ParticlePanelStreak() override;
	void InitPanel( void ) override;
	void performUpdate( bool toUI ) override;
private:
	Ui::ParticlePanelStreak *m_ui;
};
