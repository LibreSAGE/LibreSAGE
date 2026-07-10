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

// FILE: VelocityTypePanels.cpp /////////////////////////////////////////////////
// Desc:       Qt6 port of the velocity type panels.
///////////////////////////////////////////////////////////////////////////////

#include "VelocityTypePanels.h"
#include "ParticleEditorDialog.h"

#include <QLineEdit>

#include "ui_VelocityPanelOrtho.h"
#include "ui_VelocityPanelSphere.h"
#include "ui_VelocityPanelHemisphere.h"
#include "ui_VelocityPanelCylinder.h"
#include "ui_VelocityPanelOutward.h"

static QString fmtReal( Real v ) { return QString::asprintf( FORMAT_STRING, v ); }

// VelocityPanelOrtho ///////////////////////////////////////////////////////////
VelocityPanelOrtho::VelocityPanelOrtho( DebugWindowDialog *parentDialog )
	: ISwapablePanel( parentDialog )
	, m_ui( new Ui::VelocityPanelOrtho )
{
	m_ui->setupUi( this );
	connect( m_ui->OrthoXMin, &QLineEdit::editingFinished, this, &ISwapablePanel::onParticleSystemEdit );
	connect( m_ui->OrthoYMin, &QLineEdit::editingFinished, this, &ISwapablePanel::onParticleSystemEdit );
	connect( m_ui->OrthoZMin, &QLineEdit::editingFinished, this, &ISwapablePanel::onParticleSystemEdit );
	connect( m_ui->OrthoXMax, &QLineEdit::editingFinished, this, &ISwapablePanel::onParticleSystemEdit );
	connect( m_ui->OrthoYMax, &QLineEdit::editingFinished, this, &ISwapablePanel::onParticleSystemEdit );
	connect( m_ui->OrthoZMax, &QLineEdit::editingFinished, this, &ISwapablePanel::onParticleSystemEdit );
}

VelocityPanelOrtho::~VelocityPanelOrtho() { delete m_ui; }
void VelocityPanelOrtho::InitPanel( void ) {}

void VelocityPanelOrtho::performUpdate( bool toUI )
{
	if (!m_dialog) {
		return;
	}

	struct { QLineEdit *edit; int coord; } fields[] = {
		{ m_ui->OrthoXMin, 0 }, { m_ui->OrthoYMin, 1 }, { m_ui->OrthoZMin, 2 },
		{ m_ui->OrthoXMax, 3 }, { m_ui->OrthoYMax, 4 }, { m_ui->OrthoZMax, 5 },
	};

	for (auto &f : fields) {
		Real ortho;
		if (toUI) {
			m_dialog->getVelOrthoFromSystem( f.coord, ortho );
			f.edit->setText( fmtReal( ortho ) );
		} else {
			ortho = f.edit->text().toDouble();
			m_dialog->updateVelOrthoToSystem( f.coord, ortho );
		}
	}
}

// VelocityPanelSphere //////////////////////////////////////////////////////////
VelocityPanelSphere::VelocityPanelSphere( DebugWindowDialog *parentDialog )
	: ISwapablePanel( parentDialog )
	, m_ui( new Ui::VelocityPanelSphere )
{
	m_ui->setupUi( this );
	connect( m_ui->SphereRadialMin, &QLineEdit::editingFinished, this, &ISwapablePanel::onParticleSystemEdit );
	connect( m_ui->SphereRadialMax, &QLineEdit::editingFinished, this, &ISwapablePanel::onParticleSystemEdit );
}

VelocityPanelSphere::~VelocityPanelSphere() { delete m_ui; }
void VelocityPanelSphere::InitPanel( void ) {}

void VelocityPanelSphere::performUpdate( bool toUI )
{
	if (!m_dialog) {
		return;
	}

	struct { QLineEdit *edit; int coord; } fields[] = {
		{ m_ui->SphereRadialMin, 0 }, { m_ui->SphereRadialMax, 1 },
	};

	for (auto &f : fields) {
		Real radial;
		if (toUI) {
			m_dialog->getVelSphereFromSystem( f.coord, radial );
			f.edit->setText( fmtReal( radial ) );
		} else {
			radial = f.edit->text().toDouble();
			m_dialog->updateVelSphereToSystem( f.coord, radial );
		}
	}
}

// VelocityPanelHemisphere //////////////////////////////////////////////////////
VelocityPanelHemisphere::VelocityPanelHemisphere( DebugWindowDialog *parentDialog )
	: ISwapablePanel( parentDialog )
	, m_ui( new Ui::VelocityPanelHemisphere )
{
	m_ui->setupUi( this );
	connect( m_ui->HemisphereRadialMin, &QLineEdit::editingFinished, this, &ISwapablePanel::onParticleSystemEdit );
	connect( m_ui->HemisphereRadialMax, &QLineEdit::editingFinished, this, &ISwapablePanel::onParticleSystemEdit );
}

VelocityPanelHemisphere::~VelocityPanelHemisphere() { delete m_ui; }
void VelocityPanelHemisphere::InitPanel( void ) {}

void VelocityPanelHemisphere::performUpdate( bool toUI )
{
	if (!m_dialog) {
		return;
	}

	struct { QLineEdit *edit; int coord; } fields[] = {
		{ m_ui->HemisphereRadialMin, 0 }, { m_ui->HemisphereRadialMax, 1 },
	};

	for (auto &f : fields) {
		Real radial;
		if (toUI) {
			m_dialog->getVelHemisphereFromSystem( f.coord, radial );
			f.edit->setText( fmtReal( radial ) );
		} else {
			radial = f.edit->text().toDouble();
			m_dialog->updateVelHemisphereToSystem( f.coord, radial );
		}
	}
}

// VelocityPanelCylinder ////////////////////////////////////////////////////////
VelocityPanelCylinder::VelocityPanelCylinder( DebugWindowDialog *parentDialog )
	: ISwapablePanel( parentDialog )
	, m_ui( new Ui::VelocityPanelCylinder )
{
	m_ui->setupUi( this );
	connect( m_ui->CylinderRadialMin, &QLineEdit::editingFinished, this, &ISwapablePanel::onParticleSystemEdit );
	connect( m_ui->CylinderNormalMin, &QLineEdit::editingFinished, this, &ISwapablePanel::onParticleSystemEdit );
	connect( m_ui->CylinderRadialMax, &QLineEdit::editingFinished, this, &ISwapablePanel::onParticleSystemEdit );
	connect( m_ui->CylinderNormalMax, &QLineEdit::editingFinished, this, &ISwapablePanel::onParticleSystemEdit );
}

VelocityPanelCylinder::~VelocityPanelCylinder() { delete m_ui; }
void VelocityPanelCylinder::InitPanel( void ) {}

void VelocityPanelCylinder::performUpdate( bool toUI )
{
	if (!m_dialog) {
		return;
	}

	struct { QLineEdit *edit; int coord; } fields[] = {
		{ m_ui->CylinderRadialMin, 0 }, { m_ui->CylinderNormalMin, 1 },
		{ m_ui->CylinderRadialMax, 2 }, { m_ui->CylinderNormalMax, 3 },
	};

	for (auto &f : fields) {
		Real cylinder;
		if (toUI) {
			m_dialog->getVelCylinderFromSystem( f.coord, cylinder );
			f.edit->setText( fmtReal( cylinder ) );
		} else {
			cylinder = f.edit->text().toDouble();
			m_dialog->updateVelCylinderToSystem( f.coord, cylinder );
		}
	}
}

// VelocityPanelOutward /////////////////////////////////////////////////////////
VelocityPanelOutward::VelocityPanelOutward( DebugWindowDialog *parentDialog )
	: ISwapablePanel( parentDialog )
	, m_ui( new Ui::VelocityPanelOutward )
{
	m_ui->setupUi( this );
	connect( m_ui->OutwardSpeedMin, &QLineEdit::editingFinished, this, &ISwapablePanel::onParticleSystemEdit );
	connect( m_ui->OutwardOtherMin, &QLineEdit::editingFinished, this, &ISwapablePanel::onParticleSystemEdit );
	connect( m_ui->OutwardSpeedMax, &QLineEdit::editingFinished, this, &ISwapablePanel::onParticleSystemEdit );
	connect( m_ui->OutwardOtherMax, &QLineEdit::editingFinished, this, &ISwapablePanel::onParticleSystemEdit );
}

VelocityPanelOutward::~VelocityPanelOutward() { delete m_ui; }
void VelocityPanelOutward::InitPanel( void ) {}

void VelocityPanelOutward::performUpdate( bool toUI )
{
	if (!m_dialog) {
		return;
	}

	struct { QLineEdit *edit; int coord; } fields[] = {
		{ m_ui->OutwardSpeedMin, 0 }, { m_ui->OutwardOtherMin, 1 },
		{ m_ui->OutwardSpeedMax, 2 }, { m_ui->OutwardOtherMax, 3 },
	};

	for (auto &f : fields) {
		Real outward;
		if (toUI) {
			m_dialog->getVelOutwardFromSystem( f.coord, outward );
			f.edit->setText( fmtReal( outward ) );
		} else {
			outward = f.edit->text().toDouble();
			m_dialog->updateVelOutwardToSystem( f.coord, outward );
		}
	}
}
