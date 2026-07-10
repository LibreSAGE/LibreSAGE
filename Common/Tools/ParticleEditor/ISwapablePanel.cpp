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

#include "ISwapablePanel.h"
#include "ParticleEditorDialog.h"

ISwapablePanel::ISwapablePanel( DebugWindowDialog *parentDialog )
	: QWidget( parentDialog )
	, m_dialog( parentDialog )
{
}

void ISwapablePanel::onParticleSystemEdit()
{
	if (m_dialog) {
		m_dialog->signalParticleSystemEdit();
	}
}
