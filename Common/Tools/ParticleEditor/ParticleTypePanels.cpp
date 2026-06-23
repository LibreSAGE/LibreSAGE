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

// FILE: ParticleTypePanels.cpp /////////////////////////////////////////////////
// Desc:       Qt6 port of the particle type panels.
///////////////////////////////////////////////////////////////////////////////

#include "ParticleTypePanels.h"
#include "ParticleEditorDialog.h"

#include <QComboBox>
#include <QDir>

#include "ui_ParticlePanelParticle.h"
#include "ui_ParticlePanelDrawable.h"
#include "ui_ParticlePanelStreak.h"

// ParticlePanelParticle ////////////////////////////////////////////////////////
ParticlePanelParticle::ParticlePanelParticle( DebugWindowDialog *parentDialog )
	: ISwapablePanel( parentDialog )
	, m_ui( new Ui::ParticlePanelParticle )
{
	m_ui->setupUi( this );
	connect( m_ui->ParticleTypeParticle, &QComboBox::activated, this, &ISwapablePanel::onParticleSystemEdit );
}

ParticlePanelParticle::~ParticlePanelParticle() { delete m_ui; }

void ParticlePanelParticle::InitPanel( void )
{
	// The original scans "Art\Textures\" for files matching "EX*.*" and adds
	// their filenames to the combo.
	m_ui->ParticleTypeParticle->clear();

	QDir dir( "Art/Textures" );
	if (dir.exists()) {
		for (const QString &name : dir.entryList( QStringList{ "EX*" }, QDir::Files )) {
			m_ui->ParticleTypeParticle->addItem( name );
		}
	}
}

void ParticlePanelParticle::performUpdate( bool toUI )
{
	if (!m_dialog) {
		return;
	}

	if (toUI) {
		char buff[128];
		m_dialog->getParticleNameFromSystem( buff, 127 );
		int idx = m_ui->ParticleTypeParticle->findText( QString::fromLatin1( buff ) );
		if (idx >= 0) {
			m_ui->ParticleTypeParticle->setCurrentIndex( idx );
		}
	} else {
		if (m_ui->ParticleTypeParticle->currentIndex() >= 0) {
			m_dialog->updateParticleNameToSystem( m_ui->ParticleTypeParticle->currentText().toLatin1().constData() );
		}
	}
}

// ParticlePanelDrawable ////////////////////////////////////////////////////////
ParticlePanelDrawable::ParticlePanelDrawable( DebugWindowDialog *parentDialog )
	: ISwapablePanel( parentDialog )
	, m_ui( new Ui::ParticlePanelDrawable )
{
	m_ui->setupUi( this );
	connect( m_ui->ParticleTypeDrawable, &QComboBox::activated, this, &ISwapablePanel::onParticleSystemEdit );
}

ParticlePanelDrawable::~ParticlePanelDrawable() { delete m_ui; }
void ParticlePanelDrawable::InitPanel( void ) {}

void ParticlePanelDrawable::performUpdate( bool toUI )
{
	if (!m_dialog) {
		return;
	}

	if (m_ui->ParticleTypeDrawable->count() == 0) {
		// This is done here because InitPanel is called before ThingTemplates have been sent over.
		m_ui->ParticleTypeDrawable->addItem( NONE_STRING );
		const std::list<std::string> &r = m_dialog->getAllThingTemplates();
		for (std::list<std::string>::const_iterator cit = r.begin(); cit != r.end(); ++cit) {
			m_ui->ParticleTypeDrawable->addItem( QString::fromStdString( *cit ) );
		}
	}

	if (toUI) {
		char buff[128];
		m_dialog->getDrawableNameFromSystem( buff, 127 );
		int idx = m_ui->ParticleTypeDrawable->findText( QString::fromLatin1( buff ) );
		if (idx >= 0) {
			m_ui->ParticleTypeDrawable->setCurrentIndex( idx );
		}
	} else {
		if (m_ui->ParticleTypeDrawable->currentIndex() >= 0) {
			m_dialog->updateDrawableNameToSystem( m_ui->ParticleTypeDrawable->currentText().toLatin1().constData() );
		}
	}
}

void ParticlePanelDrawable::clearAllThingTemplates( void )
{
	m_ui->ParticleTypeDrawable->clear();
}

// ParticlePanelStreak //////////////////////////////////////////////////////////
ParticlePanelStreak::ParticlePanelStreak( DebugWindowDialog *parentDialog )
	: ISwapablePanel( parentDialog )
	, m_ui( new Ui::ParticlePanelStreak )
{
	m_ui->setupUi( this );
	connect( m_ui->ParticleTypeParticle, &QComboBox::activated, this, &ISwapablePanel::onParticleSystemEdit );
}

ParticlePanelStreak::~ParticlePanelStreak() { delete m_ui; }

void ParticlePanelStreak::InitPanel( void )
{
	// IDENTICAL behavior to ParticlePanelParticle: scan "Art\Textures\" for
	// files matching "EX*.*" and add their filenames to the combo.
	m_ui->ParticleTypeParticle->clear();

	QDir dir( "Art/Textures" );
	if (dir.exists()) {
		for (const QString &name : dir.entryList( QStringList{ "EX*" }, QDir::Files )) {
			m_ui->ParticleTypeParticle->addItem( name );
		}
	}
}

void ParticlePanelStreak::performUpdate( bool toUI )
{
	if (!m_dialog) {
		return;
	}

	if (toUI) {
		char buff[128];
		m_dialog->getParticleNameFromSystem( buff, 127 );
		int idx = m_ui->ParticleTypeParticle->findText( QString::fromLatin1( buff ) );
		if (idx >= 0) {
			m_ui->ParticleTypeParticle->setCurrentIndex( idx );
		}
	} else {
		if (m_ui->ParticleTypeParticle->currentIndex() >= 0) {
			m_dialog->updateParticleNameToSystem( m_ui->ParticleTypeParticle->currentText().toLatin1().constData() );
		}
	}
}
