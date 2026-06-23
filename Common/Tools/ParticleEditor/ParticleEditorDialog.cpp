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

// FILE: ParticleEditorDialog.cpp ///////////////////////////////////////////////
// Desc:       Qt6 port of the particle editor main window (class DebugWindowDialog).
///////////////////////////////////////////////////////////////////////////////

#define DEFINE_PARTICLE_SYSTEM_NAMES 1

#include "ParticleEditorDialog.h"

#include <cmath>

#include <QComboBox>
#include <QLineEdit>

#include "ui_ParticleEditorDialog.h"

#include "CColorAlphaDialog.h"
#include "CSwitchesDialog.h"
#include "MoreParmsDialog.h"
#include "EmissionTypePanels.h"
#include "ParticleTypePanels.h"
#include "VelocityTypePanels.h"
#include "ParticleEditorExport.h"

#define ARBITRARY_BUFF_SIZE 128

static QString fmtReal( Real v ) { return QString::asprintf( FORMAT_STRING, v ); }

DebugWindowDialog::DebugWindowDialog( QWidget *parent )
	: QMainWindow( parent )
	, m_ui( new Ui::ParticleEditorDialog )
{
	m_ui->setupUi( this );

	m_activeEmissionPage = 0;
	m_activeVelocityPage = 0;
	m_activeParticlePage = 0;
	m_particleSystem = NULL;

	m_changeHasOcurred = false;
	m_shouldWriteINI = false;
	m_showColorDlg = false;
	m_showSwitchesDlg = false;
	m_showMoreParmsDlg = false;
	m_shouldBusyWait = false;
	m_shouldReload = false;
	m_shouldUpdateParticleCap = false;
	m_shouldReloadTextures = false;
	m_shouldKillAllParticleSystems = false;

	m_emissionTypePanels[0] = new EmissionPanelPoint( this );
	m_emissionTypePanels[1] = new EmissionPanelLine( this );
	m_emissionTypePanels[2] = new EmissionPanelBox( this );
	m_emissionTypePanels[3] = new EmissionPanelSphere( this );
	m_emissionTypePanels[4] = new EmissionPanelCylinder( this );

	m_velocityTypePanels[0] = new VelocityPanelOrtho( this );
	m_velocityTypePanels[1] = new VelocityPanelSphere( this );
	m_velocityTypePanels[2] = new VelocityPanelHemisphere( this );
	m_velocityTypePanels[3] = new VelocityPanelCylinder( this );
	m_velocityTypePanels[4] = new VelocityPanelOutward( this );

	m_particleTypePanels[0] = new ParticlePanelParticle( this );
	m_particleTypePanels[1] = new ParticlePanelDrawable( this );
	m_particleTypePanels[2] = new ParticlePanelStreak( this );

	m_particleParmValues.resize( PARM_NumParms );

	m_colorAlphaDialog = new CColorAlphaDialog( this );
	m_switchesDialog = new CSwitchesDialog( this );
	m_moreParmsDialog = new MoreParmsDialog( this );

	// Host the swapable panels inside their stacked widgets.
	int i;
	for (i = 0; i < NUM_EMISSION_TYPES; ++i) {
		m_ui->EmissionPanel->addWidget( m_emissionTypePanels[i] );
	}
	for (i = 0; i < NUM_VELOCITY_TYPES; ++i) {
		m_ui->VelocityPanel->addWidget( m_velocityTypePanels[i] );
	}
	for (i = 0; i < NUM_PARTICLE_TYPES; ++i) {
		m_ui->ParticlePanel->addWidget( m_particleTypePanels[i] );
	}

	// Wire up the controls (activated == user only, matching MFC CBN_SELCHANGE).
	connect( m_ui->ParticleSystem, &QComboBox::activated, this, &DebugWindowDialog::OnParticleSystemChange );
	connect( m_ui->Go, &QPushButton::clicked, this, &DebugWindowDialog::OnParticleSystemChange );
	connect( m_ui->KillAll, &QPushButton::clicked, this, &DebugWindowDialog::OnKillAllParticleSystems );
	connect( m_ui->EditColorButton, &QPushButton::clicked, this, &DebugWindowDialog::OnEditColorAlpha );
	connect( m_ui->Continued, &QPushButton::clicked, this, &DebugWindowDialog::OnEditMoreParms );
	connect( m_ui->EditSwitchesButton, &QPushButton::clicked, this, &DebugWindowDialog::OnEditSwitches );

	connect( m_ui->Priority, &QComboBox::activated, this, &DebugWindowDialog::OnParticleSystemEdit );
	connect( m_ui->EmissionType, &QComboBox::activated, this, &DebugWindowDialog::OnParticleSystemEdit );
	connect( m_ui->VelocityType, &QComboBox::activated, this, &DebugWindowDialog::OnParticleSystemEdit );
	connect( m_ui->ParticleType, &QComboBox::activated, this, &DebugWindowDialog::OnParticleSystemEdit );
	connect( m_ui->ShaderType, &QComboBox::activated, this, &DebugWindowDialog::OnParticleSystemEdit );

	connect( m_ui->actionReloadCurrent, &QAction::triggered, this, &DebugWindowDialog::OnReloadCurrent );
	connect( m_ui->actionReloadAll, &QAction::triggered, this, &DebugWindowDialog::OnReloadAll );
	connect( m_ui->actionSaveCurrent, &QAction::triggered, this, &DebugWindowDialog::OnSaveCurrent );
	connect( m_ui->actionSaveAll, &QAction::triggered, this, &DebugWindowDialog::OnSaveAll );
	connect( m_ui->actionReloadTextures, &QAction::triggered, this, &DebugWindowDialog::OnReloadTextures );

	connect( m_ui->CurrentParticleCap, &QLineEdit::editingFinished, this, &DebugWindowDialog::OnParticleCapEdit );

	QLineEdit *editFields[] = {
		m_ui->AngleXMin, m_ui->AngleYMin, m_ui->AngleZMin,
		m_ui->AngleXMax, m_ui->AngleYMax, m_ui->AngleZMax,
		m_ui->AngularRateXMin, m_ui->AngularRateYMin, m_ui->AngularRateZMin,
		m_ui->AngularRateXMax, m_ui->AngularRateYMax, m_ui->AngularRateZMax,
		m_ui->AngleDampingMin, m_ui->AngleDampingMax,
		m_ui->VelocityDampingMin, m_ui->VelocityDampingMax,
		m_ui->Gravity,
	};
	for (QLineEdit *edit : editFields) {
		connect( edit, &QLineEdit::editingFinished, this, &DebugWindowDialog::OnParticleSystemEdit );
	}
}

DebugWindowDialog::~DebugWindowDialog()
{
	delete m_ui;
}

void DebugWindowDialog::InitPanel( void )
{
	auto fillCombo = []( QComboBox *combo, const char *const *names ) {
		if (!combo) {
			return;
		}
		for (int i = 1; names[i]; ++i) {
			combo->addItem( names[i] );
		}
		combo->setCurrentIndex( 0 );
	};

	fillCombo( m_ui->Priority, ParticlePriorityNames );
	fillCombo( m_ui->EmissionType, EmissionVolumeTypeNames );
	fillCombo( m_ui->VelocityType, EmissionVelocityTypeNames );
	fillCombo( m_ui->ParticleType, ParticleTypeNames );
	fillCombo( m_ui->ShaderType, ParticleShaderTypeNames );

	m_colorAlphaDialog->InitPanel();
	m_switchesDialog->InitPanel();
	m_moreParmsDialog->InitPanel();

	int j;
	for (j = 0; j < NUM_EMISSION_TYPES; ++j) {
		m_emissionTypePanels[j]->InitPanel();
	}
	for (j = 0; j < NUM_VELOCITY_TYPES; ++j) {
		m_velocityTypePanels[j]->InitPanel();
	}
	for (j = 0; j < NUM_PARTICLE_TYPES; ++j) {
		m_particleTypePanels[j]->InitPanel();
	}

	m_ui->EmissionPanel->setCurrentIndex( 0 );
	m_ui->VelocityPanel->setCurrentIndex( 0 );
	m_ui->ParticlePanel->setCurrentIndex( 0 );
}

void DebugWindowDialog::addParticleSystem( const char *particleSystem )
{
	if (!particleSystem) {
		return;
	}

	std::string particleSystemName = particleSystem;
	appendParticleSystemToList( particleSystemName );
}

void DebugWindowDialog::addThingTemplate( const char *thingTemplate )
{
	if (!thingTemplate) {
		return;
	}

	std::string thingTemplateName = thingTemplate;
	appendThingTemplateToList( thingTemplateName );
}

void DebugWindowDialog::clearAllParticleSystems( void )
{
	m_listOfParticleSystems.clear();
	m_ui->ParticleSystem->clear();
}

void DebugWindowDialog::clearAllThingTemplates( void )
{
	m_listOfThingTemplates.clear();
	// this is a kindof(dirty_hack), because there's no way at runtime to verify that
	// particleTypePanels[1] is actually a ParticlePanelDrawable
	((ParticlePanelDrawable*)m_particleTypePanels[1])->clearAllThingTemplates();
}

void DebugWindowDialog::appendParticleSystemToList( const std::string &rString )
{
	m_listOfParticleSystems.push_front( rString );
	m_ui->ParticleSystem->addItem( QString::fromStdString( rString ) );
}

void DebugWindowDialog::appendThingTemplateToList( const std::string &rString )
{
	m_listOfThingTemplates.push_back( rString );
}

void DebugWindowDialog::OnParticleSystemChange()
{
	m_changeHasOcurred = true;
}

void DebugWindowDialog::OnPushSave()
{
	m_shouldWriteINI = true;
}

void DebugWindowDialog::OnReloadTextures()
{
	// First, reload the textures
	ParticlePanelParticle* pParticle = (ParticlePanelParticle*) m_particleTypePanels[0];
	if (!pParticle) {
		return;
	}

	pParticle->InitPanel();

	// Then, force an update to the ui, to repick the appropriate texture.
	performUpdate( true );

	// Finally, signal a flag to the asset manager to reload the actual textures.
	m_shouldReloadTextures = true;
}

Bool DebugWindowDialog::hasSelectionChanged( void )
{
	if (m_changeHasOcurred) {
		m_changeHasOcurred = false;
		return true;
	}

	return false;
}

void DebugWindowDialog::getSelectedSystemName( char *bufferToCopyInto ) const
{
	if (!bufferToCopyInto) {
		return;
	}

	QComboBox *combo = m_ui->ParticleSystem;
	int ndx = combo->currentIndex();
	QString string;
	if (ndx > -1) {
		string = combo->itemText( ndx );
	} else {
		string = combo->currentText();
	}

	strcpy( bufferToCopyInto, string.toLatin1().constData() );
}

void DebugWindowDialog::getSelectedParticleAsciiStringParm( int parmNum, char *bufferToCopyInto ) const
{
	if (parmNum < 0 || parmNum >= PARM_NumParms || !bufferToCopyInto || !m_particleSystem) {
		return;
	}

	if (m_particleParmValues[parmNum].length()) {
		strcpy( bufferToCopyInto, m_particleParmValues[parmNum].c_str() );
	} else {
		bufferToCopyInto[0] = 0;
	}
}

void DebugWindowDialog::updateParticleAsciiStringParm( int parmNum, const char *bufferToCopyFrom )
{
	if (parmNum < 0 || parmNum >= PARM_NumParms || !bufferToCopyFrom) {
		return;
	}

	m_particleParmValues[parmNum] = bufferToCopyFrom;
}

void DebugWindowDialog::getColorValueFromSystem( Int systemNum, RGBColorKeyframe &colorFrame ) const
{
	if (systemNum >= MAX_KEYFRAMES || systemNum < 0 || !m_particleSystem) {
		return;
	}

	colorFrame = m_particleSystem->m_colorKey[systemNum];
}

void DebugWindowDialog::updateColorValueToSystem( Int systemNum, const RGBColorKeyframe &colorFrame )
{
	if (systemNum >= MAX_KEYFRAMES || systemNum < 0 || !m_particleSystem) {
		return;
	}

	m_particleSystem->m_colorKey[systemNum] = colorFrame;
}

void DebugWindowDialog::getAlphaRangeFromSystem( Int systemNum, ParticleSystemInfo::RandomKeyframe &randomVar ) const
{
	if (systemNum >= MAX_KEYFRAMES || systemNum < 0 || !m_particleSystem) {
		return;
	}

	randomVar = m_particleSystem->m_alphaKey[systemNum];
}

void DebugWindowDialog::updateAlphaRangeToSystem( Int systemNum, const ParticleSystemInfo::RandomKeyframe &randomVar )
{
	if (systemNum >= MAX_KEYFRAMES || systemNum < 0 || !m_particleSystem) {
		return;
	}

	m_particleSystem->m_alphaKey[systemNum] = randomVar;
}

void DebugWindowDialog::getHalfSizeFromSystem( Int coordNum, Real& halfSize ) const
{
	if (!m_particleSystem) {
		return;
	}

	switch (coordNum)
	{
		case 0: halfSize = m_particleSystem->m_emissionVolume.box.halfSize.x; return;
		case 1: halfSize = m_particleSystem->m_emissionVolume.box.halfSize.y; return;
		case 2: halfSize = m_particleSystem->m_emissionVolume.box.halfSize.z; return;
		default: return;
	};
}

void DebugWindowDialog::updateHalfSizeToSystem( Int coordNum, const Real& halfSize )
{
	if (!m_particleSystem) {
		return;
	}

	switch (coordNum)
	{
		case 0: m_particleSystem->m_emissionVolume.box.halfSize.x = halfSize; return;
		case 1: m_particleSystem->m_emissionVolume.box.halfSize.y = halfSize; return;
		case 2: m_particleSystem->m_emissionVolume.box.halfSize.z = halfSize; return;
		default: return;
	};
}

void DebugWindowDialog::getSphereRadiusFromSystem( Real &radius ) const
{
	if (!m_particleSystem) {
		return;
	}

	radius = m_particleSystem->m_emissionVolume.sphere.radius;
}

void DebugWindowDialog::updateSphereRadiusToSystem( const Real &radius )
{
	if (!m_particleSystem) {
		return;
	}

	m_particleSystem->m_emissionVolume.sphere.radius = radius;
}

void DebugWindowDialog::getCylinderRadiusFromSystem( Real &radius ) const
{
	if (!m_particleSystem) {
		return;
	}

	radius = m_particleSystem->m_emissionVolume.cylinder.radius;
}

void DebugWindowDialog::updateCylinderRadiusToSystem( const Real &radius )
{
	if (!m_particleSystem) {
		return;
	}

	m_particleSystem->m_emissionVolume.cylinder.radius = radius;
}

void DebugWindowDialog::getCylinderLengthFromSystem( Real &length ) const
{
	if (!m_particleSystem) {
		return;
	}

	length = m_particleSystem->m_emissionVolume.cylinder.length;
}

void DebugWindowDialog::updateCylinderLengthToSystem( const Real &length )
{
	if (!m_particleSystem) {
		return;
	}

	m_particleSystem->m_emissionVolume.cylinder.length = length;
}

void DebugWindowDialog::getLineFromSystem( Int coordNum, Real& linePoint ) const
{
	if (!m_particleSystem) {
		return;
	}

	switch (coordNum)
	{
		case 0: linePoint = m_particleSystem->m_emissionVolume.line.start.x; return;
		case 1: linePoint = m_particleSystem->m_emissionVolume.line.start.y; return;
		case 2: linePoint = m_particleSystem->m_emissionVolume.line.start.z; return;
		case 3: linePoint = m_particleSystem->m_emissionVolume.line.end.x; return;
		case 4: linePoint = m_particleSystem->m_emissionVolume.line.end.y; return;
		case 5: linePoint = m_particleSystem->m_emissionVolume.line.end.z; return;
		default: return;
	};
}

void DebugWindowDialog::updateLineToSystem( Int coordNum, const Real& linePoint )
{
	if (!m_particleSystem) {
		return;
	}

	switch (coordNum)
	{
		case 0: m_particleSystem->m_emissionVolume.line.start.x = linePoint; return;
		case 1: m_particleSystem->m_emissionVolume.line.start.y = linePoint; return;
		case 2: m_particleSystem->m_emissionVolume.line.start.z = linePoint; return;
		case 3: m_particleSystem->m_emissionVolume.line.end.x = linePoint; return;
		case 4: m_particleSystem->m_emissionVolume.line.end.y = linePoint; return;
		case 5: m_particleSystem->m_emissionVolume.line.end.z = linePoint; return;
		default: return;
	};
}

void DebugWindowDialog::getVelSphereFromSystem( Int velNum, Real &radius ) const
{
	if (!m_particleSystem) {
		return;
	}

	switch (velNum)
	{
		case 0: radius = m_particleSystem->m_emissionVelocity.spherical.speed.m_low; return;
		case 1: radius = m_particleSystem->m_emissionVelocity.spherical.speed.m_high; return;
		default: return;
	};
}

void DebugWindowDialog::updateVelSphereToSystem( Int velNum, const Real &radius )
{
	if (!m_particleSystem) {
		return;
	}

	switch (velNum)
	{
		case 0: m_particleSystem->m_emissionVelocity.spherical.speed.m_low = radius; return;
		case 1: m_particleSystem->m_emissionVelocity.spherical.speed.m_high = radius; return;
		default: return;
	};
}

void DebugWindowDialog::getVelHemisphereFromSystem( Int velNum, Real &radius ) const
{
	if (!m_particleSystem) {
		return;
	}

	switch (velNum)
	{
		case 0: radius = m_particleSystem->m_emissionVelocity.hemispherical.speed.m_low; return;
		case 1: radius = m_particleSystem->m_emissionVelocity.hemispherical.speed.m_high; return;
		default: return;
	};
}

void DebugWindowDialog::updateVelHemisphereToSystem( Int velNum, const Real &radius )
{
	if (!m_particleSystem) {
		return;
	}

	switch (velNum)
	{
		case 0: m_particleSystem->m_emissionVelocity.hemispherical.speed.m_low = radius; return;
		case 1: m_particleSystem->m_emissionVelocity.hemispherical.speed.m_high = radius; return;
		default: return;
	};
}

void DebugWindowDialog::getVelOrthoFromSystem( Int coordNum, Real& ortho ) const
{
	if (!m_particleSystem) {
		return;
	}

	switch (coordNum)
	{
		case 0: ortho = m_particleSystem->m_emissionVelocity.ortho.x.m_low; return;
		case 1: ortho = m_particleSystem->m_emissionVelocity.ortho.y.m_low; return;
		case 2: ortho = m_particleSystem->m_emissionVelocity.ortho.z.m_low; return;
		case 3: ortho = m_particleSystem->m_emissionVelocity.ortho.x.m_high; return;
		case 4: ortho = m_particleSystem->m_emissionVelocity.ortho.y.m_high; return;
		case 5: ortho = m_particleSystem->m_emissionVelocity.ortho.z.m_high; return;
		default: return;
	};
}

void DebugWindowDialog::updateVelOrthoToSystem( Int coordNum, const Real& ortho )
{
	if (!m_particleSystem) {
		return;
	}

	switch (coordNum)
	{
		case 0: m_particleSystem->m_emissionVelocity.ortho.x.m_low = ortho; return;
		case 1: m_particleSystem->m_emissionVelocity.ortho.y.m_low = ortho; return;
		case 2: m_particleSystem->m_emissionVelocity.ortho.z.m_low = ortho; return;
		case 3: m_particleSystem->m_emissionVelocity.ortho.x.m_high = ortho; return;
		case 4: m_particleSystem->m_emissionVelocity.ortho.y.m_high = ortho; return;
		case 5: m_particleSystem->m_emissionVelocity.ortho.z.m_high = ortho; return;
		default: return;
	};
}

void DebugWindowDialog::getVelCylinderFromSystem( Int coordNum, Real& cylinder ) const
{
	if (!m_particleSystem) {
		return;
	}

	switch (coordNum)
	{
		case 0: cylinder = m_particleSystem->m_emissionVelocity.cylindrical.radial.m_low; return;
		case 1: cylinder = m_particleSystem->m_emissionVelocity.cylindrical.normal.m_low; return;
		case 2: cylinder = m_particleSystem->m_emissionVelocity.cylindrical.radial.m_high; return;
		case 3: cylinder = m_particleSystem->m_emissionVelocity.cylindrical.normal.m_high; return;
		default: return;
	};
}

void DebugWindowDialog::updateVelCylinderToSystem( Int coordNum, const Real& cylinder )
{
	if (!m_particleSystem) {
		return;
	}

	switch (coordNum)
	{
		case 0: m_particleSystem->m_emissionVelocity.cylindrical.radial.m_low = cylinder; return;
		case 1: m_particleSystem->m_emissionVelocity.cylindrical.normal.m_low = cylinder; return;
		case 2: m_particleSystem->m_emissionVelocity.cylindrical.radial.m_high = cylinder; return;
		case 3: m_particleSystem->m_emissionVelocity.cylindrical.normal.m_high = cylinder; return;
		default: return;
	};
}

void DebugWindowDialog::getVelOutwardFromSystem( Int coordNum, Real& cylinder ) const
{
	if (!m_particleSystem) {
		return;
	}

	switch (coordNum)
	{
		case 0: cylinder = m_particleSystem->m_emissionVelocity.outward.speed.m_low; return;
		case 1: cylinder = m_particleSystem->m_emissionVelocity.outward.otherSpeed.m_low; return;
		case 2: cylinder = m_particleSystem->m_emissionVelocity.outward.speed.m_high; return;
		case 3: cylinder = m_particleSystem->m_emissionVelocity.outward.otherSpeed.m_high; return;
		default: return;
	};
}

void DebugWindowDialog::updateVelOutwardToSystem( Int coordNum, const Real& cylinder )
{
	if (!m_particleSystem) {
		return;
	}

	switch (coordNum)
	{
		case 0: m_particleSystem->m_emissionVelocity.outward.speed.m_low = cylinder; return;
		case 1: m_particleSystem->m_emissionVelocity.outward.otherSpeed.m_low = cylinder; return;
		case 2: m_particleSystem->m_emissionVelocity.outward.speed.m_high = cylinder; return;
		case 3: m_particleSystem->m_emissionVelocity.outward.otherSpeed.m_high = cylinder; return;
		default: return;
	};
}

void DebugWindowDialog::getParticleNameFromSystem( char *buffer, int /*buffLen*/ ) const
{
	if (!m_particleSystem || !buffer) {
		return;
	}

	getSelectedParticleAsciiStringParm( PARM_ParticleTypeName, buffer );
}

void DebugWindowDialog::updateParticleNameToSystem( const char *buffer )
{
	if (!m_particleSystem || !buffer) {
		return;
	}

	updateParticleAsciiStringParm( PARM_ParticleTypeName, buffer );
}

void DebugWindowDialog::getDrawableNameFromSystem( char *buffer, int buffLen ) const
{
	getParticleNameFromSystem( buffer, buffLen );
}

void DebugWindowDialog::updateDrawableNameToSystem( const char* buffer )
{
	updateParticleNameToSystem( buffer );
}

void DebugWindowDialog::updateCurrentParticleSystem( ParticleSystemTemplate *particleTemplate )
{
	m_particleSystem = particleTemplate;
	performUpdate( true );
}

void DebugWindowDialog::updateSystemUseParameters( ParticleSystemTemplate *particleTemplate )
{
	m_particleSystem = particleTemplate;
	performUpdate( false );
}

void DebugWindowDialog::getInitialDelayFromSystem( Int parmNum, Real& initialDelay ) const
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: initialDelay = m_particleSystem->m_initialDelay.m_low; return;
		case 1: initialDelay = m_particleSystem->m_initialDelay.m_high; return;
		default: return;
	};
}

void DebugWindowDialog::updateInitialDelayToSystem( Int parmNum, const Real& initialDelay )
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: m_particleSystem->m_initialDelay.m_low = initialDelay; return;
		case 1: m_particleSystem->m_initialDelay.m_high = initialDelay; return;
		default: return;
	};
}

void DebugWindowDialog::getBurstDelayFromSystem( Int parmNum, Real& burstDelay ) const
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: burstDelay = m_particleSystem->m_burstDelay.m_low; return;
		case 1: burstDelay = m_particleSystem->m_burstDelay.m_high; return;
		default: return;
	};
}

void DebugWindowDialog::updateBurstDelayToSystem( Int parmNum, const Real& burstDelay )
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: m_particleSystem->m_burstDelay.m_low = burstDelay; return;
		case 1: m_particleSystem->m_burstDelay.m_high = burstDelay; return;
		default: return;
	};
}

void DebugWindowDialog::getBurstCountFromSystem( Int parmNum, Real& burstCount ) const
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: burstCount = m_particleSystem->m_burstCount.m_low; return;
		case 1: burstCount = m_particleSystem->m_burstCount.m_high; return;
		default: return;
	};
}

void DebugWindowDialog::updateBurstCountToSystem( Int parmNum, const Real& burstCount )
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: m_particleSystem->m_burstCount.m_low = burstCount; return;
		case 1: m_particleSystem->m_burstCount.m_high = burstCount; return;
		default: return;
	};
}

void DebugWindowDialog::getColorScaleFromSystem( Int parmNum, Real& colorScale ) const
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: colorScale = m_particleSystem->m_colorScale.m_low; return;
		case 1: colorScale = m_particleSystem->m_colorScale.m_high; return;
		default: return;
	};
}

void DebugWindowDialog::updateColorScaleToSystem( Int parmNum, const Real& colorScale )
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: m_particleSystem->m_colorScale.m_low = colorScale; return;
		case 1: m_particleSystem->m_colorScale.m_high = colorScale; return;
		default: return;
	};
}

void DebugWindowDialog::getParticleLifetimeFromSystem( Int parmNum, Real& particleLifetime ) const
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: particleLifetime = m_particleSystem->m_lifetime.m_low; return;
		case 1: particleLifetime = m_particleSystem->m_lifetime.m_high; return;
		default: return;
	};
}

void DebugWindowDialog::updateParticleLifetimeToSystem( Int parmNum, const Real& particleLifetime )
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: m_particleSystem->m_lifetime.m_low = particleLifetime; return;
		case 1: m_particleSystem->m_lifetime.m_high = particleLifetime; return;
		default: return;
	};
}

void DebugWindowDialog::getParticleSizeFromSystem( Int parmNum, Real& particleSize ) const
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: particleSize = m_particleSystem->m_startSize.m_low; return;
		case 1: particleSize = m_particleSystem->m_startSize.m_high; return;
		default: return;
	};
}

void DebugWindowDialog::updateParticleSizeToSystem( Int parmNum, const Real& particleSize )
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: m_particleSystem->m_startSize.m_low = particleSize; return;
		case 1: m_particleSystem->m_startSize.m_high = particleSize; return;
		default: return;
	};
}

void DebugWindowDialog::getStartSizeRateFromSystem( Int parmNum, Real& startSizeRate ) const
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: startSizeRate = m_particleSystem->m_startSizeRate.m_low; return;
		case 1: startSizeRate = m_particleSystem->m_startSizeRate.m_high; return;
		default: return;
	};
}

void DebugWindowDialog::updateStartSizeRateToSystem( Int parmNum, const Real& startSizeRate )
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: m_particleSystem->m_startSizeRate.m_low = startSizeRate; return;
		case 1: m_particleSystem->m_startSizeRate.m_high = startSizeRate; return;
		default: return;
	};
}

void DebugWindowDialog::getSizeRateFromSystem( Int parmNum, Real& sizeRate ) const
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: sizeRate = m_particleSystem->m_sizeRate.m_low; return;
		case 1: sizeRate = m_particleSystem->m_sizeRate.m_high; return;
		default: return;
	};
}

void DebugWindowDialog::updateSizeRateToSystem( Int parmNum, const Real& sizeRate )
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: m_particleSystem->m_sizeRate.m_low = sizeRate; return;
		case 1: m_particleSystem->m_sizeRate.m_high = sizeRate; return;
		default: return;
	};
}

void DebugWindowDialog::getSizeDampingFromSystem( Int parmNum, Real& sizeDamping ) const
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: sizeDamping = m_particleSystem->m_sizeRateDamping.m_low; return;
		case 1: sizeDamping = m_particleSystem->m_sizeRateDamping.m_high; return;
		default: return;
	};
}

void DebugWindowDialog::updateSizeDampingToSystem( Int parmNum, const Real& sizeDamping )
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: m_particleSystem->m_sizeRateDamping.m_low = sizeDamping; return;
		case 1: m_particleSystem->m_sizeRateDamping.m_high = sizeDamping; return;
		default: return;
	};
}

void DebugWindowDialog::getSystemLifetimeFromSystem( Real& systemLifetime ) const
{
	if (!m_particleSystem) {
		return;
	}

	systemLifetime = m_particleSystem->m_systemLifetime;
}

void DebugWindowDialog::updateSystemLifetimeToSystem( const Real& systemLifetime )
{
	if (!m_particleSystem) {
		return;
	}

	m_particleSystem->m_systemLifetime = systemLifetime;
}

void DebugWindowDialog::getSlaveOffsetFromSystem( Int parmNum, Real& slaveOffset ) const
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: slaveOffset = m_particleSystem->m_slavePosOffset.x; return;
		case 1: slaveOffset = m_particleSystem->m_slavePosOffset.y; return;
		case 2: slaveOffset = m_particleSystem->m_slavePosOffset.z; return;
		default: return;
	};
}

void DebugWindowDialog::updateSlaveOffsetToSystem( Int parmNum, const Real& slaveOffset )
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: m_particleSystem->m_slavePosOffset.x = slaveOffset; return;
		case 1: m_particleSystem->m_slavePosOffset.y = slaveOffset; return;
		case 2: m_particleSystem->m_slavePosOffset.z = slaveOffset; return;
		default: return;
	};
}

void DebugWindowDialog::getDriftVelocityFromSystem( Int parmNum, Real& driftVelocity ) const
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: driftVelocity = m_particleSystem->m_driftVelocity.x; return;
		case 1: driftVelocity = m_particleSystem->m_driftVelocity.y; return;
		case 2: driftVelocity = m_particleSystem->m_driftVelocity.z; return;
		default: return;
	};
}

void DebugWindowDialog::updateDriftVelocityToSystem( Int parmNum, const Real& driftVelocity )
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: m_particleSystem->m_driftVelocity.x = driftVelocity; return;
		case 1: m_particleSystem->m_driftVelocity.y = driftVelocity; return;
		case 2: m_particleSystem->m_driftVelocity.z = driftVelocity; return;
		default: return;
	};
}

void DebugWindowDialog::getSlaveSystemFromSystem( char *buffer, Int /*bufferSize*/ ) const
{
	if (!m_particleSystem) {
		return;
	}

	getSelectedParticleAsciiStringParm( PARM_SlaveSystemName, buffer );
}

void DebugWindowDialog::updateSlaveSystemToSystem( const char *buffer )
{
	if (!m_particleSystem) {
		return;
	}

	updateParticleAsciiStringParm( PARM_SlaveSystemName, buffer );
}

void DebugWindowDialog::getPerParticleSystemFromSystem( char *buffer, Int /*bufferSize*/ ) const
{
	if (!m_particleSystem) {
		return;
	}

	getSelectedParticleAsciiStringParm( PARM_AttachedSystemName, buffer );
}

void DebugWindowDialog::updatePerParticleSystemToSystem( const char *buffer )
{
	if (!m_particleSystem) {
		return;
	}

	updateParticleAsciiStringParm( PARM_AttachedSystemName, buffer );
}

void DebugWindowDialog::getSwitchFromSystem( SwitchType switchType, Bool& switchVal ) const
{
	if (!m_particleSystem) {
		return;
	}

	switch (switchType)
	{
		case ST_HOLLOW: switchVal = m_particleSystem->m_isEmissionVolumeHollow; break;
		case ST_ONESHOT: switchVal = m_particleSystem->m_isOneShot; break;
		case ST_ALIGNXY: switchVal = m_particleSystem->m_isGroundAligned; break;
		case ST_EMITABOVEGROUNDONLY: switchVal = m_particleSystem->m_isEmitAboveGroundOnly; break;
		case ST_PARTICLEUPTOWARDSEMITTER: switchVal = m_particleSystem->m_isParticleUpTowardsEmitter; break;
	};
}

void DebugWindowDialog::updateSwitchToSystem( SwitchType switchType, const Bool& switchVal )
{
	if (!m_particleSystem) {
		return;
	}

	switch (switchType)
	{
		case ST_HOLLOW: m_particleSystem->m_isEmissionVolumeHollow = switchVal; break;
		case ST_ONESHOT: m_particleSystem->m_isOneShot = switchVal; break;
		case ST_ALIGNXY: m_particleSystem->m_isGroundAligned = switchVal; break;
		case ST_EMITABOVEGROUNDONLY: m_particleSystem->m_isEmitAboveGroundOnly = switchVal; break;
		case ST_PARTICLEUPTOWARDSEMITTER: m_particleSystem->m_isParticleUpTowardsEmitter = switchVal; break;
	};
}

// ------------------------------------------------------------------------------------------------
static Real degreeToRadian( Real degree ) { return (degree / 180.0f) * (Real)M_PI; }
static Real radianToDegree( Real radian ) { return (180.0f * radian) / (Real)M_PI; }

void DebugWindowDialog::getPingPongStartAngleFromSystem( Int parmNum, Real& angle ) const
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: angle = m_particleSystem->m_windMotionStartAngleMin; break;
		case 1: angle = m_particleSystem->m_windMotionStartAngleMax; break;
		default: return;
	};

	angle = radianToDegree( angle );
}

void DebugWindowDialog::updatePingPongStartAngleToSystem( Int parmNum, const Real& angle )
{
	if (!m_particleSystem) {
		return;
	}

	Real radian = degreeToRadian( angle );

	switch (parmNum)
	{
		case 0: m_particleSystem->m_windMotionStartAngleMin = radian; return;
		case 1: m_particleSystem->m_windMotionStartAngleMax = radian; return;
		default: return;
	};
}

void DebugWindowDialog::getPingPongEndAngleFromSystem( Int parmNum, Real& angle ) const
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: angle = m_particleSystem->m_windMotionEndAngleMin; break;
		case 1: angle = m_particleSystem->m_windMotionEndAngleMax; break;
		default: return;
	};

	angle = radianToDegree( angle );
}

void DebugWindowDialog::updatePingPongEndAngleToSystem( Int parmNum, const Real& angle )
{
	if (!m_particleSystem) {
		return;
	}

	Real radian = degreeToRadian( angle );

	switch (parmNum)
	{
		case 0: m_particleSystem->m_windMotionEndAngleMin = radian; return;
		case 1: m_particleSystem->m_windMotionEndAngleMax = radian; return;
		default: return;
	};
}

void DebugWindowDialog::getWindAngleChangeFromSystem( Int parmNum, Real& angle ) const
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: angle = m_particleSystem->m_windAngleChangeMin; break;
		case 1: angle = m_particleSystem->m_windAngleChangeMax; break;
		default: return;
	};

	angle = radianToDegree( angle );
}

void DebugWindowDialog::updateWindAngleChangeToSystem( Int parmNum, const Real& angle )
{
	if (!m_particleSystem) {
		return;
	}

	Real radian = degreeToRadian( angle );

	switch (parmNum)
	{
		case 0: m_particleSystem->m_windAngleChangeMin = radian; return;
		case 1: m_particleSystem->m_windAngleChangeMax = radian; return;
		default: return;
	};
}

void DebugWindowDialog::getWindMotionFromSystem( ParticleSystemInfo::WindMotion& windMotion ) const
{
	if (!m_particleSystem) {
		return;
	}

	windMotion = m_particleSystem->m_windMotion;
}

void DebugWindowDialog::updateWindMotionToSystem( const ParticleSystemInfo::WindMotion& windMotion )
{
	if (!m_particleSystem) {
		return;
	}

	m_particleSystem->m_windMotion = windMotion;
}

// The reason for this function is to prohibit forgetting to add an update one way
// or the other; they're all in one place.
void DebugWindowDialog::performUpdate( Bool toUI )
{
	if (!m_particleSystem) {
		return;
	}

	{	// Update the emission type, velocity type, particle type and shader type.
		{
			QComboBox *pCombo = m_ui->Priority;
			if (toUI) {
				int idx = pCombo->findText( ParticlePriorityNames[(long) m_particleSystem->m_priority] );
				if (idx >= 0) {
					pCombo->setCurrentIndex( idx );
				}
			} else {
				int selndx = pCombo->currentIndex();
				if (selndx >= 0) {
					m_particleSystem->m_priority = (ParticlePriorityType)(selndx + 1);
				}
			}
		}

		{
			QComboBox *pCombo = m_ui->EmissionType;
			int selndx;
			if (toUI) {
				selndx = pCombo->findText( EmissionVolumeTypeNames[(long) m_particleSystem->m_emissionVolumeType] );
				if (selndx >= 0) {
					pCombo->setCurrentIndex( selndx );
				}
			} else {
				selndx = pCombo->currentIndex();
				if (selndx >= 0) {
					m_particleSystem->m_emissionVolumeType = (ParticleSystemInfo::EmissionVolumeType) (selndx + 1);
				}
			}

			if (selndx != m_activeEmissionPage && selndx >= 0) {
				m_activeEmissionPage = selndx;
				m_ui->EmissionPanel->setCurrentIndex( m_activeEmissionPage );
			}
		}

		{
			QComboBox *pCombo = m_ui->VelocityType;
			int selndx;
			if (toUI) {
				selndx = pCombo->findText( EmissionVelocityTypeNames[(long) m_particleSystem->m_emissionVelocityType] );
				if (selndx >= 0) {
					pCombo->setCurrentIndex( selndx );
				}
			} else {
				selndx = pCombo->currentIndex();
				if (selndx >= 0) {
					m_particleSystem->m_emissionVelocityType = (ParticleSystemInfo::EmissionVelocityType) (selndx + 1);
				}
			}

			if (selndx != m_activeVelocityPage && selndx >= 0) {
				m_activeVelocityPage = selndx;
				m_ui->VelocityPanel->setCurrentIndex( m_activeVelocityPage );
			}
		}

		{
			QComboBox *pCombo = m_ui->ParticleType;
			int selndx;
			if (toUI) {
				selndx = pCombo->findText( ParticleTypeNames[(long) m_particleSystem->m_particleType] );
				if (selndx >= 0) {
					pCombo->setCurrentIndex( selndx );
				}
			} else {
				selndx = pCombo->currentIndex();
				if (selndx >= 0) {
					m_particleSystem->m_particleType = (ParticleSystemInfo::ParticleType) (selndx + 1);
				}
			}

			if (selndx != m_activeParticlePage && selndx >= 0) {
				m_activeParticlePage = selndx;
				m_ui->ParticlePanel->setCurrentIndex( m_activeParticlePage );
			}
		}

		{
			QComboBox *pCombo = m_ui->ShaderType;
			if (toUI) {
				int idx = pCombo->findText( ParticleShaderTypeNames[(long) m_particleSystem->m_shaderType] );
				if (idx >= 0) {
					pCombo->setCurrentIndex( idx );
				}
			} else {
				int selndx = pCombo->currentIndex();
				if (selndx >= 0) {
					m_particleSystem->m_shaderType = (ParticleSystemInfo::ParticleShaderType) (selndx + 1);
				}
			}
		}
	}

	{	// Angle X, Y, Z and Angular Rate X, Y, Z.  Only Z is editable; X/Y are display-only zeros.
		if (toUI) {
			m_ui->AngleXMin->setText( fmtReal( 0.0f ) );
			m_ui->AngleYMin->setText( fmtReal( 0.0f ) );
			m_ui->AngleXMax->setText( fmtReal( 0.0f ) );
			m_ui->AngleYMax->setText( fmtReal( 0.0f ) );
			m_ui->AngularRateXMin->setText( fmtReal( 0.0f ) );
			m_ui->AngularRateYMin->setText( fmtReal( 0.0f ) );
			m_ui->AngularRateXMax->setText( fmtReal( 0.0f ) );
			m_ui->AngularRateYMax->setText( fmtReal( 0.0f ) );

			m_ui->AngleZMin->setText( fmtReal( m_particleSystem->m_angleZ.getMinimumValue() ) );
			m_ui->AngleZMax->setText( fmtReal( m_particleSystem->m_angleZ.getMaximumValue() ) );
			m_ui->AngularRateZMin->setText( fmtReal( m_particleSystem->m_angularRateZ.getMinimumValue() ) );
			m_ui->AngularRateZMax->setText( fmtReal( m_particleSystem->m_angularRateZ.getMaximumValue() ) );
		} else {
			m_particleSystem->m_angleZ.m_low = m_ui->AngleZMin->text().toDouble();
			m_particleSystem->m_angleZ.m_high = m_ui->AngleZMax->text().toDouble();
			m_particleSystem->m_angularRateZ.m_low = m_ui->AngularRateZMin->text().toDouble();
			m_particleSystem->m_angularRateZ.m_high = m_ui->AngularRateZMax->text().toDouble();
		}
	}

	{	// damping values.
		if (toUI) {
			m_ui->AngleDampingMin->setText( fmtReal( m_particleSystem->m_angularDamping.getMinimumValue() ) );
			m_ui->AngleDampingMax->setText( fmtReal( m_particleSystem->m_angularDamping.getMaximumValue() ) );
			m_ui->VelocityDampingMin->setText( fmtReal( m_particleSystem->m_velDamping.getMinimumValue() ) );
			m_ui->VelocityDampingMax->setText( fmtReal( m_particleSystem->m_velDamping.getMaximumValue() ) );
		} else {
			m_particleSystem->m_angularDamping.m_low = m_ui->AngleDampingMin->text().toDouble();
			m_particleSystem->m_angularDamping.m_high = m_ui->AngleDampingMax->text().toDouble();
			m_particleSystem->m_velDamping.m_low = m_ui->VelocityDampingMin->text().toDouble();
			m_particleSystem->m_velDamping.m_high = m_ui->VelocityDampingMax->text().toDouble();
		}
	}

	{	// gravity
		if (toUI) {
			m_ui->Gravity->setText( fmtReal( m_particleSystem->m_gravity ) );
		} else {
			m_particleSystem->m_gravity = m_ui->Gravity->text().toDouble();
		}
	}

	{	// all the kids need to update too.
		m_colorAlphaDialog->performUpdate( toUI );
		m_switchesDialog->performUpdate( toUI );
		m_moreParmsDialog->performUpdate( toUI );
		m_emissionTypePanels[m_activeEmissionPage]->performUpdate( toUI );
		m_velocityTypePanels[m_activeVelocityPage]->performUpdate( toUI );
		m_particleTypePanels[m_activeParticlePage]->performUpdate( toUI );
	}
}

void DebugWindowDialog::OnParticleSystemEdit()
{
	signalParticleSystemEdit();
}

void DebugWindowDialog::signalParticleSystemEdit( void )
{
	performUpdate( false );
}

void DebugWindowDialog::OnEditColorAlpha()
{
	m_showColorDlg = !m_showColorDlg;

	m_colorAlphaDialog->setVisible( m_showColorDlg );
	m_ui->EditColorButton->setChecked( m_showColorDlg );
}

void DebugWindowDialog::OnEditSwitches()
{
	m_showSwitchesDlg = !m_showSwitchesDlg;

	m_switchesDialog->setVisible( m_showSwitchesDlg );
	m_ui->EditSwitchesButton->setChecked( m_showSwitchesDlg );
}

void DebugWindowDialog::OnKillAllParticleSystems()
{
	m_shouldKillAllParticleSystems = true;
}

void DebugWindowDialog::OnEditMoreParms()
{
	m_showMoreParmsDlg = !m_showMoreParmsDlg;

	m_moreParmsDialog->setVisible( m_showMoreParmsDlg );
	m_ui->Continued->setChecked( m_showMoreParmsDlg );
}

Bool DebugWindowDialog::shouldWriteINI( void )
{
	if (m_shouldWriteINI) {
		m_shouldWriteINI = false;
		return true;
	}

	return false;
}

Bool DebugWindowDialog::hasRequestedReload( void )
{
	if (m_shouldReload) {
		m_shouldReload = false;
		return true;
	}

	return false;
}

Bool DebugWindowDialog::shouldBusyWait( void )
{
	return m_shouldBusyWait;
}

Bool DebugWindowDialog::shouldUpdateParticleCap( void )
{
	if (m_shouldUpdateParticleCap) {
		m_shouldUpdateParticleCap = false;
		return true;
	}

	return false;
}

Bool DebugWindowDialog::shouldReloadTextures( void )
{
	if (m_shouldReloadTextures) {
		m_shouldReloadTextures = false;
		return true;
	}

	return false;
}

Bool DebugWindowDialog::shouldKillAllParticleSystems( void )
{
	if (m_shouldKillAllParticleSystems) {
		m_shouldKillAllParticleSystems = false;
		return true;
	}

	return false;
}

void DebugWindowDialog::OnReloadSystem( void )
{
	m_shouldReload = true;
}

void DebugWindowDialog::OnReloadCurrent()
{
	OnReloadSystem();
}

void DebugWindowDialog::OnReloadAll()
{
	OnReloadSystem();
}

void DebugWindowDialog::OnSaveCurrent()
{
	OnPushSave();
}

void DebugWindowDialog::OnSaveAll()
{
	OnPushSave();
}

void DebugWindowDialog::OnParticleCapEdit()
{
	m_shouldUpdateParticleCap = true;
}

void DebugWindowDialog::updateCurrentParticleCap( int particleCap )
{
	m_ui->CurrentParticleCap->setText( QString::number( particleCap ) );
}

void DebugWindowDialog::updateCurrentNumParticles( int particleCount )
{
	m_ui->CurrentParticleCount->setText( QString::number( particleCount ) );
}

int DebugWindowDialog::getNewParticleCap( void )
{
	return m_ui->CurrentParticleCap->text().toInt();
}
