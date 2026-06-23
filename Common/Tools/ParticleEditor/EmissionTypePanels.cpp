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

// FILE: EmissionTypePanels.cpp /////////////////////////////////////////////////
// Desc:       Qt6 port of the emission volume panels.
///////////////////////////////////////////////////////////////////////////////

#include "EmissionTypePanels.h"
#include "ParticleEditorDialog.h"

#include <QLineEdit>

#include "ui_EmissionPanelPoint.h"
#include "ui_EmissionPanelLine.h"
#include "ui_EmissionPanelBox.h"
#include "ui_EmissionPanelSphere.h"
#include "ui_EmissionPanelCylinder.h"

static QString fmtReal( Real v ) { return QString::asprintf( FORMAT_STRING, v ); }

// EmissionPanelPoint ///////////////////////////////////////////////////////////
EmissionPanelPoint::EmissionPanelPoint( DebugWindowDialog *parentDialog )
	: ISwapablePanel( parentDialog )
	, m_ui( new Ui::EmissionPanelPoint )
{
	m_ui->setupUi( this );
}

EmissionPanelPoint::~EmissionPanelPoint() { delete m_ui; }
void EmissionPanelPoint::InitPanel( void ) {}
void EmissionPanelPoint::performUpdate( bool /*toUI*/ ) {}

// EmissionPanelLine ////////////////////////////////////////////////////////////
EmissionPanelLine::EmissionPanelLine( DebugWindowDialog *parentDialog )
	: ISwapablePanel( parentDialog )
	, m_ui( new Ui::EmissionPanelLine )
{
	m_ui->setupUi( this );
	connect( m_ui->LineStartX, &QLineEdit::editingFinished, this, &ISwapablePanel::onParticleSystemEdit );
	connect( m_ui->LineStartY, &QLineEdit::editingFinished, this, &ISwapablePanel::onParticleSystemEdit );
	connect( m_ui->LineStartZ, &QLineEdit::editingFinished, this, &ISwapablePanel::onParticleSystemEdit );
	connect( m_ui->LineEndX, &QLineEdit::editingFinished, this, &ISwapablePanel::onParticleSystemEdit );
	connect( m_ui->LineEndY, &QLineEdit::editingFinished, this, &ISwapablePanel::onParticleSystemEdit );
	connect( m_ui->LineEndZ, &QLineEdit::editingFinished, this, &ISwapablePanel::onParticleSystemEdit );
}

EmissionPanelLine::~EmissionPanelLine() { delete m_ui; }
void EmissionPanelLine::InitPanel( void ) {}

void EmissionPanelLine::performUpdate( bool toUI )
{
	if (!m_dialog) {
		return;
	}

	struct { QLineEdit *edit; int coord; } fields[] = {
		{ m_ui->LineStartX, 0 }, { m_ui->LineStartY, 1 }, { m_ui->LineStartZ, 2 },
		{ m_ui->LineEndX, 3 }, { m_ui->LineEndY, 4 }, { m_ui->LineEndZ, 5 },
	};

	for (auto &f : fields) {
		Real linePoint;
		if (toUI) {
			m_dialog->getLineFromSystem( f.coord, linePoint );
			f.edit->setText( fmtReal( linePoint ) );
		} else {
			linePoint = f.edit->text().toDouble();
			m_dialog->updateLineToSystem( f.coord, linePoint );
		}
	}
}

// EmissionPanelBox /////////////////////////////////////////////////////////////
EmissionPanelBox::EmissionPanelBox( DebugWindowDialog *parentDialog )
	: ISwapablePanel( parentDialog )
	, m_ui( new Ui::EmissionPanelBox )
{
	m_ui->setupUi( this );
	connect( m_ui->BoxHalfSizeX, &QLineEdit::editingFinished, this, &ISwapablePanel::onParticleSystemEdit );
	connect( m_ui->BoxHalfSizeY, &QLineEdit::editingFinished, this, &ISwapablePanel::onParticleSystemEdit );
	connect( m_ui->BoxHalfSizeZ, &QLineEdit::editingFinished, this, &ISwapablePanel::onParticleSystemEdit );
}

EmissionPanelBox::~EmissionPanelBox() { delete m_ui; }
void EmissionPanelBox::InitPanel( void ) {}

void EmissionPanelBox::performUpdate( bool toUI )
{
	if (!m_dialog) {
		return;
	}

	struct { QLineEdit *edit; int coord; } fields[] = {
		{ m_ui->BoxHalfSizeX, 0 }, { m_ui->BoxHalfSizeY, 1 }, { m_ui->BoxHalfSizeZ, 2 },
	};

	for (auto &f : fields) {
		Real halfSize;
		if (toUI) {
			m_dialog->getHalfSizeFromSystem( f.coord, halfSize );
			f.edit->setText( fmtReal( halfSize ) );
		} else {
			halfSize = f.edit->text().toDouble();
			m_dialog->updateHalfSizeToSystem( f.coord, halfSize );
		}
	}
}

// EmissionPanelSphere //////////////////////////////////////////////////////////
EmissionPanelSphere::EmissionPanelSphere( DebugWindowDialog *parentDialog )
	: ISwapablePanel( parentDialog )
	, m_ui( new Ui::EmissionPanelSphere )
{
	m_ui->setupUi( this );
	connect( m_ui->SphereRadius, &QLineEdit::editingFinished, this, &ISwapablePanel::onParticleSystemEdit );
}

EmissionPanelSphere::~EmissionPanelSphere() { delete m_ui; }
void EmissionPanelSphere::InitPanel( void ) {}

void EmissionPanelSphere::performUpdate( bool toUI )
{
	if (!m_dialog) {
		return;
	}

	// NOTE: the original editor reads/writes the sphere radius via the half-size
	// accessor (the emission volume is a union); preserved verbatim.
	Real radius;
	if (toUI) {
		m_dialog->getHalfSizeFromSystem( 0, radius );
		m_ui->SphereRadius->setText( fmtReal( radius ) );
	} else {
		radius = m_ui->SphereRadius->text().toDouble();
		m_dialog->updateHalfSizeToSystem( 0, radius );
	}
}

// EmissionPanelCylinder ////////////////////////////////////////////////////////
EmissionPanelCylinder::EmissionPanelCylinder( DebugWindowDialog *parentDialog )
	: ISwapablePanel( parentDialog )
	, m_ui( new Ui::EmissionPanelCylinder )
{
	m_ui->setupUi( this );
	connect( m_ui->CylRadius, &QLineEdit::editingFinished, this, &ISwapablePanel::onParticleSystemEdit );
	connect( m_ui->CylLength, &QLineEdit::editingFinished, this, &ISwapablePanel::onParticleSystemEdit );
}

EmissionPanelCylinder::~EmissionPanelCylinder() { delete m_ui; }
void EmissionPanelCylinder::InitPanel( void ) {}

void EmissionPanelCylinder::performUpdate( bool toUI )
{
	if (!m_dialog) {
		return;
	}

	Real radius;
	if (toUI) {
		m_dialog->getCylinderRadiusFromSystem( radius );
		m_ui->CylRadius->setText( fmtReal( radius ) );
	} else {
		radius = m_ui->CylRadius->text().toDouble();
		m_dialog->updateCylinderRadiusToSystem( radius );
	}

	Real length;
	if (toUI) {
		m_dialog->getCylinderLengthFromSystem( length );
		m_ui->CylLength->setText( fmtReal( length ) );
	} else {
		length = m_ui->CylLength->text().toDouble();
		m_dialog->updateCylinderLengthToSystem( length );
	}
}
