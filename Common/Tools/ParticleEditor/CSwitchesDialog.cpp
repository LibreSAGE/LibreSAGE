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

// FILE: CSwitchesDialog.cpp /////////////////////////////////////////////////////
// Desc:       Qt6 port of the emission switches editor.
///////////////////////////////////////////////////////////////////////////////

#include "CSwitchesDialog.h"
#include "ParticleEditorDialog.h"

#include <QCheckBox>

#include "ui_CSwitchesDialog.h"

CSwitchesDialog::CSwitchesDialog( DebugWindowDialog *parentDialog )
	: QDialog( parentDialog )
	, m_ui( new Ui::CSwitchesDialog )
	, m_dialog( parentDialog )
{
	m_ui->setupUi( this );

	connect( m_ui->OneShot, &QCheckBox::toggled, this, &CSwitchesDialog::onParticleSystemEdit );
	connect( m_ui->Hollow, &QCheckBox::toggled, this, &CSwitchesDialog::onParticleSystemEdit );
	connect( m_ui->GroundAligned, &QCheckBox::toggled, this, &CSwitchesDialog::onParticleSystemEdit );
	connect( m_ui->EmitAboveGroundOnly, &QCheckBox::toggled, this, &CSwitchesDialog::onParticleSystemEdit );
	connect( m_ui->ParticleUpTowardsEmitter, &QCheckBox::toggled, this, &CSwitchesDialog::onParticleSystemEdit );
}

CSwitchesDialog::~CSwitchesDialog() { delete m_ui; }

void CSwitchesDialog::InitPanel( void ) {}

// if true, updates the UI from the Particle System.
// if false, updates the Particle System from the UI
void CSwitchesDialog::performUpdate( bool toUI )
{
	if (!m_dialog) {
		return;
	}

	struct { QCheckBox *box; SwitchType type; } fields[] = {
		{ m_ui->Hollow, ST_HOLLOW },
		{ m_ui->OneShot, ST_ONESHOT },
		{ m_ui->GroundAligned, ST_ALIGNXY },
		{ m_ui->EmitAboveGroundOnly, ST_EMITABOVEGROUNDONLY },
		{ m_ui->ParticleUpTowardsEmitter, ST_PARTICLEUPTOWARDSEMITTER },
	};

	for (auto &f : fields) {
		Bool switchVal;
		if (toUI) {
			m_dialog->getSwitchFromSystem( f.type, switchVal );
			f.box->setChecked( switchVal );
		} else {
			switchVal = f.box->isChecked();
			m_dialog->updateSwitchToSystem( f.type, switchVal );
		}
	}
}

void CSwitchesDialog::onParticleSystemEdit()
{
	if (m_dialog) {
		m_dialog->signalParticleSystemEdit();
	}
}
